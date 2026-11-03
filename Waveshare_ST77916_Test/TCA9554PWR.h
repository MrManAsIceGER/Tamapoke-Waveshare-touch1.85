#pragma once

// Small compatibility layer for the original Waveshare display driver.
// The actual expander control is handled by the previously tested tca9554.cpp.
#include <Arduino.h>
#include "tca9554.h"

#define Low       LOW
#define High      HIGH
#define EXIO_PIN2 2

static inline void Set_EXIO(uint8_t pin, uint8_t state)
{
    if (!tca9554SetPin(pin, state == HIGH)) {
        Serial.printf("TCA9554: Fehler beim Schalten von EXIO%u\n", pin);
    }
}
