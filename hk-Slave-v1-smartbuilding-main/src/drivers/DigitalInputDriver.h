#ifndef DRIVERS_DIGITAL_INPUT_DRIVER_H
#define DRIVERS_DIGITAL_INPUT_DRIVER_H

#include <Arduino.h>

class DigitalInputDriver {
private:
    uint8_t _pin;
    bool _initialized;

public:
    DigitalInputDriver(uint8_t pin);
    void begin();
    bool readValue();
    bool isInitialized() const { return _initialized; }
};

#endif // DRIVERS_DIGITAL_INPUT_DRIVER_H
