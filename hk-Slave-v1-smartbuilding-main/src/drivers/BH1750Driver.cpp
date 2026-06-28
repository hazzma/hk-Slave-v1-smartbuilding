#include "BH1750Driver.h"

BH1750Driver::BH1750Driver(uint8_t address) : _address(address), _initialized(false) {}

bool BH1750Driver::begin() {
    // Initialize standard ESP32-C3 I2C pins: SDA=8, SCL=7 at 100kHz
    Wire.begin(8, 7);
    Wire.setClock(100000);

    // Initialize BH1750 sensor with mode and address
    _initialized = _lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, _address, &Wire);
    return _initialized;
}

float BH1750Driver::readLightLevel() {
    if (!_initialized) {
        return -1.0f; // Return sentinel error
    }
    return _lightMeter.readLightLevel();
}
