#ifndef CORE_SLAVE_RUNTIME_H
#define CORE_SLAVE_RUNTIME_H

#include <Arduino.h>
#include "ModuleStatus.h"

struct SlaveIdentity {
  uint8_t mac[6];
  uint16_t fw_version;
};

struct SlaveCapability {
  uint8_t temp_assignment_mask;
  uint8_t lux_assignment_mask;
  uint8_t co2_count;
  uint8_t presence_assignment_mask;
  uint8_t relay_assignment_mask;
  uint8_t ir_projector_enable;
  uint8_t ir_ac_1_enable;
  uint8_t ir_ac_2_enable;
};

struct SlaveRuntimeState {
  uint8_t active_address;
  bool waiting_at_pairing_address;
  uint16_t last_error;
  int16_t temp_x10[4];
  uint16_t lux_lx[4];
  uint16_t co2_ppm;
  uint16_t presence_state[4];
  uint16_t relay_state[2];
  uint16_t ac_1_command_status;
  uint16_t ac_2_command_status;
  uint16_t projector_command_status;
};

class SlaveRuntime {
private:
    SlaveIdentity _identity;
    SlaveCapability _capability;
    SlaveRuntimeState _state;

public:
    SlaveRuntime();
    void init();

    // Getters and setters
    SlaveIdentity& getIdentity() { return _identity; }
    SlaveCapability& getCapability() { return _capability; }
    SlaveRuntimeState& getState() { return _state; }

    const uint8_t* getMac() const { return _identity.mac; }
    uint16_t getFwVersion() const { return _identity.fw_version; }

    uint8_t getActiveAddress() const { return _state.active_address; }
    void setActiveAddress(uint8_t addr);

    bool isWaitingAtPairingAddress() const { return _state.waiting_at_pairing_address; }
    void setWaitingAtPairingAddress(bool waiting);

    uint16_t getLastError() const { return _state.last_error; }
    void setLastError(uint16_t err) { _state.last_error = err; }
};

// Global runtime instance declaration
extern SlaveRuntime runtime;

#endif // CORE_SLAVE_RUNTIME_H
