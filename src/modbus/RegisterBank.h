#ifndef MODBUS_REGISTER_BANK_H
#define MODBUS_REGISTER_BANK_H

#include <Arduino.h>
#include <ModbusRTU.h>
#include "../core/SlaveRuntime.h"

// Sensor module headers to query values
#include "../modules/DHT22Module.h"
#include "../modules/BH1750Module.h"
#include "../modules/SCD30Module.h"
#include "../modules/PresenceDigitalModule.h"
#include "../modules/IRComboModule.h"

class RegisterBank {
private:
    ModbusRTU& _mb;
    
    DHT22Module* _dht;
    BH1750Module* _lux;
    SCD30Module* _co2;
    PresenceDigitalModule* _presence;
    IRComboModule* _ir;

public:
    RegisterBank(ModbusRTU& mb, DHT22Module* dht, BH1750Module* lux, SCD30Module* co2, 
                 PresenceDigitalModule* presence, IRComboModule* ir);

    /**
     * @brief Setup and declare all Modbus holding registers in the ModbusRTU instance.
     */
    void initRegisters();
    
    /**
     * @brief Sync current runtime parameters and sensor values to the Modbus holding registers.
     */
    void update(uint32_t now_ms);
};

#endif // MODBUS_REGISTER_BANK_H
