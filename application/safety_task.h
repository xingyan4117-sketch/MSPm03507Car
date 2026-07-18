#ifndef SAFETY_TASK_H
#define SAFETY_TASK_H

#include <stdbool.h>
#include <stdint.h>

#include "app_types.h"
#include "safety.h"

#define SAFETY_TASK_PERIOD_MS (10U)

bool SafetyTask_Init(void);
void SafetyTask_ProcessCycle(SafetyController *controller,
    const AppMotorCommand *command, const AppMotorStatus *motorStatus,
    bool keyPressed, bool externalFault, bool clearRequest, uint32_t nowMs);

#endif
