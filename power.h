
#pragma once
#include <stdint.h>

// Politica de consumo. Todo lo que decide cuanto trabaja la placa vive aqui como
// funciones puras (sin hardware), para poder probarlas en el PC (test/test_power.cpp).

// Escalones de inactividad (ms desde el ultimo toque o evento visible)
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

// ms desde `last`, sin desbordar si `last` es posterior a `now`
inline uint32_t idleSince(uint32_t now, uint32_t last) {
  int32_t d = (int32_t)(now - last);
  return d < 0 && d > -1000 ? 0 : now - last;
}

inline uint8_t dimStageFor(uint32_t idleMs) {
  return idleMs > POWER_DIM2_MS ? DIM_OFF :
         idleMs > POWER_DIM1_MS ? DIM_LOW :
         DIM_AWAKE;
}

// bateria baja: se ahorra mas
inline bool batteryLow(const PowerCtx &c) {
  return c.batPct >= 0 && c.batPct < 20 &&
         !c.charging && !c.usb;
}

inline bool batteryCritical(const PowerCtx &c) {
  return c.batPct >= 0 && c.batPct < 10 &&
         !c.charging && !c.usb;
}

// brillo del panel (0-255)
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

// ms entre frames; 0 = no se dibuja
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

// el amplificador del altavoz consume en reposo
inline bool amplifierWanted(const PowerCtx &c) {
  return !c.screenOff &&
         !c.sleeping &&
         dimStageFor(c.idleMs) == DIM_AWAKE;
}

// pausa del bucle principal en un pase que no dibuja
inline uint32_t idleDelayMs(const PowerCtx &c) {
  return c.screenOff ? 20 : 1;
}

// con la pantalla apagada o casi apagada se duerme la CPU
inline bool lightSleepAllowed(const PowerCtx &c) {
  return !c.usb &&
         (c.screenOff || dimStageFor(c.idleMs) == DIM_OFF);
}

#define POWER_SLEEP_MS 200

// cada cuanto se lee la bateria
inline uint32_t powerCacheMs(const PowerCtx &c) {
  return (c.screenOff || dimStageFor(c.idleMs) == DIM_OFF)
         ? 10000 : 2000;
}
