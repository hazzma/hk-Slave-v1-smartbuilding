#ifndef CORE_COMMAND_MANAGER_H
#define CORE_COMMAND_MANAGER_H

#include <Arduino.h>
#include "SlaveRuntime.h"
#include "../modules/IRComboModule.h"

class CommandManager {
private:
    IRComboModule* _irModule;

public:
    CommandManager(IRComboModule* ir);
    
    void begin();
    
    /**
     * @brief Handle AC command request coming from Modbus writes.
     */
    bool handleAcWrite(uint8_t channel, bool power, uint16_t tempX10, uint8_t mode,
                       uint8_t fanSpeed, uint8_t swingVertical, uint8_t swingHorizontal);

    /**
     * @brief Handle Projector command request coming from Modbus writes.
     */
    bool handleProjectorWrite(bool power, uint16_t inputVal);

    /**
     * @brief Syncs IRComboModule command execution states back to the runtime registry.
     */
    void update(uint32_t now_ms);
};

#endif // CORE_COMMAND_MANAGER_H
