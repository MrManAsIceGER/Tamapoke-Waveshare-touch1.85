
#pragma once
#include <stdint.h>

// Power policy. All decisions about how much work the board performs live here as
// pure functions (no hardware), so they can be tested on a PC (test/test_power.cpp).

// Inactivity stages (ms since the last touch or visible event)
#define POWER_CALM_MS 60000UL
#define POWER_DIM1_MS 90000UL
#define POWER_DIM2_MS 300000UL

enum DimStage : uint8_t {
  DIM_AWAKE = 0,
  DIM_LOW = 1,
  DIM_OFF = 2
};

struct PowerCtx {
  uint32_t idleMs;
  bool screenOff;
  bool sleeping;
  bool gameOpen;
  bool usb;
  bool charging;
  int batPct;
};

// Milliseconds since `last`, without underflow if `last` is later than `now`
inline uint32_t idleSince(uint32_t now, uint32_t last) {
  int32_t d = (int32_t)(now - last);
  return d < 0 && d > -1000 ? 0 : now - last;
}

inline uint8_t dimStageFor(uint32_t idleMs) {
  return idleMs > POWER_DIM2_MS ? DIM_OFF :
         idleMs > POWER_DIM1_MS ? DIM_LOW :
         DIM_AWAKE;
}

// Low battery: save more power
inline bool batteryLow(const PowerCtx &c) {
  return c.batPct >= 0 && c.batPct < 20 &&
         !c.charging && !c.usb;
}

inline bool batteryCritical(const PowerCtx &c) {
  return c.batPct >= 0 && c.batPct < 10 &&
         !c.charging && !c.usb;
}

// Panel brightness (0-255)
inline uint8_t brightnessFor(const PowerCtx &c) {
  if (c.screenOff) return 0;

  uint8_t stage = dimStageFor(c.idleMs);
  uint8_t t = c.sleeping ? 25 : (c.usb ? 180 : 145);

  if (stage == DIM_LOW) {
    t = c.sleeping ? 10 : 60;
  } else if (stage == DIM_OFF) {
    t = 8;
  }

  if (batteryCritical(c) && t > 60) {
    t = 60;
  } else if (batteryLow(c) && t > 100) {
    t = 100;
  }

  return t;
}

// Milliseconds between frames; 0 = do not draw
inline uint32_t frameIntervalMs(const PowerCtx &c) {
  if (c.screenOff) return 0;
  if (c.gameOpen) return 85;

  uint32_t ms;
  uint8_t stage = dimStageFor(c.idleMs);

  if (stage == DIM_OFF) {
    ms = 1000;
  } else if (c.sleeping && c.idleMs > POWER_CALM_MS) {
    ms = 500;
  } else if (stage == DIM_LOW) {
    ms = 250;
  } else {
    ms = c.idleMs > POWER_CALM_MS ? 150 : 100;
  }

  if (batteryLow(c)) ms *= 2;

  return ms;
}

// The speaker amplifier consumes power while idle
inline bool amplifierWanted(const PowerCtx &c) {
  return !c.screenOff &&
         !c.sleeping &&
         dimStageFor(c.idleMs) == DIM_AWAKE;
}

// Pause the main loop during an iteration that does not draw
inline uint32_t idleDelayMs(const PowerCtx &c) {
  return c.screenOff ? 20 : 1;
}

// With the display off or almost off, put the CPU to sleep
inline bool lightSleepAllowed(const PowerCtx &c) {
  return !c.usb &&
         (c.screenOff || dimStageFor(c.idleMs) == DIM_OFF);
}

#define POWER_SLEEP_MS 200

// How often to read the battery
inline uint32_t powerCacheMs(const PowerCtx &c) {
  return (c.screenOff || dimStageFor(c.idleMs) == DIM_OFF)
         ? 10000 : 2000;
}
