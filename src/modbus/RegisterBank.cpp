#include "RegisterBank.h"
#include "../config/RegisterMap.h"

RegisterBank::RegisterBank(ModbusRTU& mb, DHT22Module* dht, BH1750Module* lux, SCD30Module* co2, 
                           PresenceDigitalModule* presence, IRComboModule* ir)
    : _mb(mb), _dht(dht), _lux(lux), _co2(co2), _presence(presence), _ir(ir) {}

void RegisterBank::initRegisters() {
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
    _mb.addHreg(REG_RELAY_1_STATE, 0xFFFE); // Relays not actively managed in FSD V1, keep unassigned
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
    
    _mb.addHreg(REG_PROJECTOR_POWER, 0);
    _mb.addHreg(REG_PROJECTOR_INPUT, 0);
    _mb.addHreg(REG_PROJECTOR_COMMAND_STATUS, CMD_IDLE);
}

void RegisterBank::update(uint32_t now_ms) {
    // 1. Sync System registers
    _mb.Hreg(REG_NODE_ADDRESS, runtime.getActiveAddress());
    _mb.Hreg(REG_LAST_ERROR, runtime.getLastError());

    // 2. Sync Uptime
    uint32_t uptime = now_ms / 1000;
    _mb.Hreg(REG_UPTIME_LOW, uptime & 0xFFFF);
    _mb.Hreg(REG_UPTIME_HIGH, (uptime >> 16) & 0xFFFF);

    // 3. Sync DHT22 Temperature Sensors
    if (_dht != nullptr) {
        _mb.Hreg(REG_TEMP_1_X10, _dht->getTemperatureX10(0));
        _mb.Hreg(REG_TEMP_2_X10, _dht->getTemperatureX10(1));
        _mb.Hreg(REG_TEMP_3_X10, _dht->getTemperatureX10(2));
        _mb.Hreg(REG_TEMP_4_X10, _dht->getTemperatureX10(3));
    }

    // 4. Sync BH1750 Lux Sensors
    if (_lux != nullptr) {
        _mb.Hreg(REG_LUX_1_LX, _lux->getLux(0));
        _mb.Hreg(REG_LUX_2_LX, _lux->getLux(1));
        _mb.Hreg(REG_LUX_3_LX, _lux->getLux(2));
        _mb.Hreg(REG_LUX_4_LX, _lux->getLux(3));
    }

    // 5. Sync SCD30 CO2 Sensor
    if (_co2 != nullptr) {
        _mb.Hreg(REG_CO2_PPM, _co2->getCO2());
    }

    // 6. Sync Presence Sensors
    if (_presence != nullptr) {
        _mb.Hreg(REG_PRESENCE_1_STATE, _presence->getPresenceState(0));
        _mb.Hreg(REG_PRESENCE_2_STATE, _presence->getPresenceState(1));
        _mb.Hreg(REG_PRESENCE_3_STATE, _presence->getPresenceState(2));
        _mb.Hreg(REG_PRESENCE_4_STATE, _presence->getPresenceState(3));
    }

    // 7. Sync Command Statuses
    _mb.Hreg(REG_AC_1_COMMAND_STATUS, runtime.getState().ac_1_command_status);
    _mb.Hreg(REG_AC_2_COMMAND_STATUS, runtime.getState().ac_2_command_status);
    _mb.Hreg(REG_PROJECTOR_COMMAND_STATUS, runtime.getState().projector_command_status);
}
