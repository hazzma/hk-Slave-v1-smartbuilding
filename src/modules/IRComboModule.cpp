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
      _ac1Enabled(false), 
      _ac2Enabled(false), 
      _projEnabled(false), 
      _ac1CmdStatus(CMD_IDLE), 
      _ac2CmdStatus(CMD_IDLE), 
      _projCmdStatus(CMD_IDLE) {
      
    _activeCmd = { IR_TARGET_AC1, false, 0, 0 };
}

IRComboModule::~IRComboModule() {
    delete _driver;
}

bool IRComboModule::begin() {
    _status = MODULE_INIT;
    if (_ac1Enabled || _ac2Enabled || _projEnabled) {
        _driver->begin();
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
    if (_busy && !_commandPending) {
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
        if (!_driver->isInitialized()) {
            _driver->begin();
        }

        executeCommand();
        _cooldownStartMs = now_ms;
    }
}

void IRComboModule::executeCommand() {
    bool success = false;
    
    if (_activeCmd.target == IR_TARGET_AC1 || _activeCmd.target == IR_TARGET_AC2) {
        uint8_t channel = (_activeCmd.target == IR_TARGET_AC1) ? 1 : 2;
        
        // Form a standard 27-byte Panasonic AC protocol state payload
        uint8_t state[27] = {
            0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06,
            0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x80,
            0x30, 0x20, 0x80, 0x0F, 0x00, 0x00, 0x0E, 0xE0,
            0x00, 0x00, 0x00
        };

        // Extract temperature integer (defaults to 24C if not specified or out of bounds)
        uint8_t tempC = static_cast<uint8_t>(_activeCmd.value / 10);
        if (tempC < 16) tempC = 16;
        if (tempC > 30) tempC = 30;
        state[19] = tempC * 2; 

        // Set Mode (Cool = 0, Dry = 1, Fan = 2, Heat = 3, etc.)
        state[13] = _activeCmd.mode;

        // Set Power (1 = On, 0 = Off)
        state[14] = _activeCmd.power ? 0x09 : 0x08;

        // Recalculate sum checksum at byte index 26
        uint8_t checksum = 0;
        for (int i = 0; i < 26; i++) {
            checksum += state[i];
        }
        state[26] = checksum;

        success = _driver->sendPanasonicAc(channel, state, sizeof(state));

        if (channel == 1) {
            _ac1CmdStatus = success ? CMD_SUCCESS : CMD_FAILED;
        } else {
            _ac2CmdStatus = success ? CMD_SUCCESS : CMD_FAILED;
        }

    } else if (_activeCmd.target == IR_TARGET_PROJECTOR) {
        // Panasonic Projector standard codes (usually 48-bit Panasonic TV-like protocols)
        // 0x4004072AULL is an example power toggling payload
        uint64_t data = _activeCmd.power ? 0x4004072AULL : 0x4004072BULL; 
        
        success = _driver->sendPanasonicProjector(data, 48);
        _projCmdStatus = success ? CMD_SUCCESS : CMD_FAILED;
    }

    if (!success) {
        _lastError = 9; // SLAVE_ERR_IR_COMMAND_FAILED
        _status = MODULE_ERROR;
    } else {
        _lastError = 0;
    }
}

void IRComboModule::setEnabled(bool enabled) {
    _enabled = enabled;
    if (!_enabled) {
        _status = MODULE_DISABLED;
    }
}

bool IRComboModule::queueAcCommand(uint8_t channel, bool power, uint16_t tempX10, uint8_t mode) {
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
    
    if (_busy) {
        if (channel == 1) _ac1CmdStatus = CMD_BUSY;
        else _ac2CmdStatus = CMD_BUSY;
        return false;
    }

    _activeCmd.target = (channel == 1) ? IR_TARGET_AC1 : IR_TARGET_AC2;
    _activeCmd.power = power;
    _activeCmd.value = tempX10;
    _activeCmd.mode = mode;
    
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

    _commandPending = true;
    _busy = true;
    _projCmdStatus = CMD_BUSY;

    return true;
}

uint16_t IRComboModule::getStatus() const {
    return static_cast<uint16_t>(_status);
}
