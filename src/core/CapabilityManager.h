#ifndef CORE_CAPABILITY_MANAGER_H
#define CORE_CAPABILITY_MANAGER_H

#include <Arduino.h>
#include "SlaveRuntime.h"
#include "../modules/DHT22Module.h"
#include "../modules/BH1750Module.h"
#include "../modules/SCD30Module.h"
#include "../modules/PresenceDigitalModule.h"
#include "../modules/IRComboModule.h"

class CapabilityManager {
private:
    DHT22Module* _dhtModule;
    BH1750Module* _luxModule;
    SCD30Module* _co2Module;
    PresenceDigitalModule* _presenceModule;
    IRComboModule* _irModule;

    uint8_t _lastTempMask;
    uint8_t _lastLuxMask;
    uint8_t _lastCo2Count;
    uint8_t _lastPresenceMask;
    uint8_t _lastIrProj;
    uint8_t _lastIrAc1;
    uint8_t _lastIrAc2;

public:
    CapabilityManager(DHT22Module* dht, BH1750Module* lux, SCD30Module* co2, 
                      PresenceDigitalModule* presence, IRComboModule* ir);
    
    void begin();
    
    /**
     * @brief Checks if capability assignment registers changed and updates module states.
     */
    void update(uint32_t now_ms);
};

#endif // CORE_CAPABILITY_MANAGER_H
