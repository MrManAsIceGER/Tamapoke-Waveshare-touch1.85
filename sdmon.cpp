
#include "sdmon.h"
#include "pin_config.h"
#include <FS.h>
#include <SD_MMC.h>

bool sdReady = false;
bool sdDirty = false;
SdThumbs thumbs;

bool PmdMon::load(uint8_t dexNum, bool shiny) {
  unload();
  if (!sdReady) return false;

  char path[28];
  snprintf(path, sizeof(path), "/mons/p%s%03u.bin", shiny ? "s" : "", dexNum);
  File f = SD_MMC.open(path, FILE_READ);
  if (!f && shiny) {
    snprintf(path, sizeof(path), "/mons/p%03u.bin", dexNum);
    f = SD_MMC.open(path, FILE_READ);
  }
  if (!f) return false;

  uint32_t size = f.size();
  if (size < 7 || size > 3UL * 1024 * 1024) {
    f.close();
    return false;
  }

  blob = (uint8_t *)ps_malloc(size);
  if (!blob || f.read(blob, size) != size || memcmp(blob, "TPK2", 4) != 0) {
    if (blob) {
      free(blob);
      blob = nullptr;
    }
    f.close();
    return false;
  }
  f.close();

  uint8_t nActs = blob[4];
  memcpy(&palCount, blob + 5, 2);
  if (palCount > 256 || (uint32_t)7 + palCount * 2 > size) {
    unload();
    return false;
  }
  memcpy(pal, blob + 7, palCount * 2);

  const uint8_t *p = blob + 7 + palCount * 2;
  const uint8_t *end = blob + size;

  for (uint8_t i = 0; i < nActs && p + 4 <= end; i++) {
    uint8_t id = p[0], w = p[1], h = p[2], nf = p[3];
    p += 4;

    if (id >= PMD_NACTS || nf > 24) {
      unload();
      return false;
    }

    uint32_t bytes = (uint32_t)nf * 2 + (uint32_t)w * h * nf;
    if (w == 0 || h == 0 || nf == 0 || p + bytes > end) {
      unload();
      return false;
    }

    PmdAct &a = acts[id];
    a.w = w;
    a.h = h;
    a.frames = nf;

    for (uint8_t k = 0; k < nf; k++) {
      a.ms[k] = p[0] | (p[1] << 8);
      if (a.ms[k] == 0) a.ms[k] = 100;
      p += 2;
    }

    a.data = p;
    p += (uint32_t)w * h * nf;

    uint8_t base = 1;
    for (uint8_t f = 0; f < nf; f++) {
      const uint8_t *fr = a.data + (uint32_t)f * w * h;
      for (int r = h - 1; r >= 0; r--) {
        bool any = false;
        for (int c = 0; c < w && !any; c++) {
          if (fr[r * w + c] != 0xFF) any = true;
        }
        if (any) {
          if (r + 1 > base) base = r + 1;
          break;
        }
      }
    }
    a.base = base;
  }

  loaded = true;
  Serial.printf("cargado %s (%u KB)\n", path, size / 1024);
  return true;
}

void PmdMon::unload() {
  if (blob) {
    free(blob);
    blob = nullptr;
  }
  for (auto &a : acts) {
    a.w = a.h = a.frames = a.base = 0;
    a.data = nullptr;
  }
  loaded = false;
}

void SdThumbs::unload() {
  if (data) {
    free(data);
    data = nullptr;
  }
  loaded = false;
  count = 0;
  size = 0;
}

bool SdThumbs::load() {
  unload();
  if (!sdReady) return false;

  File f = SD_MMC.open("/mons/thumbs.bin", FILE_READ);
  if (!f) {
    Serial.println("sin thumbs.bin (galeria sin miniaturas)");
    return false;
  }

  uint32_t sz = f.size();
  if (sz < 10 || sz > 2UL * 1024 * 1024) {
    Serial.println("thumbs.bin invalido (tamano)");
    f.close();
    return false;
  }

  data = (uint8_t *)ps_malloc(sz);
  if (!data || f.read(data, sz) != sz || memcmp(data, "TPTH", 4) != 0) {
    Serial.println("thumbs.bin invalido");
    if (data) {
      free(data);
      data = nullptr;
    }
    f.close();
    return false;
  }
  f.close();

  memcpy(&count, data + 4, 2);
  if ((uint32_t)6 + 4UL * count > sz) {
    Serial.println("thumbs.bin invalido (tabla de offsets)");
    unload();
    return false;
  }

  size = sz;
  loaded = true;
  Serial.printf("miniaturas cargadas: %u (%u KB)\n", count, sz / 1024);
  return true;
}

const uint8_t *SdThumbs::get(int16_t dex) const {
  if (!loaded || dex < 1 || dex > count) return nullptr;

  uint32_t off;
  memcpy(&off, data + 6 + 4 * (dex - 1), 4);

  if (off > size - 3) return nullptr;

  uint32_t need = 3 + (uint32_t)data[off + 2] * 2 +
                  (uint32_t)data[off] * data[off + 1];

  if (need > size || off > size - need) return nullptr;
  return data + off;
}

