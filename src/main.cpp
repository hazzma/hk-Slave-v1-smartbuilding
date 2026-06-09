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
#include "drivers/RelayDriver.h"

// Modules
#include "modules/DHT22Module.h"
#include "modules/BH1750Module.h"
#include "modules/SCD30Module.h"
#include "modules/PresenceDigitalModule.h"
#include "modules/RelayModule.h"
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
RelayModule relayModule;
IRComboModule irComboModule;

// Instantiate managers globally
CapabilityManager capabilityManager(&dhtModule, &luxModule, &co2Module, &presenceModule, &relayModule, &irComboModule);
CommandManager commandManager(&irComboModule);
ErrorManager errorManager(&dhtModule, &luxModule, &co2Module, &presenceModule, &relayModule, &irComboModule);

// Instantiate Modbus RTU slave server globally
ModbusSlaveServer modbusServer(&dhtModule, &luxModule, &co2Module, &presenceModule, &relayModule, &irComboModule);

void setup() {
    Serial.begin(115200);
    
    // 1. Initialize core system database (efuse MAC read, default address = 247)
    runtime.init();
    
    Serial.printf("Smart Building Modbus Slave Booting... FW Version: %d\n", runtime.getFwVersion());

    // 2. Set pointers used by Modbus callbacks
    globalCapManager = &capabilityManager;
    globalCmdManager = &commandManager;
    globalRelayModule = &relayModule;

    // 3. Initialize managers (synchronize initial state)
    capabilityManager.begin();
    commandManager.begin();
    errorManager.begin();

    // 4. Initialize Modbus RTU server (starts Serial1 on Rx=20, Tx=21, Dir=2)
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
    relayModule.update(now);
    irComboModule.update(now);

    // 4. Update system command manager status state maps
    commandManager.update(now);

    // 5. Update error flag registers
    errorManager.update(now);
}
