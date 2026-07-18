#ifndef BSP_WS2812_H
#define BSP_WS2812_H

#include <stdint.h>

void BspWs2812_Init(void);
void BspWs2812_SetConfig(uint8_t count, uint8_t brightness,
    const uint8_t colors[3]);
void BspWs2812_Tick(uint32_t nowMs);
void BspWs2812_Off(void);

#endif
