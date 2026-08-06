#include "IRDriver.h"

IRDriver::IRDriver() 
    : _panasonicAc1(PIN_IR_AC_1),
      _panasonicAc2(PIN_IR_AC_2),
      _coolixAc1(PIN_IR_AC_1),
      _coolixAc2(PIN_IR_AC_2),
      _irSendProjA(PIN_IR_PROJECTOR_A), 
      _irSendProjB(PIN_IR_PROJECTOR_B), 
      _initialized(false),
      _ac1Initialized(false),
      _ac2Initialized(false),
      _projectorInitialized(false) {}

void IRDriver::begin(bool enableAc1, bool enableAc2, bool enableProjector) {
    if (enableAc1 && !_ac1Initialized) {
        _panasonicAc1.begin();
        _panasonicAc1.setModel(kPanasonicDke);
        _panasonicAc1.setFan(kPanasonicAcFanAuto);
        _panasonicAc1.setSwingVertical(kPanasonicAcSwingVAuto);
        _panasonicAc1.setSwingHorizontal(kPanasonicAcSwingHAuto);

        _coolixAc1.begin();
        _coolixAc1.setFan(kCoolixFanAuto);
        _ac1Initialized = true;
    }

    if (enableAc2 && !_ac2Initialized) {
        _panasonicAc2.begin();
        _panasonicAc2.setModel(kPanasonicDke);
        _panasonicAc2.setFan(kPanasonicAcFanAuto);
        _panasonicAc2.setSwingVertical(kPanasonicAcSwingVAuto);
        _panasonicAc2.setSwingHorizontal(kPanasonicAcSwingHAuto);

        _coolixAc2.begin();
        _coolixAc2.setFan(kCoolixFanAuto);
        _ac2Initialized = true;
    }

    if (enableProjector && !_projectorInitialized) {
        _irSendProjA.begin();
        _irSendProjB.begin();
        _projectorInitialized = true;
    }

    _initialized = _ac1Initialized || _ac2Initialized || _projectorInitialized;
}

uint8_t IRDriver::mapCoolixMode(uint8_t mode) {
    switch (mode) {
        case 0: return kCoolixCool;
        case 1: return kCoolixDry;
        case 2: return kCoolixFan;
        case 3: return kCoolixHeat;
        case 4: return kCoolixAuto;
        default: return kCoolixCool;
    }
}

uint8_t IRDriver::mapCoolixFan(uint8_t fanSpeed) {
    switch (fanSpeed) {
        case 0: return kCoolixFanAuto;
        case 1: return kCoolixFanMin;
        case 2: return kCoolixFanMed;
        case 3: return kCoolixFanMax;
        case 4: return kCoolixFanMin; // Quiet -> Min
        case 5: return kCoolixFanMax; // Powerful -> Max
        default: return kCoolixFanAuto;
    }
}

uint8_t IRDriver::mapPanasonicDkeMode(uint8_t mode) {
    switch (mode) {
        case 0: return kPanasonicAcCool;
        case 1: return kPanasonicAcDry;
        case 2: return kPanasonicAcFan;
        case 3: return kPanasonicAcHeat;
        case 4: return kPanasonicAcAuto;
        default: return kPanasonicAcCool;
    }
}

bool IRDriver::applyPanasonicDkeFan(IRPanasonicAc* ac, uint8_t fanSpeed) {
    switch (fanSpeed) {
        case 0:
            ac->setQuiet(false);
            ac->setPowerful(false);
            ac->setFan(kPanasonicAcFanAuto);
            return true;
        case 1:
            ac->setQuiet(false);
            ac->setPowerful(false);
            ac->setFan(kPanasonicAcFanLow);
            return true;
        case 2:
            ac->setQuiet(false);
            ac->setPowerful(false);
            ac->setFan(kPanasonicAcFanMed);
            return true;
        case 3:
            ac->setQuiet(false);
            ac->setPowerful(false);
            ac->setFan(kPanasonicAcFanHigh);
            return true;
        case 4:
            ac->setPowerful(false);
            ac->setQuiet(true);
            return true;
        case 5:
            ac->setQuiet(false);
            ac->setPowerful(true);
            return true;
        case 99:
            return true;
        default:
            return false;
    }
}

bool IRDriver::applyPanasonicDkeSwingVertical(IRPanasonicAc* ac, uint8_t swingVertical) {
    switch (swingVertical) {
        case 0: ac->setSwingVertical(kPanasonicAcSwingVMiddle); return true;
        case 1: ac->setSwingVertical(kPanasonicAcSwingVAuto); return true;
        case 2: ac->setSwingVertical(kPanasonicAcSwingVHighest); return true;
        case 3: ac->setSwingVertical(kPanasonicAcSwingVHigh); return true;
        case 4: ac->setSwingVertical(kPanasonicAcSwingVMiddle); return true;
        case 5: ac->setSwingVertical(kPanasonicAcSwingVLow); return true;
        case 6: ac->setSwingVertical(kPanasonicAcSwingVLowest); return true;
        case 7: {
            uint8_t current = ac->getSwingVertical();
            if (current < kPanasonicAcSwingVHighest || current >= kPanasonicAcSwingVLowest) {
                current = kPanasonicAcSwingVMiddle;
            }
            ac->setSwingVertical(current + 1);
            return true;
        }
        case 8: {
            uint8_t current = ac->getSwingVertical();
            if (current <= kPanasonicAcSwingVHighest || current > kPanasonicAcSwingVLowest) {
                current = kPanasonicAcSwingVMiddle;
            }
            ac->setSwingVertical(current - 1);
            return true;
        }
        case 9:
        case 10:
        case 99:
            return true;
        default:
            return false;
    }
}

