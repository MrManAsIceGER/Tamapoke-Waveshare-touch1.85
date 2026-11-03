
#pragma once

#include <Arduino.h>
#include <Wire.h>

// Initialisiert den TCA9554 auf dem bereits gestarteten I2C-Bus.
// EXIO1 und EXIO2 werden als Ausgänge konfiguriert und auf HIGH gesetzt.
bool tca9554Begin(TwoWire &wire = Wire);

// Setzt einen EXIO-Ausgang (Nummerierung 1 bis 8).
// HIGH = 3,3 V, LOW = 0 V.
bool tca9554SetPin(uint8_t exio, bool level);

// Liest den Zustand eines EXIO-Pins (Nummerierung 1 bis 8).
bool tca9554ReadPin(uint8_t exio, bool &level);

// Führt einen Reset-Impuls aus:
// LOW für lowMs, anschließend HIGH und settleMs warten.
bool tca9554PulseReset(
    uint8_t exio,
    uint16_t lowMs = 10,
    uint16_t settleMs = 50
);

// Gibt zurück, ob die Initialisierung erfolgreich war.
bool tca9554Ready();
