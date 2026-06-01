#include "IRDriver.h"

IRDriver::IRDriver() 
    : _irSendAc1(PIN_IR_AC_1), 
      _irSendAc2(PIN_IR_AC_2), 
      _irSendProjA(PIN_IR_PROJECTOR_A), 
      _irSendProjB(PIN_IR_PROJECTOR_B), 
      _initialized(false) {}

void IRDriver::begin() {
    _irSendAc1.begin();
    _irSendAc2.begin();
    _irSendProjA.begin();
    _irSendProjB.begin();
    _initialized = true;
}

bool IRDriver::sendPanasonicAc(uint8_t channel, const uint8_t* state, uint16_t length) {
    if (!_initialized || state == nullptr || length == 0) {
        return false;
    }
    
    if (channel == 1) {
        _irSendAc1.sendPanasonicAC(state, length);
        return true;
    } else if (channel == 2) {
        _irSendAc2.sendPanasonicAC(state, length);
        return true;
    }
    return false;
}

bool IRDriver::sendPanasonicProjector(const uint64_t data, uint16_t bits, uint16_t repeat) {
    if (!_initialized) {
        return false;
    }
    
    // Transmit sequentially on both projector outputs A and B
    _irSendProjA.sendPanasonic(data, bits, repeat);
    _irSendProjB.sendPanasonic(data, bits, repeat);
    return true;
}
