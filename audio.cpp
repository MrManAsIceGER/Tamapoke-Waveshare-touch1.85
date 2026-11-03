#include "audio.h"
#include "pin_config.h"

#include <Arduino.h>
#include <ESP_I2S.h>
#include <Preferences.h>

// ------------------------------------------------------------
// TamaPoke Audio – PCM5101 I2S-DAC
//
// DOUT = GPIO47, LRCK = GPIO38, BCLK = GPIO48.
// Kein ES8311-I2C-Setup und kein MCLK erforderlich.
// ------------------------------------------------------------

#define SAMPLE_RATE 16000

static I2SClass i2s;

static bool gReady = false;
static bool gOn = true;
static bool gSleeping = false;
static bool gIdle = false;

static uint32_t gAmpHoldUntil = 0;
static bool gAmpHold = false;

static QueueHandle_t gQ = nullptr;

struct Note {
  uint16_t f;
  uint16_t ms;
};

static const Note N_TAP[] = {{880, 35}};
static const Note N_EAT[] = {{660, 45}, {0, 12}, {660, 45}};
static const Note N_PLAY[] = {{784, 45}, {988, 60}};
static const Note N_HEART[] = {{1047, 55}, {1319, 90}};
static const Note N_HATCH[] = {{523, 80}, {659, 80}, {784, 110}, {1047, 170}};
static const Note N_EVOLVE[] = {{523, 80}, {659, 80}, {784, 80}, {1047, 90}, {1319, 230}};
static const Note N_MEDAL[] = {{784, 70}, {0, 25}, {784, 70}, {0, 25}, {1047, 200}};
static const Note N_DENY[] = {{300, 110}, {200, 170}};
static const Note N_BYE[] = {{784, 150}, {659, 150}, {523, 280}};
static const Note N_LEVEL[] = {{784, 70}, {1047, 130}};

struct SfxDef {
  const Note *n;
  uint8_t len;
};

static const SfxDef SFX[SFX_COUNT] = {
  {N_TAP, 1},
  {N_EAT, 3},
  {N_PLAY, 2},
  {N_HEART, 2},
  {N_HATCH, 4},
  {N_EVOLVE, 5},
  {N_MEDAL, 5},
  {N_DENY, 2},
  {N_BYE, 3},
  {N_LEVEL, 2}
};

static int16_t buf[256 * 2];

static void updateAmplifierPower() {
  // No confirmed enable GPIO is defined for this hardware.
  // The PCM5101 therefore remains active without GPIO control.
  if (gAmpHold &&
      (int32_t)(gAmpHoldUntil - millis()) <= 0) {
    gAmpHold = false;
  }
}

static void playTone(uint16_t f, uint16_t ms) {
  int total = SAMPLE_RATE * ms / 1000;
  int half = f ? (SAMPLE_RATE / (2 * f)) : 0;

  const int16_t amp = 5000;
  int phase = 0;
  int done = 0;
  bool high = true;

  while (done < total) {
    int n = total - done;
    if (n > 256) n = 256;

    for (int i = 0; i < n; i++) {
      int16_t s = 0;

      if (f) {
        s = high ? amp : -amp;
        int idx = done + i;

        if (idx < 64) {
          s = (int16_t)(s * idx / 64);
        } else if (idx > total - 96) {
          s = (int16_t)(s * (total - idx) / 96);
        }

        if (++phase >= half) {
          phase = 0;
          high = !high;
        }
      }

      buf[i * 2] = s;
      buf[i * 2 + 1] = s;
    }

    i2s.write((uint8_t *)buf, n * 4);
    done += n;
  }
}

static void audioTask(void *) {
  uint8_t id;

  for (;;) {
    if (xQueueReceive(gQ, &id, portMAX_DELAY)) {
      if (!gOn || gSleeping || !gReady || id >= SFX_COUNT) {
        continue;
      }

      const SfxDef &d = SFX[id];
      for (uint8_t i = 0; i < d.len; i++) {
        playTone(d.n[i].f, d.n[i].ms);
      }

      updateAmplifierPower();
    }
  }
}

void audioBegin() {
  gReady = false;

  // ESP32 Arduino Core 3.3.12:
  // I2SClass::setPins() returns void, not a bool.
  // Therefore, its result must not be tested with !.
  i2s.setPins(
    I2S_BCLK,
    I2S_LRCK,
    I2S_DOUT,
    -1,
    -1
  );

  if (!i2s.begin(
        I2S_MODE_STD,
        SAMPLE_RATE,
        I2S_DATA_BIT_WIDTH_16BIT,
        I2S_SLOT_MODE_STEREO,
        I2S_STD_SLOT_BOTH
      )) {
    Serial.println("I2S Initialisierung fehlgeschlagen");
    return;
  }

  Preferences p;
  if (p.begin("tamapoke", true)) {
    gOn = p.getBool("snd", true);
    p.end();
  }

  gQ = xQueueCreate(8, sizeof(uint8_t));
  if (!gQ) {
    Serial.println("Audio Queue konnte nicht erstellt werden");
    return;
  }

  BaseType_t taskOk = xTaskCreatePinnedToCore(
    audioTask,
    "audio",
    4096,
    nullptr,
    1,
    nullptr,
    0
  );

  if (taskOk != pdPASS) {
    vQueueDelete(gQ);
    gQ = nullptr;
    Serial.println("Audio Task konnte nicht gestartet werden");
    return;
  }

  gReady = true;
  Serial.println("PCM5101 Audio bereit");
  sfxPlay(SFX_HATCH);
}

void sfxPlay(uint8_t id) {
  if (!gReady || !gOn || !gQ || id >= SFX_COUNT) {
    return;
  }

  if (gIdle) {
    gAmpHoldUntil = millis() + 4000;
    gAmpHold = true;
    updateAmplifierPower();
  }

  xQueueSend(gQ, &id, 0);
}

void audioSetEnabled(bool on) {
  gOn = on;

  if (!on && gQ) {
    xQueueReset(gQ);
  }

  updateAmplifierPower();

  Preferences p;
  if (p.begin("tamapoke", false)) {
    p.putBool("snd", on);
    p.end();
  }
}

bool audioEnabled() {
  return gOn;
}

void audioSetSleeping(bool sleeping) {
  if (gSleeping == sleeping) {
    return;
  }

  gSleeping = sleeping;

  if (sleeping && gQ) {
    xQueueReset(gQ);
  }

  updateAmplifierPower();
}

bool audioBusy() {
  if (gAmpHold &&
      (int32_t)(gAmpHoldUntil - millis()) > 0) {
    return true;
  }

  return false;
}

void audioSetIdle(bool idle) {
  gIdle = idle;
  updateAmplifierPower();
}
