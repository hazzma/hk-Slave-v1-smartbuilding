#include "IRComboModule.h"

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

IRComboModule::IRComboModule() 
    : _driver(new IRDriver()), 
      _enabled(false), 
      _status(MODULE_DISABLED), 
      _lastError(0), 
      _commandPending(false), 
      _busy(false), 
      _commandStartMs(0), 
      _cooldownStartMs(0), 
      _projectorSequenceActive(false),
      _projectorSequenceStep(0),
      _ac1Enabled(false), 
      _ac2Enabled(false), 
      _projEnabled(false), 
      _ac1CmdStatus(CMD_IDLE), 
      _ac2CmdStatus(CMD_IDLE), 
      _projCmdStatus(CMD_IDLE),
      _irTaskHandle(nullptr),
      _irTriggerSem(nullptr),
      _irTaskRunning(false),
      _irTaskResultReady(false),
      _irTaskSuccess(false) {
      
    _activeCmd = { IR_TARGET_AC1, false, 0, 0, 99, 99, 99 };
    _stagedCmd[0] = { IR_TARGET_AC1, false, 0, 0, 99, 99, 99 };
    _stagedCmd[1] = { IR_TARGET_AC2, false, 0, 0, 99, 99, 99 };
    _stagedPending[0] = false;
    _stagedPending[1] = false;
    _settlingStartMs[0] = 0;
    _settlingStartMs[1] = 0;
}

IRComboModule::~IRComboModule() {
    if (_irTaskHandle != nullptr) {
        vTaskDelete(_irTaskHandle);
        _irTaskHandle = nullptr;
    }
    if (_irTriggerSem != nullptr) {
        vSemaphoreDelete(_irTriggerSem);
        _irTriggerSem = nullptr;
    }
    delete _driver;
}

// ---------------------------------------------------------------------------
// FreeRTOS IR Task
// ---------------------------------------------------------------------------

/**
 * Runs on its own FreeRTOS task (priority 1, same as Arduino loop).
 * Waits for the binary semaphore, then executes the blocking AC IR send.
 * The Arduino delay() and the RMT hardware-wait both call vTaskDelay()
 * internally, so the scheduler can run the Modbus loop while we wait.
 *
 * Only the volatile result flags are written here; all status-register
 * updates happen back in update() on the main loop.
 */
