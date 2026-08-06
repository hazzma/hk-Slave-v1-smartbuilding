#include "BH1750Module.h"

BH1750Module::BH1750Module() 
    : _enabled(false), _status(MODULE_DISABLED), _lastError(0) {
    
    // Address 0x23 is primary (Lux 1), 0x5C is secondary (Lux 2)
    _drivers[0] = new BH1750Driver(0x23);
    _drivers[1] = new BH1750Driver(0x5C);
    _drivers[2] = nullptr; // Not supportable directly without I2C Mux
    _drivers[3] = nullptr; // Not supportable directly without I2C Mux

    for (int i = 0; i < LUX_MAX_CHANNELS; i++) {
        _cachedLux[i] = 0xFFFE; // default not assigned sentinel
        _lastReadMs[i] = 0;
        _channelAssigned[i] = false;
        _channelError[i] = false;
    }
}

BH1750Module::~BH1750Module() {
    for (int i = 0; i < LUX_MAX_CHANNELS; i++) {
        if (_drivers[i] != nullptr) {
            delete _drivers[i];
        }
    }
}

bool BH1750Module::begin() {
    _status = MODULE_INIT;
    bool success = true;
    for (int i = 0; i < LUX_MAX_CHANNELS; i++) {
        if (_channelAssigned[i]) {
            if (_drivers[i] != nullptr) {
                if (!_drivers[i]->begin()) {
                    _channelError[i] = true;
                    success = false;
                }
            } else {
                _channelError[i] = true; // Channels 3 and 4 are unsupported physically
                success = false;
            }
            _lastReadMs[i] = 0;
        }
    }
    _status = success ? MODULE_READY : MODULE_ERROR;
    return success;
}

void BH1750Module::update(uint32_t now_ms) {
    if (!_enabled) {
        _status = MODULE_DISABLED;
        return;
    }

    _status = MODULE_READY;
    bool has_active_error = false;

    for (int i = 0; i < LUX_MAX_CHANNELS; i++) {
        if (!_channelAssigned[i]) {
            _cachedLux[i] = 0xFFFE; // Not assigned sentinel
            continue;
        }

        // Handle unsupported channels
        if (_drivers[i] == nullptr) {
            _channelError[i] = true;
            _cachedLux[i] = 0xFFFF; // Error sentinel
            has_active_error = true;
            continue;
        }

        // Lazy initialization
        if (!_drivers[i]->isInitialized()) {
            if (!_drivers[i]->begin()) {
                _channelError[i] = true;
                _cachedLux[i] = 0xFFFF;
                has_active_error = true;
                continue;
            }
            _lastReadMs[i] = 0;
        }

        // Poll every 1000 ms normally; back off to every 30000 ms once a
        // channel is in a persistent error state (e.g. sensor not wired),
        // so a disconnected sensor doesn't flood Serial with NACK errors
        // from the BH1750 library every second.
        uint32_t pollIntervalMs = _channelError[i] ? 30000 : 1000;
        if (now_ms - _lastReadMs[i] >= pollIntervalMs || _lastReadMs[i] == 0) {
            float lux = _drivers[i]->readLightLevel();
            
            if (lux < 0.0f) {
                _channelError[i] = true;
                _cachedLux[i] = 0xFFFF; // Error sentinel
            } else {
                _channelError[i] = false;
                // Clip reading to fit in 16-bit register
                if (lux > 65530.0f) {
                    _cachedLux[i] = 65530;
                } else {
                    _cachedLux[i] = static_cast<uint16_t>(lux);
                }
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

void BH1750Module::setEnabled(bool enabled) {
    _enabled = enabled;
    if (!_enabled) {
        _status = MODULE_DISABLED;
    }
}

void BH1750Module::setChannelAssignment(uint8_t chIndex, bool assigned) {
    if (chIndex >= LUX_MAX_CHANNELS) return;
    
    _channelAssigned[chIndex] = assigned;
    if (!assigned) {
        _cachedLux[chIndex] = 0xFFFE; // Reset to unassigned sentinel
        _channelError[chIndex] = false;
    } else {
        if (_enabled && _drivers[chIndex] != nullptr && !_drivers[chIndex]->isInitialized()) {
            _drivers[chIndex]->begin();
            _lastReadMs[chIndex] = 0;
        }
    }
}

uint16_t BH1750Module::getLux(uint8_t chIndex) const {
    if (chIndex >= LUX_MAX_CHANNELS) {
        return 0xFFFE;
    }
    return _cachedLux[chIndex];
}

bool BH1750Module::hasValidData() const {
    if (!_enabled) return false;
    for (int i = 0; i < LUX_MAX_CHANNELS; i++) {
        if (_channelAssigned[i] && !_channelError[i]) {
            return true;
        }
    }
    return false;
}

uint16_t BH1750Module::getStatus() const {
    return static_cast<uint16_t>(_status);
}
