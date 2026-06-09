#include "ModbusSlaveServer.h"
#include "../config/Pins.h"
#include "../config/BuildConfig.h"

// Define the global ModbusRTU pointer
ModbusRTU* globalMbInstance = nullptr;

ModbusSlaveServer::ModbusSlaveServer(DHT22Module* dht, BH1750Module* lux, SCD30Module* co2,
                                     PresenceDigitalModule* presence, RelayModule* relay, IRComboModule* ir)
    : _registerBank(_mb, dht, lux, co2, presence, relay, ir),
      _activeAddress(DEFAULT_MODBUS_ADDRESS),
      _initialized(false) {}

void ModbusSlaveServer::begin() {
    globalMbInstance = &_mb;

    // Initialize UART1 for RS485 communication
    Serial1.begin(MODBUS_BAUDRATE, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);

    // Pass Serial1 stream and hardware direction control pin.
    _mb.begin(&Serial1, PIN_RS485_DIR);
    
    // Set Modbus slave address
    _mb.slave(runtime.getActiveAddress());
    
    // Initialize Modbus register map
    _registerBank.initRegisters();
    
    // Setup write callbacks
    RegisterCallbacks::setupCallbacks(_mb);

    _initialized = true;
}

void ModbusSlaveServer::update(uint32_t now_ms) {
    if (!_initialized) {
        return;
    }

    // Check if node address has been shifted dynamically (pairing or recovery)
    uint8_t currentAddress = runtime.getActiveAddress();
    if (currentAddress != _activeAddress) {
        _activeAddress = currentAddress;
        _mb.slave(_activeAddress);
    }

    // Sync latest cached values from runtime/modules to holding registers
    _registerBank.update(now_ms);

    // Process Modbus RTU communications
    _mb.task();
}
