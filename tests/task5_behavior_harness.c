#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "app_types.h"
#include "safety.h"
#include "safety_task.h"
#include "st7735_ui.h"
#include "task.h"
#include "task5_host_support.h"

static AppMotorStatus g_publishedStatus;
static AppMotorStatus g_publishedSafetyStatus;
static uint32_t g_stopCalls;
static uint32_t g_eventOrder;
static uint32_t g_stopEventOrder;
static uint32_t g_publishEventOrder;
static uint32_t g_safetyPublishEventOrder;
static bool g_backlightEnabled;
static uint32_t g_spiWriteCalls;
static uint32_t g_failWriteCall;

static AppMotorCommand Task5Host_MakeCommand(AppState state, uint32_t sequence,
    uint32_t issuedAtMs, int32_t targetRpmB, int32_t targetRpmC)
{
    AppMotorCommand command = {
        .state = state,
        .clearRequest = 0U,
        .externalFault = 0U,
        .reserved = 0U,
        .sequence = sequence,
        .issuedAtMs = issuedAtMs,
        .targetRpmB = targetRpmB,
        .targetRpmC = targetRpmC
    };

    return command;
}

static AppMotorStatus Task5Host_MakeStatus(AppState state, uint32_t sequence,
    uint32_t observedAtMs, int32_t rpmB, int32_t rpmC, int32_t dutyB,
    int32_t dutyC, uint32_t invalidTransitionsB, uint32_t invalidTransitionsC)
{
    AppMotorStatus status = {
        .state = state,
        .commandTimedOut = 0U,
        .reserved = 0U,
        .commandSequence = sequence,
        .observedAtMs = observedAtMs,
        .rpmB = rpmB,
        .rpmC = rpmC,
        .dutyB = dutyB,
        .dutyC = dutyC,
        .invalidTransitionsB = invalidTransitionsB,
        .invalidTransitionsC = invalidTransitionsC
    };

    return status;
}

static void Task5Host_SetFailWriteCall(uint32_t writeCall)
{
    g_failWriteCall = writeCall;
}

static bool Task5Host_BacklightEnabled(void)
{
    return g_backlightEnabled;
}

static void Task5Host_AssertUnsafePublishOrder(void)
{
    assert(Task5Host_GetStopCalls() > 0U);
    assert(Task5Host_StopEventOrder() < Task5Host_PublishEventOrder());
}

static void Task5Host_AssertUnsafeSafetyPublishOrder(void)
{
    assert(Task5Host_GetStopCalls() > 0U);
    assert(Task5Host_StopEventOrder() < Task5Host_SafetyPublishEventOrder());
}

void Task5Host_Reset(void)
{
    (void) memset(&g_publishedStatus, 0, sizeof(g_publishedStatus));
    (void) memset(&g_publishedSafetyStatus, 0, sizeof(g_publishedSafetyStatus));
    g_stopCalls = 0U;
    g_eventOrder = 0U;
    g_stopEventOrder = 0U;
    g_publishEventOrder = 0U;
    g_safetyPublishEventOrder = 0U;
    g_backlightEnabled = false;
    g_spiWriteCalls = 0U;
    g_failWriteCall = 0U;
}

const AppMotorStatus *Task5Host_GetPublishedStatus(void)
{
    return &g_publishedStatus;
}

const AppMotorStatus *Task5Host_GetPublishedSafetyStatus(void)
{
    return &g_publishedSafetyStatus;
}

uint32_t Task5Host_GetStopCalls(void) { return g_stopCalls; }
uint32_t Task5Host_StopEventOrder(void) { return g_stopEventOrder; }
uint32_t Task5Host_PublishEventOrder(void) { return g_publishEventOrder; }
uint32_t Task5Host_SafetyPublishEventOrder(void) { return g_safetyPublishEventOrder; }

