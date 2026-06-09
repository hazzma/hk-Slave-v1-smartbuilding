#include "RegisterCallbacks.h"
#include "../config/RegisterMap.h"
#include "../core/ErrorManager.h"

// Define pointer handles to core managers
CapabilityManager* globalCapManager = nullptr;
CommandManager* globalCmdManager = nullptr;
RelayModule* globalRelayModule = nullptr;

void RegisterCallbacks::setupCallbacks(ModbusRTU& mb) {
    // 1. Identity write callback (Address shifting)
    mb.onSet(HREG(REG_NODE_ADDRESS), onWriteNodeAddress);
    mb.onSet(HREG(REG_FW_VERSION), onWriteReadOnly);
    mb.onSet(HREG(REG_MAC_0_1), onWriteReadOnly);
    mb.onSet(HREG(REG_MAC_2_3), onWriteReadOnly);
    mb.onSet(HREG(REG_MAC_4_5), onWriteReadOnly);

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
    mb.onSet(HREG(REG_CONFIG_VERSION), onWriteReadOnly);
    mb.onSet(HREG(REG_UPTIME_LOW), onWriteReadOnly);
    mb.onSet(HREG(REG_UPTIME_HIGH), onWriteReadOnly);

    // 4. Recovery address and MAC write callbacks
    mb.onSet(HREG(REG_RECOVERY_MAC_0_1), onWriteRecoveryAddress);
    mb.onSet(HREG(REG_RECOVERY_MAC_2_3), onWriteRecoveryAddress);
    mb.onSet(HREG(REG_RECOVERY_MAC_4_5), onWriteRecoveryAddress);
    mb.onSet(HREG(REG_RECOVERY_NODE_ADDRESS), onWriteRecoveryAddress);

    // 5. AC Control write callbacks
    mb.onSet(HREG(REG_AC_1_POWER), onWriteAcControl);
    mb.onSet(HREG(REG_AC_1_SET_TEMP), onWriteAcControl);
    mb.onSet(HREG(REG_AC_1_MODE), onWriteAcControl);
    mb.onSet(HREG(REG_AC_1_FAN_SPEED), onWriteAcControl);
    mb.onSet(HREG(REG_AC_1_SWING_VERTICAL), onWriteAcControl);
    mb.onSet(HREG(REG_AC_1_SWING_HORIZONTAL), onWriteAcControl);
    mb.onSet(HREG(REG_AC_2_POWER), onWriteAcControl);
    mb.onSet(HREG(REG_AC_2_SET_TEMP), onWriteAcControl);
    mb.onSet(HREG(REG_AC_2_MODE), onWriteAcControl);
    mb.onSet(HREG(REG_AC_2_FAN_SPEED), onWriteAcControl);
    mb.onSet(HREG(REG_AC_2_SWING_VERTICAL), onWriteAcControl);
    mb.onSet(HREG(REG_AC_2_SWING_HORIZONTAL), onWriteAcControl);

    // 6. Projector Control write callbacks
    mb.onSet(HREG(REG_PROJECTOR_POWER), onWriteProjectorControl);
    mb.onSet(HREG(REG_PROJECTOR_INPUT), onWriteProjectorControl);

    // 7. Sensor/status register write protection and relay state writes
    mb.onSet(HREG(REG_TEMP_1_X10), onWriteReadOnly);
    mb.onSet(HREG(REG_TEMP_2_X10), onWriteReadOnly);
    mb.onSet(HREG(REG_TEMP_3_X10), onWriteReadOnly);
    mb.onSet(HREG(REG_TEMP_4_X10), onWriteReadOnly);
    mb.onSet(HREG(REG_LUX_1_LX), onWriteReadOnly);
    mb.onSet(HREG(REG_LUX_2_LX), onWriteReadOnly);
    mb.onSet(HREG(REG_LUX_3_LX), onWriteReadOnly);
    mb.onSet(HREG(REG_LUX_4_LX), onWriteReadOnly);
    mb.onSet(HREG(REG_CO2_PPM), onWriteReadOnly);
    mb.onSet(HREG(REG_PRESENCE_1_STATE), onWriteReadOnly);
    mb.onSet(HREG(REG_PRESENCE_2_STATE), onWriteReadOnly);
    mb.onSet(HREG(REG_PRESENCE_3_STATE), onWriteReadOnly);
    mb.onSet(HREG(REG_PRESENCE_4_STATE), onWriteReadOnly);
    mb.onSet(HREG(REG_RELAY_1_STATE), onWriteRelayState);
    mb.onSet(HREG(REG_RELAY_2_STATE), onWriteRelayState);
    mb.onSet(HREG(REG_AC_1_COMMAND_STATUS), onWriteReadOnly);
    mb.onSet(HREG(REG_AC_2_COMMAND_STATUS), onWriteReadOnly);
    mb.onSet(HREG(REG_PROJECTOR_COMMAND_STATUS), onWriteReadOnly);
}

