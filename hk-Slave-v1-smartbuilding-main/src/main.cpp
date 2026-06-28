#include <Arduino.h>
#include <Wire.h>
#include "config/Pins.h"
#include "config/BuildConfig.h"
#include "core/ModuleStatus.h"
#include "core/Scheduler.h"
#include "drivers/DigitalInputDriver.h"
#include "drivers/DHT22Driver.h"
#include "drivers/BH1750Driver.h"
#include "drivers/SCD30Driver.h"
#include "drivers/IRDriver.h"

// Modules
#include "modules/DHT22Module.h"
#include "modules/BH1750Module.h"
#include "modules/SCD30Module.h"
#include "modules/PresenceDigitalModule.h"
#include "modules/IRComboModule.h"

// Core
#include "core/SlaveRuntime.h"
#include "core/CapabilityManager.h"
#include "core/CommandManager.h"
#include "core/ErrorManager.h"

// Modbus
#include "modbus/ModbusSlaveServer.h"

// Instantiate modules globally so they remain in scope for the lifetime of the application
DHT22Module dhtModule;
BH1750Module luxModule;
SCD30Module co2Module;
PresenceDigitalModule presenceModule;
IRComboModule irComboModule;

// Instantiate managers globally
CapabilityManager capabilityManager(&dhtModule, &luxModule, &co2Module, &presenceModule, &irComboModule);
CommandManager commandManager(&irComboModule);
ErrorManager errorManager(&dhtModule, &luxModule, &co2Module, &presenceModule, &irComboModule);

// Instantiate Modbus RTU slave server globally
ModbusSlaveServer modbusServer(&dhtModule, &luxModule, &co2Module, &presenceModule, &irComboModule);

void setup() {
    Serial.begin(115200);
    
    // 1. Initialize core system database (efuse MAC read, default address = 247)
    runtime.init();
    
    Serial.printf("Smart Building Modbus Slave Booting... FW Version: %d\n", runtime.getFwVersion());

    // 2. Set pointers used by Modbus callbacks
    globalCapManager = &capabilityManager;
    globalCmdManager = &commandManager;

    // 3. Initialize managers (synchronize initial state)
    capabilityManager.begin();
    commandManager.begin();
    errorManager.begin();

    // 4. Initialize Modbus RTU server (starts Serial1 on Rx=20, Tx=21, Dir=10)
    modbusServer.begin();

    Serial.printf("Boot complete. Modbus listening on address: %d\n", runtime.getActiveAddress());
}

void loop() {
    uint32_t now = millis();

    // 1. Process Modbus communication tasks (must be called frequently)
    modbusServer.update(now);

    // 2. Dynamic capability assignment monitoring
    capabilityManager.update(now);

    // 3. Poll sensor & actuator modules in background (non-blocking scheduler)
    dhtModule.update(now);
    luxModule.update(now);
    co2Module.update(now);
    presenceModule.update(now);
    irComboModule.update(now);

    // 4. Update system command manager status state maps
    commandManager.update(now);

    // 5. Update error flag registers
    errorManager.update(now);

    // 6. Print BH1750 status/values if Serial is connected (non-blocking, every 2 seconds)
    if (Serial) {
        static uint32_t lastBHPrint = 0;
        if (now - lastBHPrint >= 2000) {
            lastBHPrint = now;
            uint16_t status = luxModule.getStatus();
            uint16_t lux0 = luxModule.getLux(0);
            uint16_t lux1 = luxModule.getLux(1);
            
            Serial.printf("[BH1750] Module Status: %d | Lux0 (0x23): ", status);
            if (lux0 == 0xFFFE) {
                Serial.print("Unassigned");
            } else if (lux0 == 0xFFFF) {
                Serial.print("Error");
            } else {
                Serial.printf("%u lx", lux0);
            }

            Serial.print(" | Lux1 (0x5C): ");
            if (lux1 == 0xFFFE) {
                Serial.print("Unassigned");
            } else if (lux1 == 0xFFFF) {
                Serial.print("Error");
            } else {
                Serial.printf("%u lx", lux1);
            }
            Serial.println();
        }
    }
}
