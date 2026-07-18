#ifndef COMMAND_TASK_H
#define COMMAND_TASK_H

#include <stdbool.h>

#include "app_types.h"

#define COMMAND_TASK_PERIOD_MS (20U)
#define COMMAND_START_RPM (100)
#define COMMAND_MAX_RPM (600)
#define COMMAND_SPEED_STEP_RPM (10)

bool CommandTask_Init(void);
void CommandTask_ApplyButtonEvents(AppMotorCommand *command, bool speedUp,
    bool speedDown, bool toggleRun);
void CommandTask_ApplyMenuEvents(AppMotorCommand *command, bool key1,
    bool key2, bool key3);

#endif
