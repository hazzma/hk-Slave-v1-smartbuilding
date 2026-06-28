#include "DigitalInputDriver.h"

DigitalInputDriver::DigitalInputDriver(uint8_t pin) : _pin(pin), _initialized(false) {}

void DigitalInputDriver::begin() {
    pinMode(_pin, INPUT_PULLDOWN);
    _initialized = true;
}

bool DigitalInputDriver::readValue() {
    if (!_initialized) {
        return false;
    }
    return digitalRead(_pin) == HIGH;
}
