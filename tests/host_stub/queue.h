#ifndef HOST_QUEUE_H
#define HOST_QUEUE_H

#include "FreeRTOS.h"

typedef struct { uint32_t unused; } StaticQueue_t;
typedef void *QueueHandle_t;

BaseType_t xQueueReceive(QueueHandle_t queue, void *item, TickType_t wait);

#endif
