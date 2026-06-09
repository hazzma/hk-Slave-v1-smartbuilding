#include "RelayModule.h"
#include "../config/Pins.h"

RelayModule::RelayModule()
    : _enabled(false), _status(MODULE_DISABLED), _lastError(0) {
    _drivers[0] = new RelayDriver(PIN_RELAY_1);
    _drivers[1] = new RelayDriver(PIN_RELAY_2);

    for (int i = 0; i < RELAY_MAX_CHANNELS; i++) {
        _cachedState[i] = 0xFFFE;
        _desiredState[i] = false;
        _appliedState[i] = false;
        _channelAssigned[i] = false;
    }
}

RelayModule::~RelayModule() {
    for (int i = 0; i < RELAY_MAX_CHANNELS; i++) {
        delete _drivers[i];
    }
}

bool RelayModule::begin() {
    _status = MODULE_INIT;
    for (int i = 0; i < RELAY_MAX_CHANNELS; i++) {
        if (_channelAssigned[i] && !_drivers[i]->isInitialized()) {
            _drivers[i]->begin(_desiredState[i]);
            _appliedState[i] = _desiredState[i];
            _cachedState[i] = _desiredState[i] ? 1 : 0;
        }
    }
    _status = MODULE_READY;
    return true;
}

void RelayModule::update(uint32_t now_ms) {
    if (!_enabled) {
        _status = MODULE_DISABLED;
        return;
    }

    _status = MODULE_READY;
    for (int i = 0; i < RELAY_MAX_CHANNELS; i++) {
        if (!_channelAssigned[i]) {
            _cachedState[i] = 0xFFFE;
            continue;
        }

        if (!_drivers[i]->isInitialized()) {
            _drivers[i]->begin(_desiredState[i]);
            _appliedState[i] = _desiredState[i];
        }

        if (_appliedState[i] != _desiredState[i]) {
            _drivers[i]->writeState(_desiredState[i]);
            _appliedState[i] = _desiredState[i];
        }
        _cachedState[i] = _desiredState[i] ? 1 : 0;
    }

    _lastError = 0;
}

void RelayModule::setEnabled(bool enabled) {
    _enabled = enabled;
    if (!_enabled) {
        _status = MODULE_DISABLED;
    }
}

void RelayModule::setChannelAssignment(uint8_t chIndex, bool assigned) {
    if (chIndex >= RELAY_MAX_CHANNELS) {
        return;
    }

    _channelAssigned[chIndex] = assigned;
    if (!assigned) {
        if (_drivers[chIndex]->isInitialized()) {
            _drivers[chIndex]->writeState(false);
        }
        _desiredState[chIndex] = false;
        _appliedState[chIndex] = false;
        _cachedState[chIndex] = 0xFFFE;
        return;
    }

    _cachedState[chIndex] = _desiredState[chIndex] ? 1 : 0;
    if (_enabled && !_drivers[chIndex]->isInitialized()) {
        _drivers[chIndex]->begin(_desiredState[chIndex]);
        _appliedState[chIndex] = _desiredState[chIndex];
    }
}

bool RelayModule::setRelayState(uint8_t chIndex, bool on) {
    if (chIndex >= RELAY_MAX_CHANNELS || !_channelAssigned[chIndex]) {
        _lastError = 4;
        return false;
    }

    _desiredState[chIndex] = on;
    _cachedState[chIndex] = on ? 1 : 0;
    _lastError = 0;
    return true;
}

uint16_t RelayModule::getRelayState(uint8_t chIndex) const {
    if (chIndex >= RELAY_MAX_CHANNELS) {
        return 0xFFFE;
    }
    return _cachedState[chIndex];
}

bool RelayModule::hasValidData() const {
    if (!_enabled) {
        return false;
    }

    for (int i = 0; i < RELAY_MAX_CHANNELS; i++) {
        if (_channelAssigned[i]) {
            return true;
        }
    }
    return false;
}

uint16_t RelayModule::getStatus() const {
    return static_cast<uint16_t>(_status);
}
