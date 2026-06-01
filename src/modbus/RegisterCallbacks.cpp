#include "RegisterCallbacks.h"
#include "../config/RegisterMap.h"

// Define pointer handles to core managers
CapabilityManager* globalCapManager = nullptr;
CommandManager* globalCmdManager = nullptr;

void RegisterCallbacks::setupCallbacks(ModbusRTU& mb) {
    // 1. Identity write callback (Address shifting)
    mb.onSet(HREG(REG_NODE_ADDRESS), onWriteNodeAddress);

    // 2. Capability write callbacks
    mb.onSet(HREG(REG_TEMP_ASSIGNMENT), onWriteCapability);
    mb.onSet(HREG(REG_LUX_ASSIGNMENT), onWriteCapability);
    mb.onSet(HREG(REG_CO2_COUNT), onWriteCapability);
    mb.onSet(HREG(REG_PRESENCE_ASSIGNMENT), onWriteCapability);
    mb.onSet(HREG(REG_RELAY_ASSIGNMENT), onWriteCapability);
    mb.onSet(HREG(REG_IR_PROJECTOR_ENABLE), onWriteCapability);
    mb.onSet(HREG(REG_IR_AC_1_ENABLE), onWriteCapability);
    mb.onSet(HREG(REG_IR_AC_2_ENABLE), onWriteCapability);

    // 3. Error code write callback
    mb.onSet(HREG(REG_LAST_ERROR), onWriteLastError);

    // 4. Recovery address and MAC write callbacks
    mb.onSet(HREG(REG_RECOVERY_MAC_0_1), onWriteRecoveryAddress);
    mb.onSet(HREG(REG_RECOVERY_MAC_2_3), onWriteRecoveryAddress);
    mb.onSet(HREG(REG_RECOVERY_MAC_4_5), onWriteRecoveryAddress);
    mb.onSet(HREG(REG_RECOVERY_NODE_ADDRESS), onWriteRecoveryAddress);

    // 5. AC Control write callbacks
    mb.onSet(HREG(REG_AC_1_POWER), onWriteAcControl);
    mb.onSet(HREG(REG_AC_1_SET_TEMP), onWriteAcControl);
    mb.onSet(HREG(REG_AC_1_MODE), onWriteAcControl);
    mb.onSet(HREG(REG_AC_2_POWER), onWriteAcControl);
    mb.onSet(HREG(REG_AC_2_SET_TEMP), onWriteAcControl);
    mb.onSet(HREG(REG_AC_2_MODE), onWriteAcControl);

    // 6. Projector Control write callbacks
    mb.onSet(HREG(REG_PROJECTOR_POWER), onWriteProjectorControl);
    mb.onSet(HREG(REG_PROJECTOR_INPUT), onWriteProjectorControl);
}

uint16_t RegisterCallbacks::onWriteNodeAddress(TRegister* reg, uint16_t val) {
    if (val >= 2 && val <= 246) {
        runtime.setActiveAddress(val);
        return val;
    } else {
        runtime.setLastError(5); // SLAVE_ERR_BAD_ADDRESS
        return runtime.getActiveAddress(); // Reject the write, return old address
    }
}

uint16_t RegisterCallbacks::onWriteCapability(TRegister* reg, uint16_t val) {
    SlaveCapability& cap = runtime.getCapability();
    switch (reg->address.address) {
        case REG_TEMP_ASSIGNMENT:
            cap.temp_assignment_mask = val;
            break;
        case REG_LUX_ASSIGNMENT:
            cap.lux_assignment_mask = val;
            break;
        case REG_CO2_COUNT:
            cap.co2_count = val;
            break;
        case REG_PRESENCE_ASSIGNMENT:
            cap.presence_assignment_mask = val;
            break;
        case REG_RELAY_ASSIGNMENT:
            cap.relay_assignment_mask = val;
            break;
        case REG_IR_PROJECTOR_ENABLE:
            cap.ir_projector_enable = val;
            break;
        case REG_IR_AC_1_ENABLE:
            cap.ir_ac_1_enable = val;
            break;
        case REG_IR_AC_2_ENABLE:
            cap.ir_ac_2_enable = val;
            break;
    }
    return val;
}

