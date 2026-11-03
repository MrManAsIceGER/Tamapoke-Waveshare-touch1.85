
#pragma once

#include <Arduino.h>
#include <Wire.h>

// Initialize the TCA9554 on an I2C bus that has already been started.
// Configure EXIO1 and EXIO2 as outputs and set them HIGH.
bool tca9554Begin(TwoWire &wire = Wire);

// Set an EXIO output (numbered 1 through 8).
// HIGH = 3.3 V, LOW = 0 V.
bool tca9554SetPin(uint8_t exio, bool level);

// Read the state of an EXIO pin (numbered 1 through 8).
bool tca9554ReadPin(uint8_t exio, bool &level);

// Generate a reset pulse:
// hold LOW for lowMs, then set HIGH and wait settleMs.
bool tca9554PulseReset(
    uint8_t exio,
    uint16_t lowMs = 10,
    uint16_t settleMs = 50
);

// Returns whether initialization succeeded.
bool tca9554Ready();
