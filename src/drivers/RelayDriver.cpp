#include "RelayDriver.h"

RelayDriver::RelayDriver(uint8_t pin, bool activeHigh)
    : _pin(pin), _initialized(false), _activeHigh(activeHigh) {}

void RelayDriver::begin(bool initialState) {
    pinMode(_pin, OUTPUT);
    _initialized = true;
    writeState(initialState);
}

void RelayDriver::writeState(bool on) {
    if (!_initialized) {
        return;
    }

    bool level = _activeHigh ? on : !on;
    digitalWrite(_pin, level ? HIGH : LOW);
}
