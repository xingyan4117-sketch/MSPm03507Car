#include <setjmp.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "app_init.h"
#include "encoder_ab.h"
#include "tb6612_motor.h"

#define APPLICATION_HOST_MAX_READS (4U)

typedef void (*ApplicationHostTaskEntry)(void *);

static ApplicationHostTaskEntry g_motorControlTaskEntry;
static jmp_buf g_taskLoopExit;
static bool g_taskLoopActive;
static uint32_t g_taskLoopTargetCycles;
static uint32_t g_taskLoopCompletedCycles;
static TickType_t g_tickCount;
static AppMotorCommand g_scheduledCommands[APPLICATION_HOST_MAX_READS];
static bool g_commandAvailable[APPLICATION_HOST_MAX_READS];
static uint32_t g_commandReadCalls;
static AppMotorStatus g_scheduledSafety[APPLICATION_HOST_MAX_READS];
static bool g_safetyAvailable[APPLICATION_HOST_MAX_READS];
static uint32_t g_safetyReadCalls;
static AppMotorStatus g_publishedMotorStatus;
static uint32_t g_stopCoastCalls;
static uint32_t g_setDutyBCalls;
static uint32_t g_setDutyCCalls;
static uint32_t g_setDutyACalls;
static uint32_t g_setDutyDCalls;
static int32_t g_lastDutyA;
static int32_t g_lastDutyD;
static int32_t g_lastDutyB;
static int32_t g_lastDutyC;

void ApplicationHost_Reset(void)
{
    (void) memset(g_scheduledCommands, 0, sizeof(g_scheduledCommands));
    (void) memset(g_commandAvailable, 0, sizeof(g_commandAvailable));
    (void) memset(g_scheduledSafety, 0, sizeof(g_scheduledSafety));
    (void) memset(g_safetyAvailable, 0, sizeof(g_safetyAvailable));
    (void) memset(&g_publishedMotorStatus, 0, sizeof(g_publishedMotorStatus));
    g_motorControlTaskEntry = NULL;
    g_taskLoopActive = false;
    g_taskLoopTargetCycles = 0U;
    g_taskLoopCompletedCycles = 0U;
    g_tickCount = 0U;
    g_commandReadCalls = 0U;
    g_safetyReadCalls = 0U;
    g_stopCoastCalls = 0U;
    g_setDutyBCalls = 0U;
    g_setDutyCCalls = 0U;
    g_setDutyACalls = 0U;
    g_setDutyDCalls = 0U;
    g_lastDutyA = 0;
    g_lastDutyD = 0;
    g_lastDutyB = 0;
    g_lastDutyC = 0;
}

void ApplicationHost_SetTickCount(uint32_t tickCount)
{
    g_tickCount = (TickType_t) tickCount;
}

void ApplicationHost_ScheduleCommandRead(uint32_t readIndex,
    const AppMotorCommand *command, bool available)
{
    if (readIndex >= APPLICATION_HOST_MAX_READS) {
        return;
    }

    g_commandAvailable[readIndex] = available;
    if (command != NULL) {
        g_scheduledCommands[readIndex] = *command;
    } else {
        (void) memset(&g_scheduledCommands[readIndex], 0,
            sizeof(g_scheduledCommands[readIndex]));
    }
}

void ApplicationHost_ScheduleSafetyRead(uint32_t readIndex,
    const AppMotorStatus *status, bool available)
{
    if (readIndex >= APPLICATION_HOST_MAX_READS) {
        return;
    }

    g_safetyAvailable[readIndex] = available;
    if (status != NULL) {
        g_scheduledSafety[readIndex] = *status;
    } else {
        (void) memset(&g_scheduledSafety[readIndex], 0,
            sizeof(g_scheduledSafety[readIndex]));
    }
}

bool ApplicationHost_RunMotorControlTaskCycles(uint32_t cycles)
{
    if ((g_motorControlTaskEntry == NULL) || (cycles == 0U)) {
        return false;
    }

    g_taskLoopTargetCycles = cycles;
    g_taskLoopCompletedCycles = 0U;
    g_taskLoopActive = true;
    if (setjmp(g_taskLoopExit) == 0) {
        g_motorControlTaskEntry(NULL);
        g_taskLoopActive = false;
        return false;
    }

    g_taskLoopActive = false;
    return true;
}

const AppMotorStatus *ApplicationHost_GetPublishedMotorStatus(void)
{
    return &g_publishedMotorStatus;
}

uint32_t ApplicationHost_GetSafetyReadCalls(void)
{
    return g_safetyReadCalls;
}

uint32_t ApplicationHost_GetStopCoastCalls(void)
{
    return g_stopCoastCalls;
}

uint32_t ApplicationHost_GetSetDutyBCalls(void)
{
    return g_setDutyBCalls;
}

uint32_t ApplicationHost_GetSetDutyCCalls(void)
{
    return g_setDutyCCalls;
}

