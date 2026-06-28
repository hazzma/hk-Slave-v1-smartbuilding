#ifndef DRIVERS_DHT22_DRIVER_H
#define DRIVERS_DHT22_DRIVER_H

#include <Arduino.h>
#include <DHT.h>

class DHT22Driver {
private:
    uint8_t _pin;
    DHT _dht;
    bool _initialized;

public:
    DHT22Driver(uint8_t pin);
    void begin();
    float readTemperature();
    bool isInitialized() const { return _initialized; }
};

#endif // DRIVERS_DHT22_DRIVER_H
