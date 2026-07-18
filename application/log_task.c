#include "log_task.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "bsp_freertos.h"
#include "bsp_log.h"

#define LOG_TASK_STACK_WORDS (128U)
#define LOG_TASK_PERIOD_MS (100U)

static StaticTask_t g_logTaskStorage;
static StackType_t g_logTaskStack[LOG_TASK_STACK_WORDS];

static void LogTask(void *argument)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    QueueHandle_t queue = (QueueHandle_t) BspFreeRTOS_GetLogQueue();
    BspLogMessage message;

    (void) argument;
    for (;;) {
        while ((queue != NULL) && (xQueueReceive(queue, &message, 0U) == pdPASS)) {
        }
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(LOG_TASK_PERIOD_MS));
    }
}

bool LogTask_Init(void)
{
    return xTaskCreateStatic(LogTask, "log", LOG_TASK_STACK_WORDS, NULL,
        LOG_TASK_PRIORITY, g_logTaskStack, &g_logTaskStorage) != NULL;
}
