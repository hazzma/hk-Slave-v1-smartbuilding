#include "CommandManager.h"

CommandManager::CommandManager(IRComboModule* ir) : _irModule(ir) {}

void CommandManager::begin() {
    // Initial sync
    update(millis());
}

bool CommandManager::handleAcWrite(uint8_t channel, bool power, uint16_t tempX10, uint8_t mode) {
    if (_irModule == nullptr || !_irModule->isEnabled()) {
        return false;
    }
    return _irModule->queueAcCommand(channel, power, tempX10, mode);
}

bool CommandManager::handleProjectorWrite(bool power, uint16_t inputVal) {
    if (_irModule == nullptr || !_irModule->isEnabled()) {
        return false;
    }
    return _irModule->queueProjectorCommand(power, inputVal);
}

void CommandManager::update(uint32_t now_ms) {
    if (_irModule == nullptr) {
        return;
    }
    
    // Sync module status variables to runtime state registers
    SlaveRuntimeState& state = runtime.getState();
    state.ac_1_command_status = _irModule->getAC1CommandStatus();
    state.ac_2_command_status = _irModule->getAC2CommandStatus();
    state.projector_command_status = _irModule->getProjectorCommandStatus();
}
