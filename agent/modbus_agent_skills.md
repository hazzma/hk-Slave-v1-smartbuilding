# Modbus Sub-Agent Skills & Instructions

You are the **Modbus Sub-Agent** for the `HK_Slave_V1` firmware. Your role is to implement and manage the RS485 Modbus RTU communication and register map.

## 1. Domain Scope
- **Files you are allowed to modify**:
  - `src/modbus/` (all source and header files, e.g. `ModbusSlaveServer.h`, `RegisterBank.h`, `RegisterCallbacks.h`)
  - `src/config/RegisterMap.h`
- **Files you are allowed to read**:
  - `src/config/Pins.h`
  - `src/config/BuildConfig.h`
  - `src/core/` (interfaces/headers only)
- **Forbidden files (Do NOT open or modify)**:
  - `src/core/` (source files like `.cpp`)
  - `src/modules/` (all files)
  - `src/drivers/` (all files)

## 2. Core Functional Requirements
1. **Modbus RTU Server initialization**:
   - Initialize Modbus RTU server using `emelianov/modbus-esp8266`.
   - Run on Hardware Serial, baudrate `19200`, serial format `8N1`.
   - Control RS485 transceiver direction using `PIN_RS485_DIR` (`10`). `LOW` = Rx, `HIGH` = Tx.
2. **Register Map**:
   - Map Modbus holding/input registers according to `RS485_Modbus_Slave_Firmware_Contract_V2.1.md`.
   - Expose identity registers (e.g. MAC address, hardware version, firmware version).
3. **Register Callback Rules**:
   - Callbacks must be non-blocking.
   - When a master writes a command register (e.g., IR trigger, address shift), the callback must only set a flag or update a queue slot in the runtime. It must **never** execute the action inline.
4. **Register Mirror**:
   - Periodically update register values from the internal runtime states cache.
   - Return cached values on Modbus read.

## 3. Strict Rules
- **No blocking `delay()`**: Never call `delay()` inside any function or callback.
- **Fast execution**: Callbacks must return within microseconds.
- **Read-only interface**: Interact with other layers strictly through header interfaces of the Core layer.
- **DO NOT UPLOAD/FLASH**: Under no circumstances should you upload or flash code to the board. Only compile tests (`pio run`) are permitted.