AppMotorCommand App_DefaultCommand(void)
{
    AppMotorCommand command = {
        .state = APP_STATE_READY,
        .clearRequest = 0U,
        .externalFault = 0U,
        .reserved = 0U,
        .sequence = 0U,
        .issuedAtMs = 0U,
        .targetRpmB = 0,
        .targetRpmC = 0
    };
    return command;
}

bool App_PublishCommand(const AppMotorCommand *command) { return command != NULL; }
bool App_ReadLatestCommand(AppMotorCommand *command) { return command != NULL; }
bool App_ReadLatestSafetyCommand(AppMotorCommand *command) { return command != NULL; }
bool App_PublishMotorStatus(const AppMotorStatus *status) { return status != NULL; }
bool App_ReadLatestMotorStatus(AppMotorStatus *status) { return status != NULL; }
bool App_ReadLatestStatus(AppMotorStatus *status) { return status != NULL; }

bool App_PublishSafetyStatus(const AppMotorStatus *status)
{
    g_publishedSafetyStatus = *status;
    g_safetyPublishEventOrder = ++g_eventOrder;
    return true;
}

bool App_ReadLatestSafetyStatus(AppMotorStatus *status) { return status != NULL; }

bool App_PublishStatus(const AppMotorStatus *status)
{
    g_publishedStatus = *status;
    g_publishEventOrder = ++g_eventOrder;
    return true;
}

void Tb6612Motor_StopCoast(void)
{
    g_stopCalls++;
    g_stopEventOrder = ++g_eventOrder;
}

bool BspGpio_ReadKeyDebounced(void) { return false; }
void BspSpi_Init(void) {}
void BspSpi_SetBacklight(bool enabled)
{
    g_backlightEnabled = enabled;
}
void BspSpi_SetReset(bool asserted) { (void) asserted; }
void BspSpi_DelayMs(uint32_t milliseconds) { (void) milliseconds; }
bool BspSpi_Write(bool dataMode, const uint8_t *data, size_t length)
{
    (void) dataMode;
    g_spiWriteCalls++;
    if ((g_failWriteCall != 0U) && (g_spiWriteCalls == g_failWriteCall)) {
        return false;
    }
    return (data != NULL) || (length == 0U);
}

TaskHandle_t xTaskCreateStatic(void (*task)(void *), const char *name,
    uint32_t stackDepth, void *argument, uint32_t priority,
    StackType_t *stack, StaticTask_t *taskStorage)
{
    (void) task;
    (void) name;
    (void) stackDepth;
    (void) argument;
    (void) priority;
    (void) stack;
    return taskStorage;
}

TickType_t xTaskGetTickCount(void) { return 0U; }
void vTaskDelayUntil(TickType_t *lastWakeTime, TickType_t increment)
{
    (void) lastWakeTime;
    (void) increment;
}

