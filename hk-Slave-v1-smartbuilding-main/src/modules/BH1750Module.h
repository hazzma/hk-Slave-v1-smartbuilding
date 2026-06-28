#ifndef MODULES_BH1750_MODULE_H
#define MODULES_BH1750_MODULE_H

#include "ModuleInterface.h"
#include "../drivers/BH1750Driver.h"
#include "../core/ModuleStatus.h"

#define LUX_MAX_CHANNELS 4

class BH1750Module : public ModuleInterface {
private:
    BH1750Driver* _drivers[LUX_MAX_CHANNELS];
    uint16_t _cachedLux[LUX_MAX_CHANNELS];
    uint32_t _lastReadMs[LUX_MAX_CHANNELS];
    bool _channelAssigned[LUX_MAX_CHANNELS];
    bool _channelError[LUX_MAX_CHANNELS];
    
    bool _enabled;
    ModuleStatus _status;
    uint16_t _lastError;

public:
    BH1750Module();
    ~BH1750Module();

    bool begin() override;
    void update(uint32_t now_ms) override;
    
    bool isEnabled() const override { return _enabled; }
    void setEnabled(bool enabled) override;
    bool isBusy() const override { return false; }
    bool hasValidData() const override;
    uint16_t getStatus() const override;
    uint16_t getLastError() const override { return _lastError; }

    /**
     * @brief Set assignment for a specific channel.
     * If assigned, initializes the driver dynamically.
     */
    void setChannelAssignment(uint8_t chIndex, bool assigned);
    
    /**
     * @brief Get lux reading for a channel.
     * Returns sentinel values if not assigned or in error.
     */
    uint16_t getLux(uint8_t chIndex) const;
};

#endif // MODULES_BH1750_MODULE_H
