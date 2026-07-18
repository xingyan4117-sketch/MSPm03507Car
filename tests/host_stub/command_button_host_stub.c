#include "FreeRTOS.h"
#include "app_init.h"
#include "bsp_gpio.h"
#include "task.h"

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

bool App_PublishCommand(const AppMotorCommand *command)
{
    return command != NULL;
}

bool BspGpio_ReadKey1Debounced(void) { return false; }
bool BspGpio_ReadKey2Debounced(void) { return false; }
bool BspGpio_ReadKey3Debounced(void) { return false; }
void BspBuzzer_SetConfig(uint8_t mode, uint8_t volume, uint32_t nowMs)
{
    (void) mode;
    (void) volume;
    (void) nowMs;
}
void BspBuzzer_Tick(uint32_t nowMs) { (void) nowMs; }
void BspWs2812_SetConfig(uint8_t count, uint8_t brightness,
    const uint8_t colors[3])
{
    (void) count;
    (void) brightness;
    (void) colors;
}
void BspWs2812_Tick(uint32_t nowMs) { (void) nowMs; }

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
