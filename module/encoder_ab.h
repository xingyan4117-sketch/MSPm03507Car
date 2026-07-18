#ifndef ENCODER_AB_H
#define ENCODER_AB_H

#include <stdint.h>

typedef struct {
    int32_t deltaCounts;
    uint32_t invalidTransitions;
} EncoderAbSnapshot;

typedef struct {
    uint32_t countsPerRevolution;
    uint32_t samplePeriodMs;
} EncoderAbRpmConfig;

EncoderAbSnapshot EncoderAb_ReadA(void);
EncoderAbSnapshot EncoderAb_ReadB(void);
EncoderAbSnapshot EncoderAb_ReadC(void);
EncoderAbSnapshot EncoderAb_ReadD(void);
int32_t EncoderAb_CountsToRpm(const EncoderAbSnapshot *snapshot,
    const EncoderAbRpmConfig *config);

#endif
