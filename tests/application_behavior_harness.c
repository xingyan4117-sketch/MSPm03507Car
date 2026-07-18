#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_types.h"
#include "motor_control_task.h"
#include "speed_pid.h"

void ApplicationHost_Reset(void);
void ApplicationHost_SetTickCount(uint32_t tickCount);
void ApplicationHost_ScheduleCommandRead(uint32_t readIndex,
    const AppMotorCommand *command, bool available);
void ApplicationHost_ScheduleSafetyRead(uint32_t readIndex,
    const AppMotorStatus *status, bool available);
bool ApplicationHost_RunMotorControlTaskCycles(uint32_t cycles);
const AppMotorStatus *ApplicationHost_GetPublishedMotorStatus(void);
uint32_t ApplicationHost_GetSafetyReadCalls(void);
uint32_t ApplicationHost_GetStopCoastCalls(void);
uint32_t ApplicationHost_GetSetDutyBCalls(void);
uint32_t ApplicationHost_GetSetDutyCCalls(void);
uint32_t ApplicationHost_GetSetDutyACalls(void);
uint32_t ApplicationHost_GetSetDutyDCalls(void);
int32_t ApplicationHost_GetLastDutyB(void);
int32_t ApplicationHost_GetLastDutyC(void);
int32_t ApplicationHost_GetLastDutyA(void);
int32_t ApplicationHost_GetLastDutyD(void);

static AppMotorCommand ApplicationHarness_MakeRunCommand(void)
{
    AppMotorCommand command = {
        .state = APP_STATE_RUN,
        .clearRequest = 0U,
        .externalFault = 0U,
        .reserved = 0U,
        .sequence = 7U,
        .issuedAtMs = 100U,
        .targetRpmA = 100,
        .targetRpmB = 120,
        .targetRpmC = -80,
        .targetRpmD = 60
    };

    return command;
}

static AppMotorStatus ApplicationHarness_MakeSafetyStatus(AppState state)
{
    AppMotorStatus status = {
        .state = state,
        .commandTimedOut = 0U,
        .reserved = 0U,
        .commandSequence = 0U,
        .observedAtMs = 0U,
        .rpmA = 0,
        .rpmB = 0,
        .rpmC = 0,
        .rpmD = 0,
        .dutyA = 0,
        .dutyB = 0,
        .dutyC = 0,
        .dutyD = 0,
        .invalidTransitionsA = 0U,
        .invalidTransitionsB = 0U,
        .invalidTransitionsC = 0U,
        .invalidTransitionsD = 0U
    };

    return status;
}

