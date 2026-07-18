#ifndef HOST_FREERTOS_H
#define HOST_FREERTOS_H

#include <stdint.h>

typedef int BaseType_t;
typedef uint32_t TickType_t;
typedef uint32_t StackType_t;
typedef struct { uint32_t unused; } StaticTask_t;
typedef void *TaskHandle_t;

#define pdPASS ((BaseType_t)1)
#define pdFAIL ((BaseType_t)0)
#define pdMS_TO_TICKS(milliseconds) ((TickType_t)(milliseconds))
#define portTICK_PERIOD_MS (1U)

#endif
