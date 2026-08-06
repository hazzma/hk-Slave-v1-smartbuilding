#ifndef MODULES_IR_COMBO_MODULE_H
#define MODULES_IR_COMBO_MODULE_H

#include "ModuleInterface.h"
#include "../drivers/IRDriver.h"
#include "../core/ModuleStatus.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

enum IrTarget {
    IR_TARGET_AC1 = 1,
    IR_TARGET_AC2 = 2,
    IR_TARGET_PROJECTOR = 3
};

struct IrCommand {
    IrTarget target;
    bool power;
    uint16_t value; // temp for AC, input mode for projector
    uint8_t mode;   // mode for AC
    uint8_t fanSpeed;
    uint8_t swingVertical;
    uint8_t swingHorizontal;
};

class IRComboModule : public ModuleInterface {
private:
    IRDriver* _driver;
    bool _enabled;
    ModuleStatus _status;
    uint16_t _lastError;

    // Command State Machine
    bool _commandPending;
    bool _busy;
    uint32_t _commandStartMs;
    uint32_t _cooldownStartMs;
    IrCommand _activeCmd;
    bool _projectorSequenceActive;
    uint8_t _projectorSequenceStep;

    // Dynamic assignments
    bool _ac1Enabled;
    bool _ac2Enabled;
    bool _projEnabled;

    // Command status mirrors (0 = idle, 1 = success, 2 = busy, 3 = failed)
    uint16_t _ac1CmdStatus;
    uint16_t _ac2CmdStatus;
    uint16_t _projCmdStatus;

    // Staged command buffer for debouncing multi-register writes
    IrCommand _stagedCmd[2];
    bool _stagedPending[2];
    uint32_t _settlingStartMs[2];

    // FreeRTOS IR task – offloads blocking sendPanasonicDke() so the Modbus
    // main loop can continue responding during the ~150-200 ms IR transmission.
    TaskHandle_t _irTaskHandle;
    SemaphoreHandle_t _irTriggerSem;    // main loop gives → IR task wakes
    volatile bool _irTaskRunning;       // true while IR task is executing
    volatile bool _irTaskResultReady;   // task finished, result is valid
    volatile bool _irTaskSuccess;       // result: true = IR sent OK

    /**
     * @brief FreeRTOS task body for AC IR transmission.
     *        Reads _activeCmd, calls driver, writes result flags.
     */
    static void irTaskFunction(void* pvParam);

    void processProjectorSequence(uint32_t now_ms);

public:
    IRComboModule();
    ~IRComboModule();

    bool begin() override;
    void update(uint32_t now_ms) override;

    bool isEnabled() const override { return _enabled; }
    void setEnabled(bool enabled) override;
    bool isBusy() const override { return _busy; }
    bool hasValidData() const override { return _enabled; }
    uint16_t getStatus() const override;
    uint16_t getLastError() const override { return _lastError; }

    // Configuration methods
    void setAC1Enabled(bool enabled) { _ac1Enabled = enabled; }
    void setAC2Enabled(bool enabled) { _ac2Enabled = enabled; }
    void setProjectorEnabled(bool enabled) { _projEnabled = enabled; }

    /**
     * @brief Queue a command for an AC unit.
     */
    bool queueAcCommand(uint8_t channel, bool power, uint16_t tempX10, uint8_t mode,
                        uint8_t fanSpeed, uint8_t swingVertical, uint8_t swingHorizontal);
    
    /**
     * @brief Queue a command for the projector.
     */
    bool queueProjectorCommand(bool power, uint16_t inputVal);

    // Getters for status mirrors
    uint16_t getAC1CommandStatus() const { return _ac1CmdStatus; }
    uint16_t getAC2CommandStatus() const { return _ac2CmdStatus; }
    uint16_t getProjectorCommandStatus() const { return _projCmdStatus; }
};

#endif // MODULES_IR_COMBO_MODULE_H