bool IRDriver::applyPanasonicDkeSwingHorizontal(IRPanasonicAc* ac, uint8_t swingHorizontal) {
    switch (swingHorizontal) {
        case 0: ac->setSwingHorizontal(kPanasonicAcSwingHMiddle); return true;
        case 1: ac->setSwingHorizontal(kPanasonicAcSwingHAuto); return true;
        case 2: ac->setSwingHorizontal(kPanasonicAcSwingHFullLeft); return true;
        case 3: ac->setSwingHorizontal(kPanasonicAcSwingHLeft); return true;
        case 4: ac->setSwingHorizontal(kPanasonicAcSwingHMiddle); return true;
        case 5: ac->setSwingHorizontal(kPanasonicAcSwingHRight); return true;
        case 6: ac->setSwingHorizontal(kPanasonicAcSwingHFullRight); return true;
        case 7: {
            switch (ac->getSwingHorizontal()) {
                case kPanasonicAcSwingHFullLeft:
                    ac->setSwingHorizontal(kPanasonicAcSwingHLeft);
                    break;
                case kPanasonicAcSwingHLeft:
                    ac->setSwingHorizontal(kPanasonicAcSwingHMiddle);
                    break;
                case kPanasonicAcSwingHMiddle:
                    ac->setSwingHorizontal(kPanasonicAcSwingHRight);
                    break;
                case kPanasonicAcSwingHRight:
                    ac->setSwingHorizontal(kPanasonicAcSwingHFullRight);
                    break;
                default:
                    ac->setSwingHorizontal(kPanasonicAcSwingHMiddle);
                    break;
            }
            return true;
        }
        case 8: {
            switch (ac->getSwingHorizontal()) {
                case kPanasonicAcSwingHFullRight:
                    ac->setSwingHorizontal(kPanasonicAcSwingHRight);
                    break;
                case kPanasonicAcSwingHRight:
                    ac->setSwingHorizontal(kPanasonicAcSwingHMiddle);
                    break;
                case kPanasonicAcSwingHMiddle:
                    ac->setSwingHorizontal(kPanasonicAcSwingHLeft);
                    break;
                case kPanasonicAcSwingHLeft:
                    ac->setSwingHorizontal(kPanasonicAcSwingHFullLeft);
                    break;
                default:
                    ac->setSwingHorizontal(kPanasonicAcSwingHMiddle);
                    break;
            }
            return true;
        }
        case 9:
        case 10:
        case 99:
            return true;
        default:
            return false;
    }
}

bool IRDriver::sendPanasonicDke(uint8_t channel, bool power, uint16_t tempX10, uint8_t mode,
                                uint8_t fanSpeed, uint8_t swingVertical, uint8_t swingHorizontal) {
    if (!_initialized || tempX10 < 160 || tempX10 > 300 || tempX10 % 10 != 0 || mode > 4) {
        return false;
    }

    IRPanasonicAc* panasonicAc = nullptr;
    IRCoolixAC* coolixAc = nullptr;
    if (channel == 1) {
        if (!_ac1Initialized) return false;
        panasonicAc = &_panasonicAc1;
        coolixAc = &_coolixAc1;
    } else if (channel == 2) {
        if (!_ac2Initialized) return false;
        panasonicAc = &_panasonicAc2;
        coolixAc = &_coolixAc2;
    } else {
        return false;
    }

    // 1. Send Panasonic DKE AC IR signal
    panasonicAc->setModel(kPanasonicDke);
    panasonicAc->setPower(power);
    panasonicAc->setTemp(static_cast<uint8_t>(tempX10 / 10));
    panasonicAc->setMode(mapPanasonicDkeMode(mode));
    if (!applyPanasonicDkeFan(panasonicAc, fanSpeed) ||
        !applyPanasonicDkeSwingVertical(panasonicAc, swingVertical) ||
        !applyPanasonicDkeSwingHorizontal(panasonicAc, swingHorizontal)) {
        return false;
    }
    panasonicAc->send();

    // 2. Inter-protocol gap (40ms) to ensure clean separation between Panasonic and Carrier IR frames
    delay(40);

    // 3. Send Coolix IR signal using native library send()
    coolixAc->stateReset();
    if (power) {
        coolixAc->setPower(true);
        coolixAc->setTemp(static_cast<uint8_t>(tempX10 / 10));
        coolixAc->setMode(mapCoolixMode(mode));
        if (fanSpeed != 99) {
            coolixAc->setFan(mapCoolixFan(fanSpeed));
        }
    } else {
        coolixAc->off();
    }
    coolixAc->send();

    return true;
}

bool IRDriver::sendEpsonProjectorCode(uint32_t code) {
    if (!_projectorInitialized) {
        return false;
    }

    _irSendProjA.sendNEC(code, 32);
    _irSendProjB.sendNEC(code, 32);
    return true;
}
