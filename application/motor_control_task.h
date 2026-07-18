#ifndef MOTOR_CONTROL_TASK_H
#define MOTOR_CONTROL_TASK_H

#include <stdbool.h>
#include <stdint.h>

#include "app_types.h"
#include "speed_pid.h"

#define MOTOR_CONTROL_SAMPLE_PERIOD_MS (1U)
#define MOTOR_CONTROL_COMMAND_TIMEOUT_MS (100U)

bool MotorControl_Init(void);
bool MotorControl_CommandIsUsable(const AppMotorCommand *command, uint32_t nowMs);
void MotorControl_Process(SpeedPid *pidA, SpeedPid *pidB, SpeedPid *pidC,
    SpeedPid *pidD,
    const AppMotorCommand *command, uint32_t nowMs,
    const AppEncoderSnapshot *encoders, const AppMotorStatus *safetyStatus,
    AppMotorStatus *status);

#endif
