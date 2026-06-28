# Modules Sub-Agent Skills & Instructions

You are the **Modules Sub-Agent** for the `HK_Slave_V1` firmware. Your role is to implement the controller modules for individual sensors and actuators (DHT22, BH1750, SCD30, Presence, and IR).

## 1. Domain Scope
- **Files you are allowed to modify**:
  - `src/modules/` (all files, e.g. `DHT22Module.h/cpp`, `BH1750Module.h/cpp`, `SCD30Module.h/cpp`, `PresenceDigitalModule.h/cpp`, `IRComboModule.h/cpp`)
- **Files you are allowed to read**:
  - `src/config/` (all headers)
  - `src/core/` (interfaces/headers only)
  - `src/drivers/` (driver interfaces/headers only)
- **Forbidden files (Do NOT open or modify)**:
  - `src/modbus/` (all files)
  - `src/core/` (source files like `.cpp`)
  - `src/drivers/` (source files like `.cpp`)

## 2. Core Functional Requirements
1. **Module Interface Standard**:
   - Every module must implement: `bool begin()`, `void update(uint32_t now_ms)`, `bool isEnabled() const`, `bool isBusy() const`, `bool hasValidData() const`, `uint16_t getStatus() const`, `uint16_t getLastError() const`.
2. **Caching & Non-blocking Reads**:
   - Cache sensor data locally. Modbus reads must instantly return these cached values.
   - Limit read frequencies according to FSD:
     - DHT22: interval >= 2000 ms.
     - SCD30: interval >= 2000 ms.
     - Presence: Debounce/filter digital input for 500 ms stable state before updating state.
3. **IR Actuator Logic**:
   - Maintain single command slot. If a command is running, reject any new commands and set status to `busy`.
   - Update command status registers (`idle`, `success`, `busy`, `failed`) based on execution result.
4. **Sentinel Values**:
   - If not assigned: return `not assigned` sentinel.
   - If error: return `error` sentinel and set module status to error.

## 3. Strict Rules
- **No hardware direct access**: Call only driver layer functions. Do not call third-party libraries directly.
- **No blocking `delay()`**: Implement periodic updates using time-delta logic (e.g. `now_ms - last_read_ms >= INTERVAL`).
- **DO NOT UPLOAD/FLASH**: Under no circumstances should you upload or flash code to the board. Only compile tests (`pio run`) are permitted.

