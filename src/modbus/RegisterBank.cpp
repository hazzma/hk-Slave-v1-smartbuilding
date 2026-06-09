#include "RegisterBank.h"
#include "../config/RegisterMap.h"

RegisterBank::RegisterBank(ModbusRTU& mb, DHT22Module* dht, BH1750Module* lux, SCD30Module* co2, 
                           PresenceDigitalModule* presence, RelayModule* relay, IRComboModule* ir)
    : _mb(mb), _dht(dht), _lux(lux), _co2(co2), _presence(presence), _relay(relay), _ir(ir) {}

void RegisterBank::initRegisters() {
    // Coils are a separate Modbus address space. The contract requires FC01
    // support, so mirror relay state as coils at the relay register offsets.
    _mb.addCoil(REG_RELAY_1_STATE, false);
    _mb.addCoil(REG_RELAY_2_STATE, false);

    _mb.addHreg(REG_NODE_ADDRESS, runtime.getActiveAddress());
    _mb.addHreg(REG_FW_VERSION, runtime.getFwVersion());
    
    // MAC bytes big-endian mapping
    const uint8_t* mac = runtime.getMac();
    uint16_t mac01 = (mac[0] << 8) | mac[1];
    uint16_t mac23 = (mac[2] << 8) | mac[3];
    uint16_t mac45 = (mac[4] << 8) | mac[5];
    _mb.addHreg(REG_MAC_0_1, mac01);
    _mb.addHreg(REG_MAC_2_3, mac23);
    _mb.addHreg(REG_MAC_4_5, mac45);

    // Capability assignments (init to 0, master will configure)
    _mb.addHreg(REG_TEMP_ASSIGNMENT, 0);
    _mb.addHreg(REG_LUX_ASSIGNMENT, 0);
    _mb.addHreg(REG_CO2_COUNT, 0);
    _mb.addHreg(REG_PRESENCE_ASSIGNMENT, 0);
    _mb.addHreg(REG_RELAY_ASSIGNMENT, 0);
    _mb.addHreg(REG_IR_PROJECTOR_ENABLE, 0);
    _mb.addHreg(REG_IR_AC_1_ENABLE, 0);
    _mb.addHreg(REG_IR_AC_2_ENABLE, 0);

    // Config & Recovery Registers
    _mb.addHreg(REG_CONFIG_VERSION, 210); // v2.1.0 = 210
    _mb.addHreg(REG_LAST_ERROR, 0);
    _mb.addHreg(REG_UPTIME_LOW, 0);
    _mb.addHreg(REG_UPTIME_HIGH, 0);
    _mb.addHreg(REG_RECOVERY_MAC_0_1, 0);
    _mb.addHreg(REG_RECOVERY_MAC_2_3, 0);
    _mb.addHreg(REG_RECOVERY_MAC_4_5, 0);
    _mb.addHreg(REG_RECOVERY_NODE_ADDRESS, 0);

    // Initial sensor values (init to unassigned sentinels)
    _mb.addHreg(REG_TEMP_1_X10, -32767);
    _mb.addHreg(REG_TEMP_2_X10, -32767);
    _mb.addHreg(REG_TEMP_3_X10, -32767);
    _mb.addHreg(REG_TEMP_4_X10, -32767);
    _mb.addHreg(REG_LUX_1_LX, 0xFFFE);
    _mb.addHreg(REG_LUX_2_LX, 0xFFFE);
    _mb.addHreg(REG_LUX_3_LX, 0xFFFE);
    _mb.addHreg(REG_LUX_4_LX, 0xFFFE);
    _mb.addHreg(REG_CO2_PPM, 0xFFFE);
    _mb.addHreg(REG_PRESENCE_1_STATE, 0xFFFE);
    _mb.addHreg(REG_PRESENCE_2_STATE, 0xFFFE);
    _mb.addHreg(REG_PRESENCE_3_STATE, 0xFFFE);
    _mb.addHreg(REG_PRESENCE_4_STATE, 0xFFFE);
    _mb.addHreg(REG_RELAY_1_STATE, 0xFFFE);
    _mb.addHreg(REG_RELAY_2_STATE, 0xFFFE);

    // AC & Projector Control registers
    _mb.addHreg(REG_AC_1_POWER, 0);
    _mb.addHreg(REG_AC_1_SET_TEMP, 240); // 24.0 C default
    _mb.addHreg(REG_AC_1_MODE, 0);       // cool
    _mb.addHreg(REG_AC_2_POWER, 0);
    _mb.addHreg(REG_AC_2_SET_TEMP, 240);
    _mb.addHreg(REG_AC_2_MODE, 0);
    _mb.addHreg(REG_AC_1_COMMAND_STATUS, CMD_IDLE);
    _mb.addHreg(REG_AC_2_COMMAND_STATUS, CMD_IDLE);
    _mb.addHreg(REG_AC_1_FAN_SPEED, 99);
    _mb.addHreg(REG_AC_1_SWING_VERTICAL, 99);
    _mb.addHreg(REG_AC_1_SWING_HORIZONTAL, 99);
    _mb.addHreg(REG_AC_2_FAN_SPEED, 99);
    _mb.addHreg(REG_AC_2_SWING_VERTICAL, 99);
    _mb.addHreg(REG_AC_2_SWING_HORIZONTAL, 99);
    
    _mb.addHreg(REG_PROJECTOR_POWER, 0);
    _mb.addHreg(REG_PROJECTOR_INPUT, 0);
    _mb.addHreg(REG_PROJECTOR_COMMAND_STATUS, CMD_IDLE);

    // Input-register mirror for FC04. Offsets intentionally match the holding
    // register map so a master can read cached state via either FC03 or FC04.
    _mb.addIreg(REG_NODE_ADDRESS, runtime.getActiveAddress());
    _mb.addIreg(REG_FW_VERSION, runtime.getFwVersion());
    _mb.addIreg(REG_MAC_0_1, mac01);
    _mb.addIreg(REG_MAC_2_3, mac23);
    _mb.addIreg(REG_MAC_4_5, mac45);

    _mb.addIreg(REG_TEMP_ASSIGNMENT, 0);
    _mb.addIreg(REG_LUX_ASSIGNMENT, 0);
    _mb.addIreg(REG_CO2_COUNT, 0);
    _mb.addIreg(REG_PRESENCE_ASSIGNMENT, 0);
    _mb.addIreg(REG_RELAY_ASSIGNMENT, 0);
    _mb.addIreg(REG_IR_PROJECTOR_ENABLE, 0);
    _mb.addIreg(REG_IR_AC_1_ENABLE, 0);
    _mb.addIreg(REG_IR_AC_2_ENABLE, 0);

    _mb.addIreg(REG_CONFIG_VERSION, 210);
    _mb.addIreg(REG_LAST_ERROR, 0);
    _mb.addIreg(REG_UPTIME_LOW, 0);
    _mb.addIreg(REG_UPTIME_HIGH, 0);

    _mb.addIreg(REG_TEMP_1_X10, static_cast<uint16_t>(-32767));
    _mb.addIreg(REG_TEMP_2_X10, static_cast<uint16_t>(-32767));
    _mb.addIreg(REG_TEMP_3_X10, static_cast<uint16_t>(-32767));
    _mb.addIreg(REG_TEMP_4_X10, static_cast<uint16_t>(-32767));
    _mb.addIreg(REG_LUX_1_LX, 0xFFFE);
    _mb.addIreg(REG_LUX_2_LX, 0xFFFE);
    _mb.addIreg(REG_LUX_3_LX, 0xFFFE);
    _mb.addIreg(REG_LUX_4_LX, 0xFFFE);
    _mb.addIreg(REG_CO2_PPM, 0xFFFE);
    _mb.addIreg(REG_PRESENCE_1_STATE, 0xFFFE);
    _mb.addIreg(REG_PRESENCE_2_STATE, 0xFFFE);
    _mb.addIreg(REG_PRESENCE_3_STATE, 0xFFFE);
    _mb.addIreg(REG_PRESENCE_4_STATE, 0xFFFE);
    _mb.addIreg(REG_RELAY_1_STATE, 0xFFFE);
    _mb.addIreg(REG_RELAY_2_STATE, 0xFFFE);

    _mb.addIreg(REG_AC_1_POWER, 0);
    _mb.addIreg(REG_AC_1_SET_TEMP, 240);
    _mb.addIreg(REG_AC_1_MODE, 0);
    _mb.addIreg(REG_AC_2_POWER, 0);
    _mb.addIreg(REG_AC_2_SET_TEMP, 240);
    _mb.addIreg(REG_AC_2_MODE, 0);
    _mb.addIreg(REG_AC_1_COMMAND_STATUS, CMD_IDLE);
    _mb.addIreg(REG_AC_2_COMMAND_STATUS, CMD_IDLE);
    _mb.addIreg(REG_AC_1_FAN_SPEED, 99);
    _mb.addIreg(REG_AC_1_SWING_VERTICAL, 99);
    _mb.addIreg(REG_AC_1_SWING_HORIZONTAL, 99);
    _mb.addIreg(REG_AC_2_FAN_SPEED, 99);
    _mb.addIreg(REG_AC_2_SWING_VERTICAL, 99);
    _mb.addIreg(REG_AC_2_SWING_HORIZONTAL, 99);
    _mb.addIreg(REG_PROJECTOR_COMMAND_STATUS, CMD_IDLE);
}

