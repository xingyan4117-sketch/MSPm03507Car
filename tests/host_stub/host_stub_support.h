#ifndef HOST_STUB_SUPPORT_H
#define HOST_STUB_SUPPORT_H

#include <stdint.h>

#include "bsp_encoder.h"

typedef struct {
    int32_t lastDutyA;
    int32_t lastDutyB;
    int32_t lastDutyC;
    int32_t lastDutyD;
    uint32_t initCalls;
    uint32_t setDutyACalls;
    uint32_t setDutyBCalls;
    uint32_t setDutyCCalls;
    uint32_t setDutyDCalls;
    uint32_t stopAllCalls;
} HostStubPwmState;

typedef struct {
    BspEncoderReadout nextReadA;
    BspEncoderReadout nextReadB;
    BspEncoderReadout nextReadC;
    BspEncoderReadout nextReadD;
    uint32_t initCalls;
    uint32_t readACalls;
    uint32_t readBCalls;
    uint32_t readCCalls;
    uint32_t readDCalls;
    uint32_t irqCalls;
} HostStubEncoderState;

void HostStub_ResetAll(void);
void HostStub_SetEncoderReadoutA(int32_t delta, uint32_t invalidTransitions);
void HostStub_SetEncoderReadoutB(int32_t delta, uint32_t invalidTransitions);
void HostStub_SetEncoderReadoutC(int32_t delta, uint32_t invalidTransitions);
void HostStub_SetEncoderReadoutD(int32_t delta, uint32_t invalidTransitions);
const HostStubPwmState *HostStub_GetPwmState(void);
const HostStubEncoderState *HostStub_GetEncoderState(void);

#endif
