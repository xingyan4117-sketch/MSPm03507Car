#ifndef BSP_BUZZER_H
#define BSP_BUZZER_H

#include <stdbool.h>
#include <stdint.h>

void BspBuzzer_Init(void);
void BspBuzzer_SetConfig(uint8_t mode, uint8_t volume, uint32_t nowMs);
void BspBuzzer_Tick(uint32_t nowMs);
void BspBuzzer_Stop(void);

#endif
