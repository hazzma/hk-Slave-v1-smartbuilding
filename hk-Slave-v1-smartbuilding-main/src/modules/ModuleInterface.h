#ifndef MODULES_MODULE_INTERFACE_H
#define MODULES_MODULE_INTERFACE_H

#include <Arduino.h>

class ModuleInterface {
public:
    virtual ~ModuleInterface() {}
    
    /**
     * @brief Initialize the module hardware/settings.
     * @return true if initialization succeeded, false if error.
     */
    virtual bool begin() = 0;
    
    /**
     * @brief Periodic update function called from the main scheduler.
     * Must be non-blocking.
     * @param now_ms Current system time in milliseconds.
     */
    virtual void update(uint32_t now_ms) = 0;
    
    /**
     * @brief Check if the module is enabled by the capability assignment.
     */
    virtual bool isEnabled() const = 0;
    
    /**
     * @brief Set enabled/disabled state (called when capability assignment changes).
     */
    virtual void setEnabled(bool enabled) = 0;
    
    /**
     * @brief Check if the module is busy (e.g. IR sender active).
     */
    virtual bool isBusy() const = 0;
    
    /**
     * @brief Check if cached data is valid and can be exposed.
     */
    virtual bool hasValidData() const = 0;
    
    /**
     * @brief Get the ModuleStatus enum representation.
     */
    virtual uint16_t getStatus() const = 0;
    
    /**
     * @brief Get the last error code of this module.
     */
    virtual uint16_t getLastError() const = 0;
};

#endif // MODULES_MODULE_INTERFACE_H
