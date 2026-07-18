#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#include <stdint.h>

typedef struct {
    int32_t delta;
    uint32_t invalidTransitions;
} BspEncoderReadout;

void BspEncoder_Init(void);
BspEncoderReadout BspEncoder_ReadAndClearA(void);
BspEncoderReadout BspEncoder_ReadAndClearB(void);
BspEncoderReadout BspEncoder_ReadAndClearC(void);
BspEncoderReadout BspEncoder_ReadAndClearD(void);
void BspEncoder_GPIOA_IRQHandler(void);

#endif
