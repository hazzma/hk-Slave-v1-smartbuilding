#include "PresenceDigitalModule.h"
#include "../config/Pins.h"

PresenceDigitalModule::PresenceDigitalModule() 
    : _enabled(false), _status(MODULE_DISABLED), _lastError(0) {
    
    // Map drivers to physical pins from Pins.h
    _drivers[0] = new DigitalInputDriver(PIN_PRESENCE_1);
    _drivers[1] = new DigitalInputDriver(PIN_PRESENCE_2);
    _drivers[2] = new DigitalInputDriver(PIN_PRESENCE_3);
    _drivers[3] = new DigitalInputDriver(PIN_PRESENCE_4);

    for (int i = 0; i < PRESENCE_MAX_CHANNELS; i++) {
        _cachedPresence[i] = 0xFFFE; // default not assigned sentinel
        _lastRawState[i] = false;
        _lastStateChangeMs[i] = 0;
        _channelAssigned[i] = false;
        _channelError[i] = false;
    }
}

PresenceDigitalModule::~PresenceDigitalModule() {
    for (int i = 0; i < PRESENCE_MAX_CHANNELS; i++) {
        delete _drivers[i];
    }
}

bool PresenceDigitalModule::begin() {
    _status = MODULE_INIT;
    bool success = true;
    for (int i = 0; i < PRESENCE_MAX_CHANNELS; i++) {
        if (_channelAssigned[i]) {
            _drivers[i]->begin();
            // Fetch initial raw state
            _lastRawState[i] = _drivers[i]->readValue();
            _lastStateChangeMs[i] = millis();
            _cachedPresence[i] = _lastRawState[i] ? 1 : 0;
        }
    }
    _status = MODULE_READY;
    return success;
}

void PresenceDigitalModule::update(uint32_t now_ms) {
    if (!_enabled) {
        _status = MODULE_DISABLED;
        return;
    }

    _status = MODULE_READY;
    bool has_active_error = false;

    for (int i = 0; i < PRESENCE_MAX_CHANNELS; i++) {
        if (!_channelAssigned[i]) {
            _cachedPresence[i] = 0xFFFE; // Not assigned sentinel
            continue;
        }

        // Lazy initialize
        if (!_drivers[i]->isInitialized()) {
            _drivers[i]->begin();
            _lastRawState[i] = _drivers[i]->readValue();
            _lastStateChangeMs[i] = now_ms;
            _cachedPresence[i] = _lastRawState[i] ? 1 : 0;
        }

        bool rawState = _drivers[i]->readValue();

        if (rawState != _lastRawState[i]) {
            // State is unstable, record the transition
            _lastRawState[i] = rawState;
            _lastStateChangeMs[i] = now_ms;
        } else {
            // State is stable, check if stable duration is >= 500 ms
            if (now_ms - _lastStateChangeMs[i] >= 500) {
                _cachedPresence[i] = _lastRawState[i] ? 1 : 0;
            }
        }

        if (_channelError[i]) {
            has_active_error = true;
            _cachedPresence[i] = 0xFFFF; // Error sentinel
        }
    }

    if (has_active_error) {
        _status = MODULE_ERROR;
        _lastError = 1; // SLAVE_ERR_SENSOR_TIMEOUT
    } else {
        _lastError = 0;
    }
}

void PresenceDigitalModule::setEnabled(bool enabled) {
    _enabled = enabled;
    if (!_enabled) {
        _status = MODULE_DISABLED;
    }
}

void PresenceDigitalModule::setChannelAssignment(uint8_t chIndex, bool assigned) {
    if (chIndex >= PRESENCE_MAX_CHANNELS) return;
    
    _channelAssigned[chIndex] = assigned;
    if (!assigned) {
        _cachedPresence[chIndex] = 0xFFFE; // Reset to unassigned sentinel
        _channelError[chIndex] = false;
    } else {
        if (_enabled && !_drivers[chIndex]->isInitialized()) {
            _drivers[chIndex]->begin();
            _lastRawState[chIndex] = _drivers[chIndex]->readValue();
            _lastStateChangeMs[chIndex] = millis();
            _cachedPresence[chIndex] = _lastRawState[chIndex] ? 1 : 0;
        }
    }
}

uint16_t PresenceDigitalModule::getPresenceState(uint8_t chIndex) const {
    if (chIndex >= PRESENCE_MAX_CHANNELS) {
        return 0xFFFE;
    }
    return _cachedPresence[chIndex];
}

bool PresenceDigitalModule::hasValidData() const {
    if (!_enabled) return false;
    for (int i = 0; i < PRESENCE_MAX_CHANNELS; i++) {
        if (_channelAssigned[i] && !_channelError[i]) {
            return true;
        }
    }
    return false;
}

uint16_t PresenceDigitalModule::getStatus() const {
    return static_cast<uint16_t>(_status);
}
