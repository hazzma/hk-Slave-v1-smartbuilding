#ifndef CORE_MODULE_STATUS_H
#define CORE_MODULE_STATUS_H

#include <Arduino.h>

enum ModuleStatus {
  MODULE_DISABLED = 0,
  MODULE_INIT = 1,
  MODULE_READY = 2,
  MODULE_BUSY = 3,
  MODULE_ERROR = 4,
  MODULE_NOT_ASSIGNED = 5,
  MODULE_NOT_IMPLEMENTED = 6
};

enum CommandStatus {
  CMD_IDLE = 0,
  CMD_SUCCESS = 1,
  CMD_BUSY = 2,
  CMD_FAILED = 3
};

#endif // CORE_MODULE_STATUS_H
