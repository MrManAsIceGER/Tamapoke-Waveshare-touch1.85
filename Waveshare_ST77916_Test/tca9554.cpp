#include "tca9554.h"

namespace {
constexpr uint8_t TCA9554_ADDRESS = 0x20;
constexpr uint8_t REG_INPUT       = 0x00;
constexpr uint8_t REG_OUTPUT      = 0x01;
constexpr uint8_t REG_CONFIG      = 0x03;
constexpr uint8_t RESET_PINS_MASK = 0x03; // EXIO1 touch reset, EXIO2 LCD reset

TwoWire *gWire = nullptr;
uint8_t gOutputState = 0xFF;
bool gReady = false;

bool readRegister(uint8_t reg, uint8_t &value)
{
    if (!gWire) return false;

    gWire->beginTransmission(TCA9554_ADDRESS);
    gWire->write(reg);
    if (gWire->endTransmission(false) != 0) return false;

    if (gWire->requestFrom((int)TCA9554_ADDRESS, 1) != 1) return false;
    value = (uint8_t)gWire->read();
    return true;
}

bool writeRegister(uint8_t reg, uint8_t value)
{
    if (!gWire) return false;

    gWire->beginTransmission(TCA9554_ADDRESS);
    gWire->write(reg);
    gWire->write(value);
    return gWire->endTransmission() == 0;
}
}

bool tca9554Begin(TwoWire &wire)
{
    gWire = &wire;
    gReady = false;

    uint8_t config = 0xFF;
    uint8_t output = 0xFF;

    if (!readRegister(REG_CONFIG, config) || !readRegister(REG_OUTPUT, output)) {
        return false;
    }

    // Set reset outputs HIGH before configuring them as outputs.
    output |= RESET_PINS_MASK;
    if (!writeRegister(REG_OUTPUT, output)) return false;

    // Configure only EXIO1 and EXIO2 as outputs. Leave all other expander pins unchanged.
    config &= (uint8_t)~RESET_PINS_MASK;
    if (!writeRegister(REG_CONFIG, config)) return false;

    gOutputState = output;
    gReady = true;
    return true;
}

bool tca9554SetPin(uint8_t exio, bool level)
{
    if (!gReady || exio < 1 || exio > 8) return false;

    const uint8_t mask = (uint8_t)(1U << (exio - 1));
    if (level) gOutputState |= mask;
    else       gOutputState &= (uint8_t)~mask;

    return writeRegister(REG_OUTPUT, gOutputState);
}

bool tca9554Ready()
{
    return gReady;
}
