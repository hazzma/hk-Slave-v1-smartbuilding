#include "DHT22Driver.h"

DHT22Driver::DHT22Driver(uint8_t pin) : _pin(pin), _dht(pin, DHT22), _initialized(false) {}

void DHT22Driver::begin() {
    _dht.begin();
    _initialized = true;
}

float DHT22Driver::readTemperature() {
    if (!_initialized) {
        return NAN;
    }
    return _dht.readTemperature();
}
