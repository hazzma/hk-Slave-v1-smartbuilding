#ifndef MODBUS_REGISTER_CALLBACKS_H
#define MODBUS_REGISTER_CALLBACKS_H

#include <Arduino.h>
#include <ModbusRTU.h>
#include "../core/SlaveRuntime.h"
#include "../core/CapabilityManager.h"
#include "../core/CommandManager.h"

// Define pointer handles to core managers for use inside static callback scopes
extern CapabilityManager* globalCapManager;
extern CommandManager* globalCmdManager;

class RegisterCallbacks {
public:
    /**
     * @brief Attach write callbacks to the ModbusRTU server instance.
     */
    static void setupCallbacks(ModbusRTU& mb);
    
    // Callback functions for master writes
    static uint16_t onWriteNodeAddress(TRegister* reg, uint16_t val);
    static uint16_t onWriteCapability(TRegister* reg, uint16_t val);
    static uint16_t onWriteLastError(TRegister* reg, uint16_t val);
    static uint16_t onWriteRecoveryAddress(TRegister* reg, uint16_t val);
    static uint16_t onWriteAcControl(TRegister* reg, uint16_t val);
    static uint16_t onWriteProjectorControl(TRegister* reg, uint16_t val);
};

#endif // MODBUS_REGISTER_CALLBACKS_H