uint16_t RegisterCallbacks::onWriteLastError(TRegister* reg, uint16_t val) {
    runtime.setLastError(val);
    return val;
}

uint16_t RegisterCallbacks::onWriteRecoveryAddress(TRegister* reg, uint16_t val) {
    static uint16_t recoveryMac01 = 0;
    static uint16_t recoveryMac23 = 0;
    static uint16_t recoveryMac45 = 0;

    if (reg->address.address == REG_RECOVERY_NODE_ADDRESS) {
        uint8_t rMac[6];
        rMac[0] = (recoveryMac01 >> 8) & 0xFF;
        rMac[1] = recoveryMac01 & 0xFF;
        rMac[2] = (recoveryMac23 >> 8) & 0xFF;
        rMac[3] = recoveryMac23 & 0xFF;
        rMac[4] = (recoveryMac45 >> 8) & 0xFF;
        rMac[5] = recoveryMac45 & 0xFF;

        const uint8_t* localMac = runtime.getMac();
        bool macMatch = true;
        for (int i = 0; i < 6; i++) {
            if (rMac[i] != localMac[i]) {
                macMatch = false;
                break;
            }
        }

        if (macMatch) {
            if (val >= 2 && val <= 246) {
                runtime.setActiveAddress(val);
                return val;
            } else {
                runtime.setLastError(5); // SLAVE_ERR_BAD_ADDRESS
                return 0;
            }
        } else {
            runtime.setLastError(8); // SLAVE_ERR_RECOVERY_MAC
            return 0; // Reject
        }
    }

    // Capture the recovery MAC values when they are written
    if (reg->address.address == REG_RECOVERY_MAC_0_1) {
        recoveryMac01 = val;
    } else if (reg->address.address == REG_RECOVERY_MAC_2_3) {
        recoveryMac23 = val;
    } else if (reg->address.address == REG_RECOVERY_MAC_4_5) {
        recoveryMac45 = val;
    }

    return val;
}

uint16_t RegisterCallbacks::onWriteAcControl(TRegister* reg, uint16_t val) {
    if (globalCmdManager == nullptr) return val;

    // Determine target channel (1 or 2)
    uint8_t channel = (reg->address.address <= REG_AC_1_MODE) ? 1 : 2;
    
    extern ModbusRTU* globalMbInstance;
    if (globalMbInstance == nullptr) return val;

    uint16_t powerVal = 0;
    uint16_t tempVal = 0;
    uint16_t modeVal = 0;

    if (channel == 1) {
        powerVal = (reg->address.address == REG_AC_1_POWER) ? val : globalMbInstance->Hreg(REG_AC_1_POWER);
        tempVal  = (reg->address.address == REG_AC_1_SET_TEMP) ? val : globalMbInstance->Hreg(REG_AC_1_SET_TEMP);
        modeVal  = (reg->address.address == REG_AC_1_MODE) ? val : globalMbInstance->Hreg(REG_AC_1_MODE);
    } else {
        powerVal = (reg->address.address == REG_AC_2_POWER) ? val : globalMbInstance->Hreg(REG_AC_2_POWER);
        tempVal  = (reg->address.address == REG_AC_2_SET_TEMP) ? val : globalMbInstance->Hreg(REG_AC_2_SET_TEMP);
        modeVal  = (reg->address.address == REG_AC_2_MODE) ? val : globalMbInstance->Hreg(REG_AC_2_MODE);
    }

    globalCmdManager->handleAcWrite(channel, powerVal > 0, tempVal, modeVal);

    return val;
}

uint16_t RegisterCallbacks::onWriteProjectorControl(TRegister* reg, uint16_t val) {
    if (globalCmdManager == nullptr) return val;

    extern ModbusRTU* globalMbInstance;
    if (globalMbInstance == nullptr) return val;

    uint16_t powerVal = (reg->address.address == REG_PROJECTOR_POWER) ? val : globalMbInstance->Hreg(REG_PROJECTOR_POWER);
    uint16_t inputVal = (reg->address.address == REG_PROJECTOR_INPUT) ? val : globalMbInstance->Hreg(REG_PROJECTOR_INPUT);

    globalCmdManager->handleProjectorWrite(powerVal > 0, inputVal);

    return val;
}
