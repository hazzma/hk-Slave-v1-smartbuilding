#ifndef MODULES_DHT22_MODULE_H
#define MODULES_DHT22_MODULE_H

#include "ModuleInterface.h"
#include "../drivers/DHT22Driver.h"
#include "../core/ModuleStatus.h"

#define DHT_MAX_CHANNELS 4

class DHT22Module : public ModuleInterface {
private:
    DHT22Driver* _drivers[DHT_MAX_CHANNELS];
    int16_t _cachedTempX10[DHT_MAX_CHANNELS];
    uint32_t _lastReadMs[DHT_MAX_CHANNELS];
    bool _channelAssigned[DHT_MAX_CHANNELS];
    bool _channelError[DHT_MAX_CHANNELS];
    
    bool _enabled;
    ModuleStatus _status;
    uint16_t _lastError;

public:
    DHT22Module();
    ~DHT22Module();

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
     * If assigned, initializes the driver for that channel dynamically.
     */
    void setChannelAssignment(uint8_t chIndex, bool assigned);
    
    /**
     * @brief Get temperature reading in Celsius x10 for a channel.
     * Returns sentinel values if not assigned or in error.
     */
    int16_t getTemperatureX10(uint8_t chIndex) const;
};

#endif // MODULES_DHT22_MODULE_H
