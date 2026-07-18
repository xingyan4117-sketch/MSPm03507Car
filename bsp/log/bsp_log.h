#ifndef BSP_LOG_H
#define BSP_LOG_H

#include <stdbool.h>

#define BSP_LOG_MESSAGE_BYTES (64U)
#define BSP_LOG_QUEUE_DEPTH   (8U)

typedef struct {
    char text[BSP_LOG_MESSAGE_BYTES];
} BspLogMessage;

bool BspLog_Write(const char *message);

#endif
