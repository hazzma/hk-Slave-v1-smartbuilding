#include "SCD30Module.h"

SCD30Module::SCD30Module() 
    : _driver(new SCD30Driver()), 
      _cachedCO2(0xFFFE), 
      _lastReadMs(0), 
      _assigned(false), 
      _error(false), 
      _enabled(false), 
      _status(MODULE_DISABLED), 
      _lastError(0) {}

SCD30Module::~SCD30Module() {
    delete _driver;
}

bool SCD30Module::begin() {
    _status = MODULE_INIT;
    bool success = true;
    if (_assigned) {
        if (!_driver->begin()) {
            _error = true;
            success = false;
        }
        _lastReadMs = 0;
    }
    _status = success ? MODULE_READY : MODULE_ERROR;
    return success;
}

void SCD30Module::update(uint32_t now_ms) {
    if (!_enabled) {
        _status = MODULE_DISABLED;
        return;
    }

    if (!_assigned) {
        _cachedCO2 = 0xFFFE; // Not assigned sentinel
        _status = MODULE_READY;
        return;
    }

    _status = MODULE_READY;

    // Lazy initialization
    if (!_driver->isInitialized()) {
        if (!_driver->begin()) {
            _error = true;
            _cachedCO2 = 0xFFFF; // Error sentinel
            _status = MODULE_ERROR;
            _lastError = 1; // SLAVE_ERR_SENSOR_TIMEOUT
            return;
        }
        _lastReadMs = 0;
    }

    // Limit read to every 2000 ms and check dataReady()
    if (now_ms - _lastReadMs >= 2000 || _lastReadMs == 0) {
        if (_driver->dataAvailable()) {
            uint16_t co2 = _driver->readCO2();
            if (co2 == 0) {
                _error = true;
                _cachedCO2 = 0xFFFF;
            } else {
                _error = false;
                _cachedCO2 = co2;
            }
            _lastReadMs = now_ms;
        } else if (_lastReadMs != 0 && (now_ms - _lastReadMs > 10000)) {
            // Timeout if no new data for 10 seconds
            _error = true;
            _cachedCO2 = 0xFFFF;
        }
    }

    if (_error) {
        _status = MODULE_ERROR;
        _lastError = 1; // SLAVE_ERR_SENSOR_TIMEOUT
    } else {
        _lastError = 0;
    }
}

void SCD30Module::setEnabled(bool enabled) {
    _enabled = enabled;
    if (!_enabled) {
        _status = MODULE_DISABLED;
    }
}

void SCD30Module::setAssigned(bool assigned) {
    _assigned = assigned;
    if (!assigned) {
        _cachedCO2 = 0xFFFE; // Reset to unassigned sentinel
        _error = false;
    } else {
        if (_enabled && !_driver->isInitialized()) {
            _driver->begin();
            _lastReadMs = 0;
        }
    }
}

uint16_t SCD30Module::getCO2() const {
    return _cachedCO2;
}

uint16_t SCD30Module::getStatus() const {
    return static_cast<uint16_t>(_status);
}
