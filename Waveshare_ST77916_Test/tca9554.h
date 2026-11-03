#pragma once

#include <Arduino.h>
#include <Wire.h>

bool tca9554Begin(TwoWire &wire = Wire);
bool tca9554SetPin(uint8_t exio, bool level);
bool tca9554Ready();
