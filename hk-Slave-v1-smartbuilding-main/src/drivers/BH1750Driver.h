#ifndef DRIVERS_BH1750_DRIVER_H
#define DRIVERS_BH1750_DRIVER_H

#include <Arduino.h>
#include <BH1750.h>
#include <Wire.h>

class BH1750Driver {
private:
    BH1750 _lightMeter;
    uint8_t _address;
    bool _initialized;

public:
    BH1750Driver(uint8_t address = 0x23);
    bool begin();
    float readLightLevel();
    bool isInitialized() const { return _initialized; }
    void reset() { _initialized = false; }
};

#endif // DRIVERS_BH1750_DRIVER_H
