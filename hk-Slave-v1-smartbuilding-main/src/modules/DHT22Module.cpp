#include "DHT22Module.h"
#include "../config/Pins.h"
#include <cmath>

DHT22Module::DHT22Module() 
    : _enabled(false), _status(MODULE_DISABLED), _lastError(0) {
    
    // Map drivers to physical pins from Pins.h
    _drivers[0] = new DHT22Driver(PIN_DHT22_1);
    _drivers[1] = new DHT22Driver(PIN_DHT22_2);
    _drivers[2] = new DHT22Driver(PIN_DHT22_3);
    _drivers[3] = new DHT22Driver(PIN_DHT22_4);

    for (int i = 0; i < DHT_MAX_CHANNELS; i++) {
        _cachedTempX10[i] = -32767; // default not assigned sentinel
        _lastReadMs[i] = 0;
        _channelAssigned[i] = false;
        _channelError[i] = false;
    }
}

DHT22Module::~DHT22Module() {
    for (int i = 0; i < DHT_MAX_CHANNELS; i++) {
        delete _drivers[i];
    }
}

bool DHT22Module::begin() {
    _status = MODULE_INIT;
    bool success = true;
    for (int i = 0; i < DHT_MAX_CHANNELS; i++) {
        if (_channelAssigned[i]) {
            _drivers[i]->begin();
            _lastReadMs[i] = 0; // Force immediate update in loop
        }
    }
    _status = MODULE_READY;
    return success;
}

void DHT22Module::update(uint32_t now_ms) {
    if (!_enabled) {
        _status = MODULE_DISABLED;
        return;
    }

    _status = MODULE_READY;
    bool has_active_error = false;

    for (int i = 0; i < DHT_MAX_CHANNELS; i++) {
        if (!_channelAssigned[i]) {
            _cachedTempX10[i] = -32767; // Not assigned sentinel
            continue;
        }

        // Lazy initialize driver if it hasn't been initialized yet
        if (!_drivers[i]->isInitialized()) {
            _drivers[i]->begin();
            _lastReadMs[i] = 0;
        }

        // Limit reading frequency to >= 2000 ms
        if (now_ms - _lastReadMs[i] >= 2000 || _lastReadMs[i] == 0) {
            float temp = _drivers[i]->readTemperature();
            
            if (std::isnan(temp)) {
                _channelError[i] = true;
                _cachedTempX10[i] = -32768; // Error sentinel
            } else {
                _channelError[i] = false;
                _cachedTempX10[i] = static_cast<int16_t>(roundf(temp * 10.0f));
            }
            _lastReadMs[i] = now_ms;
        }

        if (_channelError[i]) {
            has_active_error = true;
        }
    }

    if (has_active_error) {
        _status = MODULE_ERROR;
        _lastError = 1; // SLAVE_ERR_SENSOR_TIMEOUT
    } else {
        _lastError = 0;
    }
}

void DHT22Module::setEnabled(bool enabled) {
    _enabled = enabled;
    if (!_enabled) {
        _status = MODULE_DISABLED;
    }
}

void DHT22Module::setChannelAssignment(uint8_t chIndex, bool assigned) {
    if (chIndex >= DHT_MAX_CHANNELS) return;
    
    _channelAssigned[chIndex] = assigned;
    if (!assigned) {
        _cachedTempX10[chIndex] = -32767; // Reset to unassigned sentinel
        _channelError[chIndex] = false;
    } else {
        // If system is already running, initialize the driver dynamically
        if (_enabled && !_drivers[chIndex]->isInitialized()) {
            _drivers[chIndex]->begin();
            _lastReadMs[chIndex] = 0;
        }
    }
}

int16_t DHT22Module::getTemperatureX10(uint8_t chIndex) const {
    if (chIndex >= DHT_MAX_CHANNELS) {
        return -32767; // Not assigned
    }
    return _cachedTempX10[chIndex];
}

bool DHT22Module::hasValidData() const {
    if (!_enabled) return false;
    // At least one channel is assigned and has no error
    for (int i = 0; i < DHT_MAX_CHANNELS; i++) {
        if (_channelAssigned[i] && !_channelError[i]) {
            return true;
        }
    }
    return false;
}

uint16_t DHT22Module::getStatus() const {
    return static_cast<uint16_t>(_status);
}
