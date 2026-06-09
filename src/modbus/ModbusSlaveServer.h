#ifndef MODBUS_SLAVE_SERVER_H
#define MODBUS_SLAVE_SERVER_H

#include <Arduino.h>
#include <ModbusRTU.h>
#include "RegisterBank.h"
#include "RegisterCallbacks.h"
#include "../core/SlaveRuntime.h"

// Define pointer handle to ModbusRTU server globally
extern ModbusRTU* globalMbInstance;

class ModbusSlaveServer {
private:
    ModbusRTU _mb;
    RegisterBank _registerBank;
    uint8_t _activeAddress;
    bool _initialized;

public:
    ModbusSlaveServer(DHT22Module* dht, BH1750Module* lux, SCD30Module* co2, 
                      PresenceDigitalModule* presence, RelayModule* relay, IRComboModule* ir);
    
    void begin();
    
    /**
     * @brief Periodically sync registers and run the Modbus RTU polling task.
     * Also checks if the node address has been dynamically shifted.
     */
    void update(uint32_t now_ms);
};

#endif // MODBUS_SLAVE_SERVER_H