bool sdBegin() {
  // Waveshare ESP32-S3-Touch-LCD-1.85 Rev 2.0:
  // SDMMC 1-Bit: CLK=GPIO14, CMD=GPIO17, D0=GPIO16.
  // D1/D2 sind nicht angeschlossen; D3/EXIO3 wird im 1-Bit-Modus
  // nicht als SPI-Chip-Select verwendet.
  SD_MMC.setPins(SD_SCLK, SD_MOSI, SD_MISO);

  // Kein automatisches Formatieren bei Mount-Fehlern.
  sdReady = SD_MMC.begin("/sdcard", true, false);

  if (sdReady) {
    Serial.printf("SD montada: %llu MB\n",
                  SD_MMC.cardSize() / (1024ULL * 1024ULL));
    SD_MMC.mkdir("/mons");
  } else {
    Serial.println("SD no detectada (el juego usa los sprites de flash)");
  }

  return sdReady;
}

bool SdMon::load(uint8_t dexNum, bool shiny) {
  unload();
  if (!sdReady) return false;

  char path[24];
  snprintf(path, sizeof(path), "/mons/%s%03u.bin",
           shiny ? "s" : "", dexNum);

  File f = SD_MMC.open(path, FILE_READ);
  if (!f && shiny) {
    snprintf(path, sizeof(path), "/mons/%03u.bin", dexNum);
    f = SD_MMC.open(path, FILE_READ);
  }

  if (!f) {
    Serial.printf("no existe %s\n", path);
    return false;
  }

  char magic[4];
  uint16_t header[4];

  if (f.read((uint8_t *)magic, 4) != 4 ||
      memcmp(magic, "TPK1", 4) != 0 ||
      f.read((uint8_t *)header, 8) != 8) {
    f.close();
    return false;
  }

  w = header[0];
  h = header[1];
  frames = header[2];
  frameMs = header[3];

  if (f.read((uint8_t *)&palCount, 2) != 2 ||
      palCount > 256 ||
      w == 0 || w > 256 ||
      h == 0 || h > 256 ||
      frames == 0 || frames > 64) {
    f.close();
    return false;
  }

  if (f.read((uint8_t *)pal, palCount * 2) != palCount * 2) {
    f.close();
    return false;
  }

  uint32_t size = (uint32_t)w * h * frames;
  data = (uint8_t *)ps_malloc(size);

  if (!data) {
    Serial.println("sin PSRAM para el sprite");
    f.close();
    return false;
  }

  uint32_t got = f.read(data, size);
  f.close();

  if (got != size) {
    Serial.printf("%s truncado (%u de %u)\n", path, got, size);
    unload();
    return false;
  }

  scale = 200 / h;
  if (scale < 2) scale = 2;
  if (scale > 5) scale = 5;

  Serial.printf("cargado %s: %ux%u x%u frames @%ums, escala %u\n",
                path, w, h, frames, frameMs, scale);

  loaded = true;
  return true;
}

void SdMon::unload() {
  if (data) {
    free(data);
    data = nullptr;
  }
  loaded = false;
}

// ---------------------------------------------------------------------------
// Protocolo de carga por USB:
//   PUT <ruta> <bytes>\n + datos crudos -> "OK" ... "DONE"
//   LS\n                                -> listado de /mons
// Usar con tools/send_sd.py
// ---------------------------------------------------------------------------

bool sdSerialCommand(const String &line) {
  if (line.startsWith("PUT ")) {
    int sp = line.lastIndexOf(' ');
    String path = line.substring(4, sp);
    uint32_t size = line.substring(sp + 1).toInt();

    if (!sdReady || size == 0 || size > 4 * 1024 * 1024) {
      Serial.println("ERR");
      return true;
    }

    if (!path.startsWith("/")) path = "/" + path;

    if (!path.startsWith("/mons/") || path.indexOf("..") >= 0) {
      Serial.println("ERR");
      return true;
    }

    if (SD_MMC.exists(path)) SD_MMC.remove(path);

    File f = SD_MMC.open(path, FILE_WRITE);
    if (!f) {
      Serial.println("ERR");
      return true;
    }

    Serial.println("OK");

    static uint8_t buf[2048];
    uint32_t remaining = size;
    Serial.setTimeout(5000);

    while (remaining > 0) {
      size_t want = remaining > sizeof(buf) ? sizeof(buf) : remaining;
      size_t n = Serial.readBytes(buf, want);
      if (n == 0) break;

      if (f.write(buf, n) != n) break;

      remaining -= n;
      Serial.println("#");
    }

    f.close();
    Serial.setTimeout(1000);

    sdDirty = (remaining == 0);
    Serial.println(remaining == 0 ? "DONE" : "ERR");
    return true;

  } else if (line == "SDINFO") {
    Serial.printf("total=%llu used=%llu\n",
                  SD_MMC.totalBytes(), SD_MMC.usedBytes());
    Serial.println("DONE");
    return true;

  } else if (line == "LS") {
    File dir = SD_MMC.open("/mons");

    if (dir) {
      File e;
      while ((e = dir.openNextFile())) {
        Serial.printf("%s %u\n", e.name(), (uint32_t)e.size());
        e.close();
      }
      dir.close();
    }

    Serial.println("DONE");
    return true;
  }

  return false;
}
