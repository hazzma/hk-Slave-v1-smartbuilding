#ifndef CORE_ERROR_MANAGER_H
#define CORE_ERROR_MANAGER_H

#include <Arduino.h>
#include "SlaveRuntime.h"
#include "../modules/DHT22Module.h"
#include "../modules/BH1750Module.h"
#include "../modules/SCD30Module.h"
#include "../modules/PresenceDigitalModule.h"
#include "../modules/IRComboModule.h"

// System Error codes from contract
#define SLAVE_ERR_NONE              0
#define SLAVE_ERR_SENSOR_TIMEOUT    1
#define SLAVE_ERR_SENSOR_CRC        2
#define SLAVE_ERR_SENSOR_RANGE      3
#define SLAVE_ERR_CONFIG_RUNTIME    4
#define SLAVE_ERR_BAD_ADDRESS       5
#define SLAVE_ERR_UNSUPPORTED_WRITE 6
#define SLAVE_ERR_BUSY              7
#define SLAVE_ERR_RECOVERY_MAC      8
#define SLAVE_ERR_IR_COMMAND_FAILED 9

class ErrorManager {
private:
    DHT22Module* _dhtModule;
    BH1750Module* _luxModule;
    SCD30Module* _co2Module;
    PresenceDigitalModule* _presenceModule;
    IRComboModule* _irModule;

public:
    ErrorManager(DHT22Module* dht, BH1750Module* lux, SCD30Module* co2, 
                 PresenceDigitalModule* presence, IRComboModule* ir);

    void begin();
    
    /**
     * @brief Report a system-level error directly.
     */
    void setSystemError(uint16_t errorCode);

    /**
     * @brief Clear the system error code.
     */
    void clearError();

    /**
     * @brief Poll errors from active modules and update the runtime state error code.
     */
    void update(uint32_t now_ms);
};

#endif // CORE_ERROR_MANAGER_H