void IRComboModule::irTaskFunction(void* pvParam) {
    IRComboModule* self = static_cast<IRComboModule*>(pvParam);
    for (;;) {
        // Block forever until the main loop releases the semaphore
        xSemaphoreTake(self->_irTriggerSem, portMAX_DELAY);

        bool success = false;
        IrTarget target = self->_activeCmd.target;

        if (target == IR_TARGET_AC1 || target == IR_TARGET_AC2) {
            uint8_t channel = (target == IR_TARGET_AC1) ? 1 : 2;
            // This call is blocking (~150-200 ms) but yields to other tasks
            // during the hardware-driven delay(40) and RMT wait.
            success = self->_driver->sendPanasonicDke(
                channel,
                self->_activeCmd.power,
                self->_activeCmd.value,
                self->_activeCmd.mode,
                self->_activeCmd.fanSpeed,
                self->_activeCmd.swingVertical,
                self->_activeCmd.swingHorizontal);
        }

        self->_irTaskSuccess   = success;
        self->_irTaskResultReady = true;  // main loop picks this up in update()
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool IRComboModule::begin() {
    _status = MODULE_INIT;
    if (_ac1Enabled || _ac2Enabled || _projEnabled) {
        _driver->begin(_ac1Enabled, _ac2Enabled, _projEnabled);
    }

    // Create the binary semaphore used to trigger the IR task
    if (_irTriggerSem == nullptr) {
        _irTriggerSem = xSemaphoreCreateBinary();
    }

    // Create the IR worker task once (priority 1 = same as Arduino loop)
    if (_irTaskHandle == nullptr && _irTriggerSem != nullptr) {
        xTaskCreate(irTaskFunction, "ir_task", 4096, this, 1, &_irTaskHandle);
    }

    _status = MODULE_READY;
    return true;
}

// ---------------------------------------------------------------------------
// Main loop update
// ---------------------------------------------------------------------------

void IRComboModule::update(uint32_t now_ms) {
    if (!_enabled) {
        _status = MODULE_DISABLED;
        return;
    }

    _status = MODULE_READY;

    // 1. Collect result from the IR task when it finishes
    if (_irTaskRunning && _irTaskResultReady) {
        _irTaskRunning    = false;
        _irTaskResultReady = false;
        bool success       = _irTaskSuccess;

        if (_activeCmd.target == IR_TARGET_AC1) {
            _ac1CmdStatus = success ? CMD_SUCCESS : CMD_FAILED;
        } else if (_activeCmd.target == IR_TARGET_AC2) {
            _ac2CmdStatus = success ? CMD_SUCCESS : CMD_FAILED;
        }

        if (!success) {
            _lastError = 9; // SLAVE_ERR_IR_COMMAND_FAILED
            _status = MODULE_ERROR;
        } else {
            _lastError = 0;
        }
        _cooldownStartMs = now_ms;  // start cooldown from completion time
    }

    // 2. Check cooldown after AC IR task or projector sequence finishes
    if (_busy && !_irTaskRunning && !_commandPending && !_projectorSequenceActive) {
        if (now_ms - _cooldownStartMs >= 1000) {
            _busy = false;
            // Return status to IDLE only if no staged command is waiting
            if (_activeCmd.target == IR_TARGET_AC1 && !_stagedPending[0]) _ac1CmdStatus = CMD_IDLE;
            else if (_activeCmd.target == IR_TARGET_AC2 && !_stagedPending[1]) _ac2CmdStatus = CMD_IDLE;
            else if (_activeCmd.target == IR_TARGET_PROJECTOR) _projCmdStatus = CMD_IDLE;
        }
    }

    // 3. Promote staged AC commands to active after 100 ms settling window
    //    (allows Master to finish writing all 6 AC registers before IR fires)
    if (!_busy && !_commandPending && !_projectorSequenceActive) {
        for (uint8_t idx = 0; idx < 2; idx++) {
            if (_stagedPending[idx]) {
                if (now_ms - _settlingStartMs[idx] >= 100) {
                    _activeCmd = _stagedCmd[idx];
                    _stagedPending[idx] = false;
                    _commandPending = true;
                    _busy = true;
                    if (idx == 0) _ac1CmdStatus = CMD_BUSY;
                    else _ac2CmdStatus = CMD_BUSY;
                    break;
                }
            }
        }
    }

    // 4. Dispatch the pending command
    if (_commandPending) {
        _commandPending = false;

        // Lazy driver initialisation
        _driver->begin(_ac1Enabled, _ac2Enabled, _projEnabled);

        if (_activeCmd.target == IR_TARGET_PROJECTOR) {
            // Projector sequence stays in the main loop (short steps, 40 ms gaps)
            _projectorSequenceActive = true;
            _projectorSequenceStep   = 0;
            _commandStartMs          = now_ms;
        } else {
            // AC: offload blocking IR send to the FreeRTOS task
            _irTaskRunning    = true;
            _irTaskResultReady = false;
            xSemaphoreGive(_irTriggerSem);  // wake irTaskFunction
        }
    }

    // 5. Step through projector sequence (stays on main loop; each step is short)
    if (_projectorSequenceActive) {
        processProjectorSequence(now_ms);
    }
}

// ---------------------------------------------------------------------------
// Projector sequence (main-loop, step-based)
// ---------------------------------------------------------------------------

void IRComboModule::processProjectorSequence(uint32_t now_ms) {
    constexpr uint32_t EPSON_CODE_1 = 0x81C00FF0;
    constexpr uint32_t EPSON_CODE_2 = 0xC1AA09F6;
    constexpr uint8_t  TOTAL_SEQUENCE_STEPS = 4;
    constexpr uint16_t INTER_CODE_GAP_MS    = 40;

    if (now_ms - _commandStartMs < INTER_CODE_GAP_MS && _projectorSequenceStep > 0) {
        return;
    }

    uint32_t code = (_projectorSequenceStep % 2 == 0) ? EPSON_CODE_1 : EPSON_CODE_2;
    if (!_driver->sendEpsonProjectorCode(code)) {
        _projectorSequenceActive = false;
        _projCmdStatus = CMD_FAILED;
        _lastError = 9; // SLAVE_ERR_IR_COMMAND_FAILED
        _status = MODULE_ERROR;
        _cooldownStartMs = now_ms;
        return;
    }

    _projectorSequenceStep++;
    _commandStartMs = now_ms;

    if (_projectorSequenceStep >= TOTAL_SEQUENCE_STEPS) {
        _projectorSequenceActive = false;
        _projCmdStatus = CMD_SUCCESS;
        _lastError = 0;
        _cooldownStartMs = now_ms;
    }
}

// ---------------------------------------------------------------------------
// Enable / disable
// ---------------------------------------------------------------------------

void IRComboModule::setEnabled(bool enabled) {
    _enabled = enabled;
    if (!_enabled) {
        _status = MODULE_DISABLED;
    }
}

// ---------------------------------------------------------------------------
// Command validation helpers
// ---------------------------------------------------------------------------

static bool isSupportedAcFan(uint8_t fanSpeed) {
    return fanSpeed <= 5 || fanSpeed == 99;
}

static bool isSupportedAcSwing(uint8_t swing) {
    return swing <= 10 || swing == 99;
}

// ---------------------------------------------------------------------------
// queueAcCommand – stage parameters, reset settling timer
// ---------------------------------------------------------------------------

bool IRComboModule::queueAcCommand(uint8_t channel, bool power, uint16_t tempX10, uint8_t mode,
                                   uint8_t fanSpeed, uint8_t swingVertical, uint8_t swingHorizontal) {
    if (!_enabled) {
        if (channel == 1) _ac1CmdStatus = CMD_FAILED;
        else if (channel == 2) _ac2CmdStatus = CMD_FAILED;
        return false;
    }
    
    if (channel == 1 && !_ac1Enabled) {
        _ac1CmdStatus = CMD_FAILED;
        return false;
    }
    if (channel == 2 && !_ac2Enabled) {
        _ac2CmdStatus = CMD_FAILED;
        return false;
    }

    if (tempX10 < 160 || tempX10 > 300 || tempX10 % 10 != 0 || mode > 4 ||
        !isSupportedAcFan(fanSpeed) || !isSupportedAcSwing(swingVertical) ||
        !isSupportedAcSwing(swingHorizontal)) {
        if (channel == 1) _ac1CmdStatus = CMD_FAILED;
        else _ac2CmdStatus = CMD_FAILED;
        return false;
    }
    
    // Always stage – this resets the settling timer even when busy,
    // so the latest value from Master is used when the IR task fires next.
    uint8_t idx = (channel == 1) ? 0 : 1;
    _stagedCmd[idx].target         = (channel == 1) ? IR_TARGET_AC1 : IR_TARGET_AC2;
    _stagedCmd[idx].power          = power;
    _stagedCmd[idx].value          = tempX10;
    _stagedCmd[idx].mode           = mode;
    _stagedCmd[idx].fanSpeed       = fanSpeed;
    _stagedCmd[idx].swingVertical  = swingVertical;
    _stagedCmd[idx].swingHorizontal = swingHorizontal;

    _stagedPending[idx]    = true;
    _settlingStartMs[idx]  = millis();

    if (channel == 1) _ac1CmdStatus = CMD_BUSY;
    else _ac2CmdStatus = CMD_BUSY;

    return true;
}

// ---------------------------------------------------------------------------
// queueProjectorCommand
// ---------------------------------------------------------------------------

bool IRComboModule::queueProjectorCommand(bool power, uint16_t inputVal) {
    if (!_enabled || !_projEnabled) {
        _projCmdStatus = CMD_FAILED;
        return false;
    }

    if (_busy) {
        _projCmdStatus = CMD_BUSY;
        return false;
    }

    _activeCmd.target          = IR_TARGET_PROJECTOR;
    _activeCmd.power           = power;
    _activeCmd.value           = inputVal;
    _activeCmd.mode            = 0;
    _activeCmd.fanSpeed        = 99;
    _activeCmd.swingVertical   = 99;
    _activeCmd.swingHorizontal = 99;

    _commandPending = true;
    _busy           = true;
    _projCmdStatus  = CMD_BUSY;

    return true;
}

// ---------------------------------------------------------------------------
// Status
// ---------------------------------------------------------------------------

uint16_t IRComboModule::getStatus() const {
    return static_cast<uint16_t>(_status);
}