int main(void)
{
    AppMotorCommand command = Task5Host_MakeCommand(APP_STATE_RUN, 9U, 10U, 40, -40);
    AppMotorStatus motor = Task5Host_MakeStatus(APP_STATE_RUN, 9U, 10U,
        120, -100, 20, -20, 0U, 0U);
    AppMotorStatus faultedMotor = Task5Host_MakeStatus(APP_STATE_RUN, 9U, 10U,
        120, -100, 20, -20, 1U, 0U);
    SafetyController safety = Safety_ControllerInit();
    SafetyController timeoutSafety = Safety_ControllerInit();
    SafetyController faultSafety = Safety_ControllerInit();
    SafetyController externalFaultSafety = Safety_ControllerInit();
    const char *frame;

    Task5Host_Reset();
    SafetyTask_ProcessCycle(&safety, &command, &motor, false, false, false, 20U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_READY);
    assert(Task5Host_GetStopCalls() == 1U);
    SafetyTask_ProcessCycle(&safety, &command, &motor, false, false, false, 25U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_RUN);
    assert(Task5Host_GetStopCalls() == 1U);

    SafetyTask_ProcessCycle(&safety, &command, &motor, true, false, false, 30U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_ESTOP);
    assert(Task5Host_GetPublishedSafetyStatus()->state == APP_STATE_ESTOP);
    assert(Task5Host_GetStopCalls() == 2U);
    Task5Host_AssertUnsafePublishOrder();
    Task5Host_AssertUnsafeSafetyPublishOrder();

    command.targetRpmB = 0;
    command.targetRpmC = 0;
    SafetyTask_ProcessCycle(&safety, &command, &motor, false, false, false, 40U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_ESTOP);
    command.clearRequest = 1U;
    SafetyTask_ProcessCycle(&safety, &command, &motor, false, false, true, 50U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_READY);

    Task5Host_Reset();
    SafetyTask_ProcessCycle(&timeoutSafety, &command, &motor, false, false, false, 20U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_READY);
    command.clearRequest = 0U;
    command.targetRpmB = 40;
    command.targetRpmC = -40;
    SafetyTask_ProcessCycle(&timeoutSafety, &command, &motor, false, false, false, 25U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_RUN);
    SafetyTask_ProcessCycle(&timeoutSafety, &command, &motor, false, false, false, 111U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_READY);
    assert(Task5Host_GetPublishedStatus()->commandTimedOut == 1U);
    Task5Host_AssertUnsafePublishOrder();

    Task5Host_Reset();
    SafetyTask_ProcessCycle(&faultSafety, &command, &motor, false, false, false, 20U);
    SafetyTask_ProcessCycle(&faultSafety, &command, &motor, false, false, false, 25U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_RUN);
    SafetyTask_ProcessCycle(&faultSafety, &command, &faultedMotor, false, false, false, 30U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_FAULT);
    assert(Task5Host_GetPublishedSafetyStatus()->state == APP_STATE_FAULT);
    Task5Host_AssertUnsafePublishOrder();
    Task5Host_AssertUnsafeSafetyPublishOrder();
    command.targetRpmB = 40;
    command.targetRpmC = -40;
    SafetyTask_ProcessCycle(&faultSafety, &command, &motor, false, false, false, 40U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_FAULT);
    command.clearRequest = 1U;
    SafetyTask_ProcessCycle(&faultSafety, &command, &motor, false, false, true, 50U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_FAULT);
    command.targetRpmB = 0;
    command.targetRpmC = 0;
    SafetyTask_ProcessCycle(&faultSafety, &command, &motor, false, false, true, 60U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_READY);

    Task5Host_Reset();
    command = Task5Host_MakeCommand(APP_STATE_RUN, 10U, 10U, 0, 0);
    command.externalFault = 1U;
    SafetyTask_ProcessCycle(&externalFaultSafety, &command, &motor, false,
        command.externalFault != 0U, command.clearRequest != 0U, 20U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_FAULT);
    Task5Host_AssertUnsafePublishOrder();
    command.externalFault = 0U;
    command.clearRequest = 1U;
    SafetyTask_ProcessCycle(&externalFaultSafety, &command, &motor, false,
        command.externalFault != 0U, command.clearRequest != 0U, 30U);
    assert(Task5Host_GetPublishedStatus()->state == APP_STATE_READY);

    Task5Host_Reset();
    Task5Host_SetFailWriteCall(1U);
    St7735Ui_Init();
    assert(!Task5Host_BacklightEnabled());

    Task5Host_Reset();
    St7735Ui_Init();
    assert(Task5Host_BacklightEnabled());
    St7735Ui_RenderStatus(&motor);
    frame = St7735Ui_GetLastFrame();
    assert(frame != NULL);
    assert(strstr(frame, "B:20/120") != NULL);
    assert(strstr(frame, "C:-20/-100") != NULL);
    assert(strlen(frame) < ST7735_UI_FRAME_BYTES);

    puts("PASS: Task 5 host safety/UI behavior.");
    return 0;
}
