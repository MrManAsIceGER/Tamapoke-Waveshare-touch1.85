#pragma once

// Kleine Kompatibilitaetsschicht fuer den originalen Waveshare-Displaytreiber.
// Die eigentliche Expander-Ansteuerung erfolgt ueber den zuvor getesteten tca9554.cpp.
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