static uint16_t currentMirrorValue(uint16_t address) {
    SlaveIdentity& identity = runtime.getIdentity();
    SlaveCapability& cap = runtime.getCapability();
    SlaveRuntimeState& state = runtime.getState();
    uint32_t uptime = millis() / 1000;

    switch (address) {
        case REG_NODE_ADDRESS:
            return runtime.getActiveAddress();
        case REG_FW_VERSION:
            return runtime.getFwVersion();
        case REG_MAC_0_1:
            return (identity.mac[0] << 8) | identity.mac[1];
        case REG_MAC_2_3:
            return (identity.mac[2] << 8) | identity.mac[3];
        case REG_MAC_4_5:
            return (identity.mac[4] << 8) | identity.mac[5];
        case REG_TEMP_ASSIGNMENT:
            return cap.temp_assignment_mask;
        case REG_LUX_ASSIGNMENT:
            return cap.lux_assignment_mask;
        case REG_CO2_COUNT:
            return cap.co2_count;
        case REG_PRESENCE_ASSIGNMENT:
            return cap.presence_assignment_mask;
        case REG_RELAY_ASSIGNMENT:
            return cap.relay_assignment_mask;
        case REG_IR_PROJECTOR_ENABLE:
            return cap.ir_projector_enable;
        case REG_IR_AC_1_ENABLE:
            return cap.ir_ac_1_enable;
        case REG_IR_AC_2_ENABLE:
            return cap.ir_ac_2_enable;
        case REG_CONFIG_VERSION:
            return 210;
        case REG_LAST_ERROR:
            return runtime.getLastError();
        case REG_UPTIME_LOW:
            return uptime & 0xFFFF;
        case REG_UPTIME_HIGH:
            return (uptime >> 16) & 0xFFFF;
        case REG_TEMP_1_X10:
            return static_cast<uint16_t>(state.temp_x10[0]);
        case REG_TEMP_2_X10:
            return static_cast<uint16_t>(state.temp_x10[1]);
        case REG_TEMP_3_X10:
            return static_cast<uint16_t>(state.temp_x10[2]);
        case REG_TEMP_4_X10:
            return static_cast<uint16_t>(state.temp_x10[3]);
        case REG_LUX_1_LX:
            return state.lux_lx[0];
        case REG_LUX_2_LX:
            return state.lux_lx[1];
        case REG_LUX_3_LX:
            return state.lux_lx[2];
        case REG_LUX_4_LX:
            return state.lux_lx[3];
        case REG_CO2_PPM:
            return state.co2_ppm;
        case REG_PRESENCE_1_STATE:
            return state.presence_state[0];
        case REG_PRESENCE_2_STATE:
            return state.presence_state[1];
        case REG_PRESENCE_3_STATE:
            return state.presence_state[2];
        case REG_PRESENCE_4_STATE:
            return state.presence_state[3];
        case REG_RELAY_1_STATE:
            return state.relay_state[0];
        case REG_RELAY_2_STATE:
            return state.relay_state[1];
        case REG_AC_1_COMMAND_STATUS:
            return state.ac_1_command_status;
        case REG_AC_2_COMMAND_STATUS:
            return state.ac_2_command_status;
        case REG_PROJECTOR_COMMAND_STATUS:
            return state.projector_command_status;
        default:
            return 0;
    }
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
            if (val > 0x0F) {
                runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
                return cap.temp_assignment_mask;
            }
            cap.temp_assignment_mask = val;
            break;
        case REG_LUX_ASSIGNMENT:
            if (val > 0x0F) {
                runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
                return cap.lux_assignment_mask;
            }
            cap.lux_assignment_mask = val;
            break;
        case REG_CO2_COUNT:
            if (val > 1) {
                runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
                return cap.co2_count;
            }
            cap.co2_count = val;
            break;
        case REG_PRESENCE_ASSIGNMENT:
            if (val > 0x0F) {
                runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
                return cap.presence_assignment_mask;
            }
            cap.presence_assignment_mask = val;
            break;
        case REG_RELAY_ASSIGNMENT:
            if (val > 0x03) {
                runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
                return cap.relay_assignment_mask;
            }
            cap.relay_assignment_mask = val;
            if (globalRelayModule != nullptr) {
                globalRelayModule->setChannelAssignment(0, (val & 2) != 0);
                globalRelayModule->setChannelAssignment(1, (val & 1) != 0);
            }
            runtime.getState().relay_state[0] = (val & 2) ? 0 : 0xFFFE;
            runtime.getState().relay_state[1] = (val & 1) ? 0 : 0xFFFE;
            break;
        case REG_IR_PROJECTOR_ENABLE:
            if (val > 1) {
                runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
                return cap.ir_projector_enable;
            }
            cap.ir_projector_enable = val;
            break;
        case REG_IR_AC_1_ENABLE:
            if (val > 1) {
                runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
                return cap.ir_ac_1_enable;
            }
            cap.ir_ac_1_enable = val;
            break;
        case REG_IR_AC_2_ENABLE:
            if (val > 1) {
                runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
                return cap.ir_ac_2_enable;
            }
            cap.ir_ac_2_enable = val;
            break;
    }
    return val;
}