uint32_t ApplicationHost_GetSetDutyACalls(void) { return g_setDutyACalls; }
uint32_t ApplicationHost_GetSetDutyDCalls(void) { return g_setDutyDCalls; }

int32_t ApplicationHost_GetLastDutyB(void)
{
    return g_lastDutyB;
}

int32_t ApplicationHost_GetLastDutyC(void)
{
    return g_lastDutyC;
}

int32_t ApplicationHost_GetLastDutyA(void) { return g_lastDutyA; }
int32_t ApplicationHost_GetLastDutyD(void) { return g_lastDutyD; }

BaseType_t xQueueReceive(QueueHandle_t queue, void *item, TickType_t wait)
{
    (void) queue;
    (void) item;
    (void) wait;
    return pdFAIL;
}

TaskHandle_t xTaskCreateStatic(void (*task)(void *), const char *name,
    uint32_t stackDepth, void *argument, uint32_t priority,
    StackType_t *stack, StaticTask_t *taskStorage)
{
    (void) name;
    (void) stackDepth;
    (void) argument;
    (void) priority;
    (void) stack;
    g_motorControlTaskEntry = task;
    return taskStorage;
}

TickType_t xTaskGetTickCount(void)
{
    return g_tickCount;
}

void vTaskDelayUntil(TickType_t *lastWakeTime, TickType_t increment)
{
    if (lastWakeTime != NULL) {
        *lastWakeTime += increment;
        g_tickCount = *lastWakeTime;
    } else {
        g_tickCount += increment;
    }

    if (g_taskLoopActive) {
        g_taskLoopCompletedCycles++;
        if (g_taskLoopCompletedCycles >= g_taskLoopTargetCycles) {
            longjmp(g_taskLoopExit, 1);
        }
    }
}

AppMotorCommand App_DefaultCommand(void)
{
    AppMotorCommand command = {
        .state = APP_STATE_READY,
        .clearRequest = 0U,
        .externalFault = 0U,
        .reserved = 0U,
        .sequence = 0U,
        .issuedAtMs = 0U,
        .targetRpmA = 0,
        .targetRpmB = 0,
        .targetRpmC = 0,
        .targetRpmD = 0
    };

    return command;
}

bool App_ReadLatestCommand(AppMotorCommand *command)
{
    bool available = false;

    if (g_commandReadCalls < APPLICATION_HOST_MAX_READS) {
        available = g_commandAvailable[g_commandReadCalls];
        if (available && (command != NULL)) {
            *command = g_scheduledCommands[g_commandReadCalls];
        }
    }
    g_commandReadCalls++;
    return available && (command != NULL);
}

bool App_PublishMotorStatus(const AppMotorStatus *status)
{
    if (status == NULL) {
        return false;
    }

    g_publishedMotorStatus = *status;
    return true;
}

bool App_ReadLatestSafetyStatus(AppMotorStatus *status)
{
    bool available = false;

    if (g_safetyReadCalls < APPLICATION_HOST_MAX_READS) {
        available = g_safetyAvailable[g_safetyReadCalls];
        if (available && (status != NULL)) {
            *status = g_scheduledSafety[g_safetyReadCalls];
        }
    }
    g_safetyReadCalls++;
    return available && (status != NULL);
}

bool App_PublishStatus(const AppMotorStatus *status)
{
    (void) status;
    return true;
}

void Tb6612Motor_Init(uint16_t maximumDuty)
{
    (void) maximumDuty;
}

void Tb6612Motor_SetDutyB(int32_t signedDuty)
{
    g_setDutyBCalls++;
    g_lastDutyB = signedDuty;
}

void Tb6612Motor_SetDutyA(int32_t signedDuty)
{
    g_setDutyACalls++;
    g_lastDutyA = signedDuty;
}

void Tb6612Motor_SetDutyC(int32_t signedDuty)
{
    g_setDutyCCalls++;
    g_lastDutyC = signedDuty;
}

void Tb6612Motor_SetDutyD(int32_t signedDuty)
{
    g_setDutyDCalls++;
    g_lastDutyD = signedDuty;
}

void Tb6612Motor_StopCoast(void)
{
    g_stopCoastCalls++;
}

EncoderAbSnapshot EncoderAb_ReadB(void)
{
    EncoderAbSnapshot snapshot = { 0, 0U };

    return snapshot;
}

EncoderAbSnapshot EncoderAb_ReadA(void)
{
    EncoderAbSnapshot snapshot = { 0, 0U };
    return snapshot;
}

EncoderAbSnapshot EncoderAb_ReadC(void)
{
    EncoderAbSnapshot snapshot = { 0, 0U };

    return snapshot;
}

EncoderAbSnapshot EncoderAb_ReadD(void)
{
    EncoderAbSnapshot snapshot = { 0, 0U };
    return snapshot;
}

int32_t EncoderAb_CountsToRpm(const EncoderAbSnapshot *snapshot,
    const EncoderAbRpmConfig *config)
{
    (void) snapshot;
    (void) config;
    return 0;
}
