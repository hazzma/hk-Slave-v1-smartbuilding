#ifndef DRIVERS_RELAY_DRIVER_H
#define DRIVERS_RELAY_DRIVER_H

#include <Arduino.h>

class RelayDriver {
private:
    uint8_t _pin;
    bool _initialized;
    bool _activeHigh;

public:
    RelayDriver(uint8_t pin, bool activeHigh = true);
    void begin(bool initialState = false);
    void writeState(bool on);
    bool isInitialized() const { return _initialized; }
};

#endif // DRIVERS_RELAY_DRIVER_H