static void ApplicationHarness_VerifyPureMotorControlInterlocks(void)
{
    AppMotorCommand command = ApplicationHarness_MakeRunCommand();
    AppEncoderSnapshot encoders = {
        .deltaCountsA = 1, .deltaCountsB = 2, .deltaCountsC = -3, .deltaCountsD = 4,
        .rpmA = 100, .rpmB = 110, .rpmC = -75, .rpmD = 60,
        .invalidTransitionsA = 0U, .invalidTransitionsB = 0U,
        .invalidTransitionsC = 1U, .invalidTransitionsD = 0U
    };
    AppMotorStatus safetyStatus = ApplicationHarness_MakeSafetyStatus(APP_STATE_RUN);
    AppMotorStatus status;
    SpeedPid pidB;
    SpeedPid pidC;
    SpeedPid pidA;
    SpeedPid pidD;

    SpeedPid_Init(&pidA, NULL);
    SpeedPid_Init(&pidB, NULL);
    SpeedPid_Init(&pidC, NULL);
    SpeedPid_Init(&pidD, NULL);
    assert(MotorControl_CommandIsUsable(&command, 150U));
    MotorControl_Process(&pidA, &pidB, &pidC, &pidD, &command, 150U, &encoders, &safetyStatus,
        &status);
    assert(status.commandSequence == 7U);
    assert(status.dutyA == 0);
    assert(status.dutyB == 10);
    assert(status.dutyC == -5);
    assert(status.dutyD == 0);
    assert(status.invalidTransitionsC == 1U);

    safetyStatus.state = APP_STATE_READY;
    MotorControl_Process(&pidA, &pidB, &pidC, &pidD, &command, 151U, &encoders, &safetyStatus,
        &status);
    assert(status.state == APP_STATE_READY);
    assert(status.dutyB == 0);
    assert(status.dutyC == 0);
    assert(pidB.integral == 0);
    assert(pidC.integral == 0);

    MotorControl_Process(&pidA, &pidB, &pidC, &pidD, &command, 152U, &encoders, NULL, &status);
    assert(status.state == APP_STATE_FAULT);
    assert(status.dutyB == 0);
    assert(status.dutyC == 0);

    command.state = APP_STATE_READY;
    assert(!MotorControl_CommandIsUsable(&command, 150U));
    MotorControl_Process(&pidA, &pidB, &pidC, &pidD, &command, 150U, &encoders, &safetyStatus,
        &status);
    assert(status.dutyB == 0);
    assert(status.dutyC == 0);

    command.state = APP_STATE_RUN;
    assert(!MotorControl_CommandIsUsable(&command, 201U));
    MotorControl_Process(&pidA, &pidB, &pidC, &pidD, &command, 201U, &encoders, &safetyStatus,
        &status);
    assert(status.commandTimedOut == 1U);
    assert(status.dutyB == 0);
    assert(status.dutyC == 0);

    command.issuedAtMs = 201U;
    command.state = APP_STATE_RUN;
    safetyStatus.state = APP_STATE_ESTOP;
    MotorControl_Process(&pidA, &pidB, &pidC, &pidD, &command, 202U, &encoders, &safetyStatus,
        &status);
    assert(status.state == APP_STATE_ESTOP);
    assert(status.dutyB == 0);
    assert(status.dutyC == 0);
    assert(pidB.integral == 0);
    assert(pidC.integral == 0);

    safetyStatus.state = APP_STATE_FAULT;
    MotorControl_Process(&pidA, &pidB, &pidC, &pidD, &command, 203U, &encoders, &safetyStatus,
        &status);
    assert(status.state == APP_STATE_FAULT);
    assert(status.dutyB == 0);
    assert(status.dutyC == 0);
    assert(pidB.previousError == 0);
    assert(pidC.previousError == 0);
}

static void ApplicationHarness_VerifyTaskCycleInterlock(AppState unsafeState)
{
    AppMotorCommand command = ApplicationHarness_MakeRunCommand();
    AppMotorStatus readySafety = ApplicationHarness_MakeSafetyStatus(APP_STATE_RUN);
    AppMotorStatus unsafeSafety = ApplicationHarness_MakeSafetyStatus(unsafeState);
    const AppMotorStatus *publishedStatus;

    ApplicationHost_Reset();
    ApplicationHost_SetTickCount(150U);
    ApplicationHost_ScheduleCommandRead(0U, &command, true);
    ApplicationHost_ScheduleCommandRead(1U, &command, false);
    ApplicationHost_ScheduleSafetyRead(0U, &readySafety, true);
    ApplicationHost_ScheduleSafetyRead(1U, &unsafeSafety, true);
    assert(MotorControl_Init());
    assert(ApplicationHost_RunMotorControlTaskCycles(2U));

    publishedStatus = ApplicationHost_GetPublishedMotorStatus();
    assert(publishedStatus != NULL);
    assert(publishedStatus->state == unsafeState);
    assert(publishedStatus->dutyA == 0);
    assert(publishedStatus->dutyB == 0);
    assert(publishedStatus->dutyC == 0);
    assert(publishedStatus->dutyD == 0);
    assert(ApplicationHost_GetSafetyReadCalls() == 2U);
    assert(ApplicationHost_GetStopCoastCalls() == 1U);
    assert(ApplicationHost_GetSetDutyACalls() == 1U);
    assert(ApplicationHost_GetSetDutyBCalls() == 1U);
    assert(ApplicationHost_GetSetDutyCCalls() == 1U);
    assert(ApplicationHost_GetSetDutyDCalls() == 1U);
    assert(ApplicationHost_GetLastDutyA() == 100);
    assert(ApplicationHost_GetLastDutyB() == 120);
    assert(ApplicationHost_GetLastDutyC() == -80);
    assert(ApplicationHost_GetLastDutyD() == 60);
}

int main(void)
{
    ApplicationHarness_VerifyPureMotorControlInterlocks();
    ApplicationHarness_VerifyTaskCycleInterlock(APP_STATE_ESTOP);
    ApplicationHarness_VerifyTaskCycleInterlock(APP_STATE_FAULT);

    puts("PASS: Application MotorControl host behavior.");
    return 0;
}
