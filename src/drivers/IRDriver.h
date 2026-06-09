#ifndef DRIVERS_IR_DRIVER_H
#define DRIVERS_IR_DRIVER_H

#include <Arduino.h>
#include <IRsend.h>
#include <ir_Panasonic.h>
#include "../config/Pins.h"

class IRDriver {
private:
    IRPanasonicAc _panasonicAc1;
    IRPanasonicAc _panasonicAc2;
    IRsend _irSendProjA;
    IRsend _irSendProjB;
    bool _initialized;
    bool _ac1Initialized;
    bool _ac2Initialized;
    bool _projectorInitialized;

    static uint8_t mapPanasonicDkeMode(uint8_t mode);
    static bool applyPanasonicDkeFan(IRPanasonicAc* ac, uint8_t fanSpeed);
    static bool applyPanasonicDkeSwingVertical(IRPanasonicAc* ac, uint8_t swingVertical);
    static bool applyPanasonicDkeSwingHorizontal(IRPanasonicAc* ac, uint8_t swingHorizontal);

public:
    IRDriver();
    void begin(bool enableAc1, bool enableAc2, bool enableProjector);
    
    /**
     * @brief Send a complete Panasonic DKE AC state.
     * @param channel 1 for AC_1, 2 for AC_2
     * @param power Desired discrete power state.
     * @param tempX10 Desired temperature in Celsius x10, from 160 to 300.
     * @param mode Slave mode enum: 0 cool, 1 dry, 2 fan, 3 heat, 4 auto.
     * @param fanSpeed Fan enum: 0 auto, 1 low, 2 medium, 3 high, 4 quiet, 5 powerful, 99 no change.
     * @param swingVertical Vane enum: 0 fixed, 1 auto, 2..6 positions, 7/8 step, 99 no change.
     * @param swingHorizontal Vane enum: 0 middle, 1 auto, 2..6 positions, 7/8 step, 99 no change.
     * @return true if successfully sent, false otherwise.
     */
    bool sendPanasonicDke(uint8_t channel, bool power, uint16_t tempX10, uint8_t mode,
                          uint8_t fanSpeed, uint8_t swingVertical, uint8_t swingHorizontal);

    /**
     * @brief Send one EPSON NEC code on both projector IR outputs.
     * @param code One 32-bit NEC code from the tested power-toggle sequence.
     * @return true if successfully sent, false otherwise.
     */
    bool sendEpsonProjectorCode(uint32_t code);
    
    bool isInitialized() const { return _initialized; }
};

#endif // DRIVERS_IR_DRIVER_H
