#ifndef DRIVERS_IR_DRIVER_H
#define DRIVERS_IR_DRIVER_H

#include <Arduino.h>
#include <IRsend.h>
#include "../config/Pins.h"

class IRDriver {
private:
    IRsend _irSendAc1;
    IRsend _irSendAc2;
    IRsend _irSendProjA;
    IRsend _irSendProjB;
    bool _initialized;

public:
    IRDriver();
    void begin();
    
    /**
     * @brief Send a Panasonic AC command using a state byte array.
     * @param channel 1 for AC_1, 2 for AC_2
     * @param state Byte array representing the Panasonic AC state (usually 27 bytes).
     * @param length Length of the byte array.
     * @return true if successfully sent, false otherwise.
     */
    bool sendPanasonicAc(uint8_t channel, const uint8_t* state, uint16_t length);

    /**
     * @brief Send a Panasonic Projector IR command.
     * Projector commands are sent on both PIN_IR_PROJECTOR_A and PIN_IR_PROJECTOR_B.
     * @param data The IR payload (Panasonic protocol payload).
     * @param bits The number of bits to transmit.
     * @param repeat Number of times to repeat the command transmission.
     * @return true if successfully sent, false otherwise.
     */
    bool sendPanasonicProjector(const uint64_t data, uint16_t bits, uint16_t repeat = 0);
    
    bool isInitialized() const { return _initialized; }
};

#endif // DRIVERS_IR_DRIVER_H
