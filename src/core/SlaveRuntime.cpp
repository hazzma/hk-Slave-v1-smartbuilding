#include "SlaveRuntime.h"
#include <esp_system.h>
#include "../config/BuildConfig.h"

// esp_mac.h is needed in newer versions of ESP32 Arduino core
#if __has_include(<esp_mac.h>)
#include <esp_mac.h>
#endif

// Instantiate global runtime
SlaveRuntime runtime;

SlaveRuntime::SlaveRuntime() {
    // Zero out identity and capability
    memset(&_identity, 0, sizeof(_identity));
    memset(&_capability, 0, sizeof(_capability));
    memset(&_state, 0, sizeof(_state));

    _identity.fw_version = 211; // representing v2.1.1

    // Set initial boot parameters according to FSD
    _state.active_address = DEFAULT_MODBUS_ADDRESS; // 247
    _state.waiting_at_pairing_address = true;
    _state.last_error = 0; // SLAVE_ERR_NONE

    // Set all initial sensor registers to "not assigned" sentinel values
    for (int i = 0; i < 4; i++) {
        _state.temp_x10[i] = -32767;  // signed sentinel
        _state.lux_lx[i] = 0xFFFE;     // unsigned sentinel
        _state.presence_state[i] = 0xFFFE;
    }
    _state.co2_ppm = 0xFFFE;
    _state.relay_state[0] = 0xFFFE;
    _state.relay_state[1] = 0xFFFE;

    _state.ac_1_command_status = CMD_IDLE;
    _state.ac_2_command_status = CMD_IDLE;
    _state.projector_command_status = CMD_IDLE;
}

void SlaveRuntime::init() {
    // Read the factory MAC address from efuse (station mac)
    esp_read_mac(_identity.mac, ESP_MAC_WIFI_STA);
}

void SlaveRuntime::setActiveAddress(uint8_t addr) {
    if (addr >= 2 && addr <= 246) {
        _state.active_address = addr;
        _state.waiting_at_pairing_address = false;
    }
}

void SlaveRuntime::setWaitingAtPairingAddress(bool waiting) {
    _state.waiting_at_pairing_address = waiting;
}
