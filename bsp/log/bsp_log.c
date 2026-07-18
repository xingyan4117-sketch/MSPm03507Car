#include "bsp_log.h"

#include "bsp_freertos.h"

#include "FreeRTOS.h"
#include "queue.h"

bool BspLog_Write(const char *message)
{
    BspLogMessage entry = {{0}};
    QueueHandle_t queue = BspFreeRTOS_GetLogQueue();
    uint32_t index;

    if ((message == NULL) || (queue == NULL)) {
        return false;
    }
    for (index = 0U; (index < (BSP_LOG_MESSAGE_BYTES - 1U)) &&
            (message[index] != '\0'); index++) {
        entry.text[index] = message[index];
    }
    return xQueueSend(queue, &entry, 0) == pdPASS;
}