uint16_t RegisterCallbacks::onWriteLastError(TRegister* reg, uint16_t val) {
    if (val == SLAVE_ERR_NONE) {
        runtime.setLastError(SLAVE_ERR_NONE);
        return SLAVE_ERR_NONE;
    }

    runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
    return SLAVE_ERR_UNSUPPORTED_WRITE;
}

uint16_t RegisterCallbacks::onWriteRecoveryAddress(TRegister* reg, uint16_t val) {
    static uint16_t recoveryMac01 = 0;
    static uint16_t recoveryMac23 = 0;
    static uint16_t recoveryMac45 = 0;

    // Read back from HREG when possible so FC16 write-multiple recovery uses
    // the values that were written in the same transaction.
    extern ModbusRTU* globalMbInstance;
    if (globalMbInstance != nullptr) {
        recoveryMac01 = globalMbInstance->Hreg(REG_RECOVERY_MAC_0_1);
        recoveryMac23 = globalMbInstance->Hreg(REG_RECOVERY_MAC_2_3);
        recoveryMac45 = globalMbInstance->Hreg(REG_RECOVERY_MAC_4_5);
    }

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
            // Contract says non-matching slaves should ignore recovery writes.
            return 0;
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

uint16_t RegisterCallbacks::onWriteReadOnly(TRegister* reg, uint16_t val) {
    runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
    return currentMirrorValue(reg->address.address);
}

uint16_t RegisterCallbacks::onWriteRelayState(TRegister* reg, uint16_t val) {
    SlaveCapability& cap = runtime.getCapability();
    SlaveRuntimeState& state = runtime.getState();

    if (val > 1) {
        runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
        return currentMirrorValue(reg->address.address);
    }

    if (reg->address.address == REG_RELAY_1_STATE) {
        if ((cap.relay_assignment_mask & 2) == 0 || globalRelayModule == nullptr) {
            runtime.setLastError(SLAVE_ERR_CONFIG_RUNTIME);
            state.relay_state[0] = 0xFFFE;
            return state.relay_state[0];
        }
        if (!globalRelayModule->setRelayState(0, val > 0)) {
            runtime.setLastError(SLAVE_ERR_CONFIG_RUNTIME);
            return state.relay_state[0];
        }
        state.relay_state[0] = val;
        return val;
    }

    if (reg->address.address == REG_RELAY_2_STATE) {
        if ((cap.relay_assignment_mask & 1) == 0 || globalRelayModule == nullptr) {
            runtime.setLastError(SLAVE_ERR_CONFIG_RUNTIME);
            state.relay_state[1] = 0xFFFE;
            return state.relay_state[1];
        }
        if (!globalRelayModule->setRelayState(1, val > 0)) {
            runtime.setLastError(SLAVE_ERR_CONFIG_RUNTIME);
            return state.relay_state[1];
        }
        state.relay_state[1] = val;
        return val;
    }

    runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
    return 0;
}

static bool isAc1ControlRegister(uint16_t address) {
    return address == REG_AC_1_POWER ||
           address == REG_AC_1_SET_TEMP ||
           address == REG_AC_1_MODE ||
           address == REG_AC_1_FAN_SPEED ||
           address == REG_AC_1_SWING_VERTICAL ||
           address == REG_AC_1_SWING_HORIZONTAL;
}

static bool isAc2ControlRegister(uint16_t address) {
    return address == REG_AC_2_POWER ||
           address == REG_AC_2_SET_TEMP ||
           address == REG_AC_2_MODE ||
           address == REG_AC_2_FAN_SPEED ||
           address == REG_AC_2_SWING_VERTICAL ||
           address == REG_AC_2_SWING_HORIZONTAL;
}

static bool isSupportedAcFan(uint16_t value) {
    return value <= 5 || value == 99;
}

static bool isSupportedAcSwing(uint16_t value) {
    return value <= 10 || value == 99;
}

uint16_t RegisterCallbacks::onWriteAcControl(TRegister* reg, uint16_t val) {
    if (globalCmdManager == nullptr) return val;

    // Determine target channel (1 or 2)
    uint16_t address = reg->address.address;
    uint8_t channel = isAc1ControlRegister(address) ? 1 : 2;

    if (!isAc1ControlRegister(address) && !isAc2ControlRegister(address)) {
        runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
        return reg->value;
    }

    if ((address == REG_AC_1_POWER || address == REG_AC_2_POWER) && val > 1) {
        runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
        return reg->value;
    }

    if ((address == REG_AC_1_SET_TEMP || address == REG_AC_2_SET_TEMP) &&
        (val < 160 || val > 300 || val % 10 != 0)) {
        runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
        return reg->value;
    }

    if ((address == REG_AC_1_MODE || address == REG_AC_2_MODE) && val > 4) {
        runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
        return reg->value;
    }

    if ((address == REG_AC_1_FAN_SPEED || address == REG_AC_2_FAN_SPEED) && !isSupportedAcFan(val)) {
        runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
        return reg->value;
    }

    if ((address == REG_AC_1_SWING_VERTICAL || address == REG_AC_1_SWING_HORIZONTAL ||
         address == REG_AC_2_SWING_VERTICAL || address == REG_AC_2_SWING_HORIZONTAL) &&
        !isSupportedAcSwing(val)) {
        runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
        return reg->value;
    }
    
    extern ModbusRTU* globalMbInstance;
    if (globalMbInstance == nullptr) return val;

    uint16_t powerVal = 0;
    uint16_t tempVal = 0;
    uint16_t modeVal = 0;
    uint16_t fanVal = 99;
    uint16_t swingVVal = 99;
    uint16_t swingHVal = 99;

    if (channel == 1) {
        powerVal = (address == REG_AC_1_POWER) ? val : globalMbInstance->Hreg(REG_AC_1_POWER);
        tempVal  = (address == REG_AC_1_SET_TEMP) ? val : globalMbInstance->Hreg(REG_AC_1_SET_TEMP);
        modeVal  = (address == REG_AC_1_MODE) ? val : globalMbInstance->Hreg(REG_AC_1_MODE);
        fanVal = (address == REG_AC_1_FAN_SPEED) ? val : globalMbInstance->Hreg(REG_AC_1_FAN_SPEED);
        swingVVal = (address == REG_AC_1_SWING_VERTICAL) ? val : globalMbInstance->Hreg(REG_AC_1_SWING_VERTICAL);
        swingHVal = (address == REG_AC_1_SWING_HORIZONTAL) ? val : globalMbInstance->Hreg(REG_AC_1_SWING_HORIZONTAL);
    } else {
        powerVal = (address == REG_AC_2_POWER) ? val : globalMbInstance->Hreg(REG_AC_2_POWER);
        tempVal  = (address == REG_AC_2_SET_TEMP) ? val : globalMbInstance->Hreg(REG_AC_2_SET_TEMP);
        modeVal  = (address == REG_AC_2_MODE) ? val : globalMbInstance->Hreg(REG_AC_2_MODE);
        fanVal = (address == REG_AC_2_FAN_SPEED) ? val : globalMbInstance->Hreg(REG_AC_2_FAN_SPEED);
        swingVVal = (address == REG_AC_2_SWING_VERTICAL) ? val : globalMbInstance->Hreg(REG_AC_2_SWING_VERTICAL);
        swingHVal = (address == REG_AC_2_SWING_HORIZONTAL) ? val : globalMbInstance->Hreg(REG_AC_2_SWING_HORIZONTAL);
    }

    bool queued = globalCmdManager->handleAcWrite(channel, powerVal > 0, tempVal,
                                                  static_cast<uint8_t>(modeVal),
                                                  static_cast<uint8_t>(fanVal),
                                                  static_cast<uint8_t>(swingVVal),
                                                  static_cast<uint8_t>(swingHVal));
    if (!queued && channel == 1 && runtime.getState().ac_1_command_status != CMD_BUSY) {
        runtime.getState().ac_1_command_status = CMD_FAILED;
    } else if (!queued && channel == 2 && runtime.getState().ac_2_command_status != CMD_BUSY) {
        runtime.getState().ac_2_command_status = CMD_FAILED;
    }

    return val;
}

uint16_t RegisterCallbacks::onWriteProjectorControl(TRegister* reg, uint16_t val) {
    if (globalCmdManager == nullptr) return val;

    if (reg->address.address == REG_PROJECTOR_INPUT) {
        runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
        return reg->value;
    }

    if (val > 1) {
        runtime.setLastError(SLAVE_ERR_UNSUPPORTED_WRITE);
        return reg->value;
    }

    bool queued = globalCmdManager->handleProjectorWrite(val > 0, 0);
    if (!queued && runtime.getState().projector_command_status != CMD_BUSY) {
        runtime.getState().projector_command_status = CMD_FAILED;
    }

    return val;
}
