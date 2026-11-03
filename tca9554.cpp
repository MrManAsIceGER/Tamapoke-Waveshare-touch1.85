
#include "tca9554.h"
#include "pin_config.h"

// TCA9554 Register
static constexpr uint8_t REG_INPUT     = 0x00;
static constexpr uint8_t REG_OUTPUT    = 0x01;
static constexpr uint8_t REG_POLARITY  = 0x02;
static constexpr uint8_t REG_CONFIG    = 0x03;

// TCA9554-Pinrichtung:
// EXIO1 = P0, EXIO2 = P1, ... EXIO8 = P7.

static TwoWire *tcaWire = nullptr;
static uint8_t outputState = 0xFF;
static bool initialized = false;


// ------------------------------------------------------------
// Interne Hilfsfunktionen
// ------------------------------------------------------------

static bool exioToMask(uint8_t exio, uint8_t &mask)
{
    if (exio < 1 || exio > 8) {
        return false;
    }

    mask = (uint8_t)(1U << (exio - 1));
    return true;
}


static bool writeRegister(uint8_t reg, uint8_t value)
{
    if (!tcaWire) {
        return false;
    }

    tcaWire->beginTransmission(TCA9554_ADDR);
    tcaWire->write(reg);
    tcaWire->write(value);

    return tcaWire->endTransmission() == 0;
}


static bool readRegister(uint8_t reg, uint8_t &value)
{
    if (!tcaWire) {
        return false;
    }

    tcaWire->beginTransmission(TCA9554_ADDR);
    tcaWire->write(reg);

    if (tcaWire->endTransmission(false) != 0) {
        return false;
    }

    if (tcaWire->requestFrom(
            (uint8_t)TCA9554_ADDR,
            (uint8_t)1) != 1) {
        return false;
    }

    value = tcaWire->read();
    return true;
}


// ------------------------------------------------------------
// Initialisierung
// ------------------------------------------------------------

bool tca9554Begin(TwoWire &wire)
{
    tcaWire = &wire;
    initialized = false;

    uint8_t config;
    uint8_t output;

    // Prüfen, ob der Expander erreichbar ist.
    if (!readRegister(REG_CONFIG, config)) {
        return false;
    }

    if (!readRegister(REG_OUTPUT, output)) {
        return false;
    }

    // Reset-Ausgänge:
    // EXIO1 = Touch-Reset
    // EXIO2 = LCD-Reset
    uint8_t resetMask = 0;

    uint8_t mask;

    if (!exioToMask(EXIO_TP_RESET, mask)) {
        return false;
    }
    resetMask |= mask;

    if (!exioToMask(EXIO_LCD_RESET, mask)) {
        return false;
    }
    resetMask |= mask;

    // Reset-Leitungen zunächst auf HIGH setzen.
    // HIGH bedeutet: Reset nicht aktiv.
    outputState = output | resetMask;

    // Ausgangszustand zuerst schreiben, damit beim Umschalten
    // der Pins auf Ausgang kein ungewollter LOW-Impuls entsteht.
    if (!writeRegister(REG_OUTPUT, outputState)) {
        return false;
    }

    // Nur die Reset-Pins als Ausgänge konfigurieren.
    // Alle anderen Pinrichtungen bleiben unverändert.
    config &= (uint8_t)~resetMask;

    if (!writeRegister(REG_CONFIG, config)) {
        return false;
    }

    initialized = true;
    return true;
}


// ------------------------------------------------------------
// Ausgang setzen
// ------------------------------------------------------------

bool tca9554SetPin(uint8_t exio, bool level)
{
    if (!initialized) {
        return false;
    }

    uint8_t mask;

    if (!exioToMask(exio, mask)) {
        return false;
    }

    if (level) {
        outputState |= mask;
    } else {
        outputState &= (uint8_t)~mask;
    }

    return writeRegister(REG_OUTPUT, outputState);
}


// ------------------------------------------------------------
// Eingang lesen
// ------------------------------------------------------------

bool tca9554ReadPin(uint8_t exio, bool &level)
{
    if (!initialized) {
        return false;
    }

    uint8_t mask;

    if (!exioToMask(exio, mask)) {
        return false;
    }

    uint8_t inputState;

    if (!readRegister(REG_INPUT, inputState)) {
        return false;
    }

    level = (inputState & mask) != 0;
    return true;
}


// ------------------------------------------------------------
// Reset-Impuls
// ------------------------------------------------------------

bool tca9554PulseReset(
    uint8_t exio,
    uint16_t lowMs,
    uint16_t settleMs)
{
    if (!initialized) {
        return false;
    }

    // Reset aktivieren.
    if (!tca9554SetPin(exio, LOW)) {
        return false;
    }

    delay(lowMs);

    // Reset freigeben.
    if (!tca9554SetPin(exio, HIGH)) {
        return false;
    }

    delay(settleMs);

    return true;
}


// ------------------------------------------------------------
// Status
// ------------------------------------------------------------

bool tca9554Ready()
{
    return initialized;
}
