#ifndef MODULES_PRESENCE_DIGITAL_MODULE_H
#define MODULES_PRESENCE_DIGITAL_MODULE_H

#include "ModuleInterface.h"
#include "../drivers/DigitalInputDriver.h"
#include "../core/ModuleStatus.h"

#define PRESENCE_MAX_CHANNELS 4

class PresenceDigitalModule : public ModuleInterface {
private:
    DigitalInputDriver* _drivers[PRESENCE_MAX_CHANNELS];
    uint16_t _cachedPresence[PRESENCE_MAX_CHANNELS];
    
    // Stable state filtering parameters
    bool _lastRawState[PRESENCE_MAX_CHANNELS];
    uint32_t _lastStateChangeMs[PRESENCE_MAX_CHANNELS];
    
    bool _channelAssigned[PRESENCE_MAX_CHANNELS];
    bool _channelError[PRESENCE_MAX_CHANNELS];
    
    bool _enabled;
    ModuleStatus _status;
    uint16_t _lastError;

public:
    PresenceDigitalModule();
    ~PresenceDigitalModule();

    bool begin() override;
    void update(uint32_t now_ms) override;
    
    bool isEnabled() const override { return _enabled; }
    void setEnabled(bool enabled) override;
    bool isBusy() const override { return false; }
    bool hasValidData() const override;
    uint16_t getStatus() const override;
    uint16_t getLastError() const override { return _lastError; }

    /**
     * @brief Set assignment for a specific presence channel.
     * If assigned, initializes the driver dynamically.
     */
    void setChannelAssignment(uint8_t chIndex, bool assigned);
    
    /**
     * @brief Get presence state (0 = no presence, 1 = presence).
     * Returns sentinel values if not assigned or in error.
     */
    uint16_t getPresenceState(uint8_t chIndex) const;
};

#endif // MODULES_PRESENCE_DIGITAL_MODULE_H
