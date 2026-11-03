
#pragma once
#include <Arduino.h>

// Legacy TPK1 animated sprite format (fallback path). The project uses
// PMD/TPK2 (PmdMon) for all sprites; this path is inactive unless NNN.bin exists on the SD card.
// Indexed data is stored in PSRAM; the palette uses RGB565.
struct SdMon {
  bool loaded = false;
  uint16_t w = 0, h = 0, frames = 0, frameMs = 100;
  uint8_t scale = 2;
  uint16_t palCount = 0;
  uint16_t pal[256];
  uint8_t *data = nullptr;

  bool load(uint8_t dexNum, bool shiny = false);
  void unload();
};

// PMD sprite actions (TPK2 format)
enum : uint8_t {
  PMD_IDLE = 0, PMD_WALKL, PMD_WALKR, PMD_SLEEP, PMD_EAT, PMD_HURT,
  PMD_ATTACK, PMD_POSE, PMD_HOP, PMD_NOD, PMD_BREATH, PMD_SIT,
  PMD_NACTS
};

struct PmdAct {
  uint8_t w = 0, h = 0, frames = 0;
  uint8_t base = 0;
  uint16_t ms[24];
  const uint8_t *data = nullptr;
};

// PMD multi-action sprite loaded from SD into PSRAM
struct PmdMon {
  bool loaded = false;
  uint16_t palCount = 0;
  uint16_t pal[256];
  uint8_t *blob = nullptr;
  PmdAct acts[PMD_NACTS];

  bool load(uint8_t dexNum, bool shiny = false);
  void unload();
  bool has(uint8_t a) const {
    return loaded && a < PMD_NACTS && acts[a].frames > 0;
  }
};

// Gallery thumbnails (the complete thumbs.bin file is stored in PSRAM)
struct SdThumbs {
  bool loaded = false;
  uint8_t *data = nullptr;
  uint16_t count = 0;
  uint32_t size = 0;

  bool load();
  void unload();
  const uint8_t *get(int16_t dex) const;
};

extern SdThumbs thumbs;

bool sdBegin();
bool sdSerialCommand(const String &line);

extern bool sdReady;
extern bool sdDirty;
