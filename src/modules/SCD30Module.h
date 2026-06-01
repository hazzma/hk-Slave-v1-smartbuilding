#ifndef MODULES_SCD30_MODULE_H
#define MODULES_SCD30_MODULE_H

#include "ModuleInterface.h"
#include "../drivers/SCD30Driver.h"
#include "../core/ModuleStatus.h"

class SCD30Module : public ModuleInterface {
private:
    SCD30Driver* _driver;
    uint16_t _cachedCO2;
    uint32_t _lastReadMs;
    bool _assigned;
    bool _error;
    
    bool _enabled;
    ModuleStatus _status;
    uint16_t _lastError;

public:
    SCD30Module();
    ~SCD30Module();

    bool begin() override;
    void update(uint32_t now_ms) override;
    
    bool isEnabled() const override { return _enabled; }
    void setEnabled(bool enabled) override;
    bool isBusy() const override { return false; }
    bool hasValidData() const override { return _enabled && _assigned && !_error; }
    uint16_t getStatus() const override;
    uint16_t getLastError() const override { return _lastError; }

    /**
     * @brief Assign the module (count > 0).
     */
    void setAssigned(bool assigned);
    
    /**
     * @brief Get CO2 PPM reading.
     * Returns sentinel values if not assigned or in error.
     */
    uint16_t getCO2() const;
};

#endif // MODULES_SCD30_MODULE_H
