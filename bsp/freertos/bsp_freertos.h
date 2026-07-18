#ifndef BSP_FREERTOS_H
#define BSP_FREERTOS_H

#include <stdbool.h>

typedef void *BspFreeRTOSLogQueueHandle;

bool BspFreeRTOS_Init(void);
BspFreeRTOSLogQueueHandle BspFreeRTOS_GetLogQueue(void);

#endif
