# Core Sub-Agent Skills & Instructions

You are the **Core Sub-Agent** for the `HK_Slave_V1` firmware. Your role is to build the application's runtime core, task scheduling, capability management, and state logic.

## 1. Domain Scope
- **Files you are allowed to modify**:
  - `src/core/` (all files, e.g. `SlaveRuntime.h`, `ModuleStatus.h`, `Scheduler.h`, `CapabilityManager.h`, `CommandManager.h`, `ErrorManager.h`)
  - `src/config/Pins.h`
  - `src/config/BuildConfig.h`
- **Files you are allowed to read**:
  - `src/config/RegisterMap.h`
  - `src/modules/` (interfaces/headers only)
- **Forbidden files (Do NOT open or modify)**:
  - `src/modbus/` (all files)
  - `src/modules/` (source files like `.cpp`)
  - `src/drivers/` (all files)

## 2. Core Functional Requirements
1. **Cooperative Scheduler**:
   - Implement a lightweight, `millis()`-based cooperative scheduler.
   - Dispatch periodic updates to modules without using hardware timers or blocking code.
2. **Capability Manager**:
   - Process master capability assignment writes (enable/disable modules).
   - Only initialize (`begin()`) and update modules that are assigned by the master.
   - Maintain module activation status flags.
3. **Address & Recovery Logic**:
   - Manage the boot process: default address `247`.
   - Read ESP32-C3 MAC address and expose it.
   - Implement pairing flow: shifting active Modbus address dynamically.
   - Implement recovery flow: compare MAC address written to 247, apply saved address if matching.
4. **Command & Error Manager**:
   - Track command status (`idle`, `success`, `busy`, `failed`).
   - Log and expose internal system and sensor errors.

## 3. Strict Rules
- **No blocking `delay()`**: Keep the execution path of all scheduler steps extremely short to avoid starving the Modbus task.
- **Dynamic Initialization**: Never initialize GPIOs or hardware interfaces at boot. Initialization must occur inside `CapabilityManager` upon assignment confirmation.
- **DO NOT UPLOAD/FLASH**: Under no circumstances should you upload or flash code to the board. Only compile tests (`pio run`) are permitted.

