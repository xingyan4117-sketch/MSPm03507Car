#ifndef HOST_TASK_H
#define HOST_TASK_H

#include "FreeRTOS.h"

TaskHandle_t xTaskCreateStatic(void (*task)(void *), const char *name,
    uint32_t stackDepth, void *argument, uint32_t priority,
    StackType_t *stack, StaticTask_t *taskStorage);
TickType_t xTaskGetTickCount(void);
void vTaskDelayUntil(TickType_t *lastWakeTime, TickType_t increment);

#endif
