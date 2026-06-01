#ifndef DRIVERS_SCD30_DRIVER_H
#define DRIVERS_SCD30_DRIVER_H

#include <Arduino.h>
#include <SparkFun_SCD30_Arduino_Library.h>
#include <Wire.h>

class SCD30Driver {
private:
    SCD30 _scd;
    bool _initialized;

public:
    SCD30Driver();
    bool begin();
    bool dataAvailable();
    uint16_t readCO2();
    bool isInitialized() const { return _initialized; }
};

#endif // DRIVERS_SCD30_DRIVER_H
