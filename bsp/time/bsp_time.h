#ifndef BSP_TIME_H
#define BSP_TIME_H

#include <stdint.h>

/* FreeRTOS is configured for 32-bit 1 kHz ticks, so this wraps at 2^32 ms. */
uint32_t BspTime_GetMs(void);

#endif
