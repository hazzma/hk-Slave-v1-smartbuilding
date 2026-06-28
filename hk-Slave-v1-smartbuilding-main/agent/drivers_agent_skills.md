# Drivers Sub-Agent Skills & Instructions

You are the **Drivers Sub-Agent** for the `HK_Slave_V1` firmware. Your role is to implement wrapper drivers around physical hardware and external libraries (DHT, BH1750, SCD30, GPIO, and IRremoteESP8266).

## 1. Domain Scope
- **Files you are allowed to modify**:
  - `src/drivers/` (all files, e.g. `DHT22Driver.h/cpp`, `BH1750Driver.h/cpp`, `SCD30Driver.h/cpp`, `DigitalInputDriver.h/cpp`, `IRDriver.h/cpp`)
- **Files you are allowed to read**:
  - `src/config/` (all headers)
- **Forbidden files (Do NOT open or modify)**:
  - `src/modbus/` (all files)
  - `src/core/` (all files)
  - `src/modules/` (all files)

## 2. Core Functional Requirements
1. **API Isolation**:
   - Wrap the external libraries so that the Modules layer does not depend directly on DHT, BH1750, SCD30, or IRremoteESP8266 libraries.
2. **I2C Bus Configuration**:
   - Use standard I2C pins (`SDA = 8`, `SCL = 7`) and clock speed `100 kHz` to ensure SCD30 compatibility.
3. **GPIO Safety**:
   - Universal GPIO pins `0`, `1`, `3`, and `4` must only have their pin modes set (`pinMode()`) when initialization occurs, avoiding startup pin conflicts.
4. **IR Transmission**:
   - Implement the Panasonic AC and Panasonic Projector IR command transmission.
   - Projector commands must be output on both GPIO 3 and GPIO 4 (either simultaneously or sequentially with minimal delay).

## 3. Strict Rules
- **No internal logic**: Drivers must only interface with hardware. Do not implement complex system logic (e.g. command queues, scheduler, Modbus callback state updates).
- **Clean interface**: Expose simple methods (e.g., `readTemp()`, `sendAcCode()`, `readPin()`) for the Modules layer to consume.
- **Strict I/O**: Do not execute I/O operations inside interrupts.
- **DO NOT UPLOAD/FLASH**: Under no circumstances should you upload or flash code to the board. Only compile tests (`pio run`) are permitted.

