#include "bsp_freertos.h"

#include "bsp_log.h"

#include "FreeRTOS.h"
#include "queue.h"

static StaticQueue_t g_logQueueStorage;
static uint8_t g_logQueueBuffer[BSP_LOG_QUEUE_DEPTH * sizeof(BspLogMessage)];
static QueueHandle_t g_logQueue;

bool BspFreeRTOS_Init(void)
{
    g_logQueue = xQueueCreateStatic(BSP_LOG_QUEUE_DEPTH, sizeof(BspLogMessage),
        g_logQueueBuffer, &g_logQueueStorage);

    return g_logQueue != NULL;
}

BspFreeRTOSLogQueueHandle BspFreeRTOS_GetLogQueue(void)
{
    return g_logQueue;
}
