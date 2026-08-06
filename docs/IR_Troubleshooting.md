# IR Troubleshooting & Fix Documentation

## The Problem
When integrating the Coolix (Carrier) AC protocol into the Slave firmware, the IR receiver consistently failed to decode the signal, identifying it as `UNKNOWN`. Analysis of the raw timings showed that the mark and space durations were significantly stretched (e.g., a header space of `5050µs` instead of the required `4416µs`). 

## The Root Cause
1. **Software Bit-Banging**: The `IRremoteESP8266` library does not use hardware timers (RMT) for sending on the ESP32 port. Instead, it relies on software bit-banging using `delayMicroseconds()`.
2. **FreeRTOS Time-Slicing**: The `ir_task` in `IRComboModule` was configured to run at FreeRTOS priority `1`. The default Arduino `loopTask` (which handles Modbus and other sensors) also runs at priority `1`.
3. **Preemption**: Because both tasks had equal priority, the FreeRTOS scheduler time-sliced between them. Every 1ms (the system tick rate), the scheduler interrupted the `delayMicroseconds()` loop in the IR sending function to run the Modbus task. This context switch took time and artificially stretched the IR pulse timings beyond acceptable receiver tolerances.

## The Fix
To prevent the scheduler from interrupting the time-sensitive IR transmission, we implemented a dynamic priority boost. 

In `IRComboModule.cpp`, just before dispatching the IR signal, the `ir_task` priority is boosted to the maximum:
```cpp
// Temporarily boost priority to prevent FreeRTOS time-slicing
UBaseType_t oldPriority = uxTaskPriorityGet(NULL);
vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);

// ... IR send function call ...

// Restore original priority
vTaskPrioritySet(NULL, oldPriority);
```
This guarantees that the `ir_task` runs uninterrupted during the bit-banging process, ensuring precise IR timings.

---

## How to Re-enable the Panasonic AC IR

During debugging, the `PanasonicDKE` IR transmission was temporarily commented out in `IRDriver.cpp` to isolate potential interference between the two AC protocols. Now that the timing issue is resolved, it is safe to re-enable it.

### Steps to re-enable safely:

1. Open `src/drivers/IRDriver.cpp`.
2. Locate the `sendPanasonicDke` method (around line 218).
3. **Uncomment** the block responsible for sending the Panasonic signal and the inter-protocol gap.

**Important Considerations to Prevent New Troubles:**

* **Keep the Inter-Protocol Gap**: Ensure the `delay(40);` between `panasonicAc->send();` and `coolixAc->send();` remains. The IR hardware needs a cooldown period to separate the two distinct protocol frames.
* **`delay()` Yields Execution**: Even though the `ir_task` is running at the highest priority during this time, `delay()` internally calls FreeRTOS `vTaskDelay()`. This places the `ir_task` in a "Blocked" state for 40ms, allowing lower-priority tasks (like your Modbus loop) to run smoothly. Thus, it will **not** freeze the system or cause Modbus timeouts.
* **No Code Renaming (Yet)**: Note that the method in `IRDriver` is still named `sendPanasonicDke()`, but it currently sends the Coolix code. If you rename it to something generic like `sendAcCommand()` in the future, remember to update the method signature in `IRDriver.h` and the call in `IRComboModule.cpp`.

### Example of the Re-enabled Method:
```cpp
bool IRDriver::sendPanasonicDke(uint8_t channel, bool power, uint16_t tempX10, uint8_t mode,
                                uint8_t fanSpeed, uint8_t swingVertical, uint8_t swingHorizontal) {
    if (!_initialized || tempX10 < 160 || tempX10 > 300 || tempX10 % 10 != 0 || mode > 4) {
        return false;
    }

    IRPanasonicAc* panasonicAc = nullptr;
    IRCoolixAC* coolixAc = nullptr;
    if (channel == 1) {
        if (!_ac1Initialized) return false;
        panasonicAc = &_panasonicAc1;
        coolixAc = &_coolixAc1;
    } else if (channel == 2) {
        if (!_ac2Initialized) return false;
        panasonicAc = &_panasonicAc2;
        coolixAc = &_coolixAc2;
    } else {
        return false;
    }

    // 1. Send Panasonic DKE AC IR signal
    panasonicAc->setModel(kPanasonicDke);
    panasonicAc->setPower(power);
    panasonicAc->setTemp(static_cast<uint8_t>(tempX10 / 10));
    panasonicAc->setMode(mapPanasonicDkeMode(mode));
    if (!applyPanasonicDkeFan(panasonicAc, fanSpeed) ||
        !applyPanasonicDkeSwingVertical(panasonicAc, swingVertical) ||
        !applyPanasonicDkeSwingHorizontal(panasonicAc, swingHorizontal)) {
        return false;
    }
    panasonicAc->send();

    // 2. Inter-protocol gap (40ms) to ensure clean separation between Panasonic and Carrier IR frames
    // This vTaskDelay allows the Modbus loop to run during the gap!
    delay(40);

    // 3. Send Coolix IR signal using native library send()
    coolixAc->stateReset();
    if (power) {
        coolixAc->setPower(true);
        coolixAc->setTemp(static_cast<uint8_t>(tempX10 / 10));
        coolixAc->setMode(mapCoolixMode(mode));
        if (fanSpeed != 99) {
            coolixAc->setFan(mapCoolixFan(fanSpeed));
        }
    } else {
        coolixAc->off();
    }
    coolixAc->send();

    return true;
}
```