void RegisterBank::update(uint32_t now_ms) {
    SlaveRuntimeState& state = runtime.getState();

    // 1. Sync System registers
    _mb.Hreg(REG_NODE_ADDRESS, runtime.getActiveAddress());
    _mb.Ireg(REG_NODE_ADDRESS, runtime.getActiveAddress());
    _mb.Ireg(REG_FW_VERSION, runtime.getFwVersion());
    _mb.Hreg(REG_LAST_ERROR, runtime.getLastError());
    _mb.Ireg(REG_LAST_ERROR, runtime.getLastError());

    SlaveCapability& cap = runtime.getCapability();
    _mb.Ireg(REG_TEMP_ASSIGNMENT, cap.temp_assignment_mask);
    _mb.Ireg(REG_LUX_ASSIGNMENT, cap.lux_assignment_mask);
    _mb.Ireg(REG_CO2_COUNT, cap.co2_count);
    _mb.Ireg(REG_PRESENCE_ASSIGNMENT, cap.presence_assignment_mask);
    _mb.Ireg(REG_RELAY_ASSIGNMENT, cap.relay_assignment_mask);
    _mb.Ireg(REG_IR_PROJECTOR_ENABLE, cap.ir_projector_enable);
    _mb.Ireg(REG_IR_AC_1_ENABLE, cap.ir_ac_1_enable);
    _mb.Ireg(REG_IR_AC_2_ENABLE, cap.ir_ac_2_enable);

    // 2. Sync Uptime
    uint32_t uptime = now_ms / 1000;
    _mb.Hreg(REG_UPTIME_LOW, uptime & 0xFFFF);
    _mb.Hreg(REG_UPTIME_HIGH, (uptime >> 16) & 0xFFFF);
    _mb.Ireg(REG_UPTIME_LOW, uptime & 0xFFFF);
    _mb.Ireg(REG_UPTIME_HIGH, (uptime >> 16) & 0xFFFF);

    // 3. Sync DHT22 Temperature Sensors
    if (_dht != nullptr) {
        state.temp_x10[0] = _dht->getTemperatureX10(0);
        state.temp_x10[1] = _dht->getTemperatureX10(1);
        state.temp_x10[2] = _dht->getTemperatureX10(2);
        state.temp_x10[3] = _dht->getTemperatureX10(3);
        _mb.Hreg(REG_TEMP_1_X10, _dht->getTemperatureX10(0));
        _mb.Hreg(REG_TEMP_2_X10, _dht->getTemperatureX10(1));
        _mb.Hreg(REG_TEMP_3_X10, _dht->getTemperatureX10(2));
        _mb.Hreg(REG_TEMP_4_X10, _dht->getTemperatureX10(3));
        _mb.Ireg(REG_TEMP_1_X10, _dht->getTemperatureX10(0));
        _mb.Ireg(REG_TEMP_2_X10, _dht->getTemperatureX10(1));
        _mb.Ireg(REG_TEMP_3_X10, _dht->getTemperatureX10(2));
        _mb.Ireg(REG_TEMP_4_X10, _dht->getTemperatureX10(3));
    }

    // 4. Sync BH1750 Lux Sensors
    if (_lux != nullptr) {
        state.lux_lx[0] = _lux->getLux(0);
        state.lux_lx[1] = _lux->getLux(1);
        state.lux_lx[2] = _lux->getLux(2);
        state.lux_lx[3] = _lux->getLux(3);
        _mb.Hreg(REG_LUX_1_LX, _lux->getLux(0));
        _mb.Hreg(REG_LUX_2_LX, _lux->getLux(1));
        _mb.Hreg(REG_LUX_3_LX, _lux->getLux(2));
        _mb.Hreg(REG_LUX_4_LX, _lux->getLux(3));
        _mb.Ireg(REG_LUX_1_LX, _lux->getLux(0));
        _mb.Ireg(REG_LUX_2_LX, _lux->getLux(1));
        _mb.Ireg(REG_LUX_3_LX, _lux->getLux(2));
        _mb.Ireg(REG_LUX_4_LX, _lux->getLux(3));
    }

    // 5. Sync SCD30 CO2 Sensor
    if (_co2 != nullptr) {
        state.co2_ppm = _co2->getCO2();
        _mb.Hreg(REG_CO2_PPM, _co2->getCO2());
        _mb.Ireg(REG_CO2_PPM, _co2->getCO2());
    }

    // 6. Sync Presence Sensors
    if (_presence != nullptr) {
        state.presence_state[0] = _presence->getPresenceState(0);
        state.presence_state[1] = _presence->getPresenceState(1);
        state.presence_state[2] = _presence->getPresenceState(2);
        state.presence_state[3] = _presence->getPresenceState(3);
        _mb.Hreg(REG_PRESENCE_1_STATE, _presence->getPresenceState(0));
        _mb.Hreg(REG_PRESENCE_2_STATE, _presence->getPresenceState(1));
        _mb.Hreg(REG_PRESENCE_3_STATE, _presence->getPresenceState(2));
        _mb.Hreg(REG_PRESENCE_4_STATE, _presence->getPresenceState(3));
        _mb.Ireg(REG_PRESENCE_1_STATE, _presence->getPresenceState(0));
        _mb.Ireg(REG_PRESENCE_2_STATE, _presence->getPresenceState(1));
        _mb.Ireg(REG_PRESENCE_3_STATE, _presence->getPresenceState(2));
        _mb.Ireg(REG_PRESENCE_4_STATE, _presence->getPresenceState(3));
    }

    if (_relay != nullptr) {
        state.relay_state[0] = _relay->getRelayState(0);
        state.relay_state[1] = _relay->getRelayState(1);
    }

    _mb.Hreg(REG_RELAY_1_STATE, state.relay_state[0]);
    _mb.Hreg(REG_RELAY_2_STATE, state.relay_state[1]);
    _mb.Ireg(REG_RELAY_1_STATE, state.relay_state[0]);
    _mb.Ireg(REG_RELAY_2_STATE, state.relay_state[1]);
    _mb.Coil(REG_RELAY_1_STATE, state.relay_state[0] == 1);
    _mb.Coil(REG_RELAY_2_STATE, state.relay_state[1] == 1);

    // 7. Sync Command Statuses and FC04 mirrors for control registers
    _mb.Ireg(REG_AC_1_POWER, _mb.Hreg(REG_AC_1_POWER));
    _mb.Ireg(REG_AC_1_SET_TEMP, _mb.Hreg(REG_AC_1_SET_TEMP));
    _mb.Ireg(REG_AC_1_MODE, _mb.Hreg(REG_AC_1_MODE));
    _mb.Ireg(REG_AC_2_POWER, _mb.Hreg(REG_AC_2_POWER));
    _mb.Ireg(REG_AC_2_SET_TEMP, _mb.Hreg(REG_AC_2_SET_TEMP));
    _mb.Ireg(REG_AC_2_MODE, _mb.Hreg(REG_AC_2_MODE));
    _mb.Ireg(REG_AC_1_FAN_SPEED, _mb.Hreg(REG_AC_1_FAN_SPEED));
    _mb.Ireg(REG_AC_1_SWING_VERTICAL, _mb.Hreg(REG_AC_1_SWING_VERTICAL));
    _mb.Ireg(REG_AC_1_SWING_HORIZONTAL, _mb.Hreg(REG_AC_1_SWING_HORIZONTAL));
    _mb.Ireg(REG_AC_2_FAN_SPEED, _mb.Hreg(REG_AC_2_FAN_SPEED));
    _mb.Ireg(REG_AC_2_SWING_VERTICAL, _mb.Hreg(REG_AC_2_SWING_VERTICAL));
    _mb.Ireg(REG_AC_2_SWING_HORIZONTAL, _mb.Hreg(REG_AC_2_SWING_HORIZONTAL));
    _mb.Hreg(REG_AC_1_COMMAND_STATUS, state.ac_1_command_status);
    _mb.Hreg(REG_AC_2_COMMAND_STATUS, state.ac_2_command_status);
    _mb.Hreg(REG_PROJECTOR_COMMAND_STATUS, state.projector_command_status);
    _mb.Ireg(REG_AC_1_COMMAND_STATUS, state.ac_1_command_status);
    _mb.Ireg(REG_AC_2_COMMAND_STATUS, state.ac_2_command_status);
    _mb.Ireg(REG_PROJECTOR_COMMAND_STATUS, state.projector_command_status);
}
