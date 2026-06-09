#ifndef MODULES_RELAY_MODULE_H
#define MODULES_RELAY_MODULE_H

#include "ModuleInterface.h"
#include "../drivers/RelayDriver.h"
#include "../core/ModuleStatus.h"

#define RELAY_MAX_CHANNELS 2

class RelayModule : public ModuleInterface {
private:
    RelayDriver* _drivers[RELAY_MAX_CHANNELS];
    uint16_t _cachedState[RELAY_MAX_CHANNELS];
    bool _desiredState[RELAY_MAX_CHANNELS];
    bool _appliedState[RELAY_MAX_CHANNELS];
    bool _channelAssigned[RELAY_MAX_CHANNELS];
    bool _enabled;
    ModuleStatus _status;
    uint16_t _lastError;

public:
    RelayModule();
    ~RelayModule();

    bool begin() override;
    void update(uint32_t now_ms) override;

    bool isEnabled() const override { return _enabled; }
    void setEnabled(bool enabled) override;
    bool isBusy() const override { return false; }
    bool hasValidData() const override;
    uint16_t getStatus() const override;
    uint16_t getLastError() const override { return _lastError; }

    void setChannelAssignment(uint8_t chIndex, bool assigned);
    bool setRelayState(uint8_t chIndex, bool on);
    uint16_t getRelayState(uint8_t chIndex) const;
};

#endif // MODULES_RELAY_MODULE_H
