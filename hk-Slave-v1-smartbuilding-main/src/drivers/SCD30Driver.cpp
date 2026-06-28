#include "SCD30Driver.h"

SCD30Driver::SCD30Driver() : _initialized(false) {}

bool SCD30Driver::begin() {
    // Initialize standard ESP32-C3 I2C pins: SDA=8, SCL=7 at 100kHz
    Wire.begin(8, 7);
    Wire.setClock(100000);

    // SCD30 requires Wire to be initialized. 
    // SparkFun SCD30 begin() returns true if sensor is detected.
    _initialized = _scd.begin(Wire);
    return _initialized;
}

bool SCD30Driver::dataAvailable() {
    if (!_initialized) {
        return false;
    }
    return _scd.dataAvailable();
}

uint16_t SCD30Driver::readCO2() {
    if (!_initialized) {
        return 0;
    }
    // _scd.getCO2() returns a uint16_t or float depending on version
    return static_cast<uint16_t>(_scd.getCO2());
}
