#include "ErrorManager.h"

ErrorManager::ErrorManager(DHT22Module* dht, BH1750Module* lux, SCD30Module* co2, 
                           PresenceDigitalModule* presence, RelayModule* relay, IRComboModule* ir)
    : _dhtModule(dht), _luxModule(lux), _co2Module(co2), _presenceModule(presence), _relayModule(relay), _irModule(ir) {}

void ErrorManager::begin() {
    clearError();
}

void ErrorManager::setSystemError(uint16_t errorCode) {
    runtime.setLastError(errorCode);
}

void ErrorManager::clearError() {
    runtime.setLastError(SLAVE_ERR_NONE);
}

void ErrorManager::update(uint32_t now_ms) {
    // If a system-level error is already set (e.g. bad address or MAC mismatch),
    // we preserve it. It must be manually cleared or overwritten by a new event.
    if (runtime.getLastError() != SLAVE_ERR_NONE) {
        return;
    }

    // Otherwise, poll modules for active error statuses
    if (_dhtModule != nullptr && _dhtModule->isEnabled() && _dhtModule->getStatus() == MODULE_ERROR) {
        runtime.setLastError(_dhtModule->getLastError());
        return;
    }

    if (_luxModule != nullptr && _luxModule->isEnabled() && _luxModule->getStatus() == MODULE_ERROR) {
        runtime.setLastError(_luxModule->getLastError());
        return;
    }

    if (_co2Module != nullptr && _co2Module->isEnabled() && _co2Module->getStatus() == MODULE_ERROR) {
        runtime.setLastError(_co2Module->getLastError());
        return;
    }

    if (_presenceModule != nullptr && _presenceModule->isEnabled() && _presenceModule->getStatus() == MODULE_ERROR) {
        runtime.setLastError(_presenceModule->getLastError());
        return;
    }

    if (_relayModule != nullptr && _relayModule->isEnabled() && _relayModule->getStatus() == MODULE_ERROR) {
        runtime.setLastError(_relayModule->getLastError());
        return;
    }

    if (_irModule != nullptr && _irModule->isEnabled() && _irModule->getStatus() == MODULE_ERROR) {
        runtime.setLastError(_irModule->getLastError());
        return;
    }
}
