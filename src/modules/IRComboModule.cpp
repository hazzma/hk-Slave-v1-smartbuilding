#include "IRComboModule.h"

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
      _projCmdStatus(CMD_IDLE) {
      
    _activeCmd = { IR_TARGET_AC1, false, 0, 0, 99, 99, 99 };
}

IRComboModule::~IRComboModule() {
    delete _driver;
}

bool IRComboModule::begin() {
    _status = MODULE_INIT;
    if (_ac1Enabled || _ac2Enabled || _projEnabled) {
        _driver->begin(_ac1Enabled, _ac2Enabled, _projEnabled);
    }
    _status = MODULE_READY;
    return true;
}

void IRComboModule::update(uint32_t now_ms) {
    if (!_enabled) {
        _status = MODULE_DISABLED;
        return;
    }

    _status = MODULE_READY;

    // Check if we are in cooldown after executing an IR transmit
    if (_busy && !_commandPending && !_projectorSequenceActive) {
        if (now_ms - _cooldownStartMs >= 1000) {
            _busy = false;
            // Transition the status of the finished command channel back to IDLE
            if (_activeCmd.target == IR_TARGET_AC1) _ac1CmdStatus = CMD_IDLE;
            else if (_activeCmd.target == IR_TARGET_AC2) _ac2CmdStatus = CMD_IDLE;
            else if (_activeCmd.target == IR_TARGET_PROJECTOR) _projCmdStatus = CMD_IDLE;
        }
    }

    if (_commandPending) {
        _busy = true;
        _commandPending = false;
        
        // Lazy initialization
        _driver->begin(_ac1Enabled, _ac2Enabled, _projEnabled);

        if (_activeCmd.target == IR_TARGET_PROJECTOR) {
            _projectorSequenceActive = true;
            _projectorSequenceStep = 0;
            _commandStartMs = now_ms;
        } else {
            executeCommand();
            _cooldownStartMs = now_ms;
        }
    }

    if (_projectorSequenceActive) {
        processProjectorSequence(now_ms);
    }
}

void IRComboModule::executeCommand() {
    bool success = false;
    
    if (_activeCmd.target == IR_TARGET_AC1 || _activeCmd.target == IR_TARGET_AC2) {
        uint8_t channel = (_activeCmd.target == IR_TARGET_AC1) ? 1 : 2;

        success = _driver->sendPanasonicDke(
            channel, _activeCmd.power, _activeCmd.value, _activeCmd.mode,
            _activeCmd.fanSpeed, _activeCmd.swingVertical, _activeCmd.swingHorizontal);

        if (channel == 1) {
            _ac1CmdStatus = success ? CMD_SUCCESS : CMD_FAILED;
        } else {
            _ac2CmdStatus = success ? CMD_SUCCESS : CMD_FAILED;
        }

    }

    if (!success) {
        _lastError = 9; // SLAVE_ERR_IR_COMMAND_FAILED
        _status = MODULE_ERROR;
    } else {
        _lastError = 0;
    }
}

void IRComboModule::processProjectorSequence(uint32_t now_ms) {
    constexpr uint32_t EPSON_CODE_1 = 0x81C00FF0;
    constexpr uint32_t EPSON_CODE_2 = 0xC1AA09F6;
    constexpr uint8_t TOTAL_SEQUENCE_STEPS = 4;
    constexpr uint16_t INTER_CODE_GAP_MS = 40;

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

void IRComboModule::setEnabled(bool enabled) {
    _enabled = enabled;
    if (!_enabled) {
        _status = MODULE_DISABLED;
    }
}

static bool isSupportedAcFan(uint8_t fanSpeed) {
    return fanSpeed <= 5 || fanSpeed == 99;
}

static bool isSupportedAcSwing(uint8_t swing) {
    return swing <= 10 || swing == 99;
}

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
    
    if (_busy) {
        if (channel == 1) _ac1CmdStatus = CMD_BUSY;
        else _ac2CmdStatus = CMD_BUSY;
        return false;
    }

    _activeCmd.target = (channel == 1) ? IR_TARGET_AC1 : IR_TARGET_AC2;
    _activeCmd.power = power;
    _activeCmd.value = tempX10;
    _activeCmd.mode = mode;
    _activeCmd.fanSpeed = fanSpeed;
    _activeCmd.swingVertical = swingVertical;
    _activeCmd.swingHorizontal = swingHorizontal;
    
    _commandPending = true;
    _busy = true;

    if (channel == 1) _ac1CmdStatus = CMD_BUSY;
    else _ac2CmdStatus = CMD_BUSY;

    return true;
}

bool IRComboModule::queueProjectorCommand(bool power, uint16_t inputVal) {
    if (!_enabled || !_projEnabled) {
        _projCmdStatus = CMD_FAILED;
        return false;
    }

    if (_busy) {
        _projCmdStatus = CMD_BUSY;
        return false;
    }

    _activeCmd.target = IR_TARGET_PROJECTOR;
    _activeCmd.power = power;
    _activeCmd.value = inputVal;
    _activeCmd.mode = 0;
    _activeCmd.fanSpeed = 99;
    _activeCmd.swingVertical = 99;
    _activeCmd.swingHorizontal = 99;

    _commandPending = true;
    _busy = true;
    _projCmdStatus = CMD_BUSY;

    return true;
}

uint16_t IRComboModule::getStatus() const {
    return static_cast<uint16_t>(_status);
}
