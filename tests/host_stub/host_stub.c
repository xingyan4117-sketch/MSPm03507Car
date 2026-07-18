#include "host_stub_support.h"

static HostStubPwmState g_pwmState;
static HostStubEncoderState g_encoderState;

void HostStub_ResetAll(void)
{
    g_pwmState.lastDutyA = 0;
    g_pwmState.lastDutyB = 0;
    g_pwmState.lastDutyC = 0;
    g_pwmState.lastDutyD = 0;
    g_pwmState.initCalls = 0;
    g_pwmState.setDutyACalls = 0;
    g_pwmState.setDutyBCalls = 0;
    g_pwmState.setDutyCCalls = 0;
    g_pwmState.setDutyDCalls = 0;
    g_pwmState.stopAllCalls = 0;

    g_encoderState.nextReadB.delta = 0;
    g_encoderState.nextReadB.invalidTransitions = 0U;
    g_encoderState.nextReadC.delta = 0;
    g_encoderState.nextReadC.invalidTransitions = 0U;
    g_encoderState.nextReadD.delta = 0;
    g_encoderState.nextReadD.invalidTransitions = 0U;
    g_encoderState.initCalls = 0;
    g_encoderState.readACalls = 0;
    g_encoderState.readBCalls = 0;
    g_encoderState.readCCalls = 0;
    g_encoderState.readDCalls = 0;
    g_encoderState.irqCalls = 0;
}

void HostStub_SetEncoderReadoutA(int32_t delta, uint32_t invalidTransitions)
{
    g_encoderState.nextReadA.delta = delta;
    g_encoderState.nextReadA.invalidTransitions = invalidTransitions;
}

void HostStub_SetEncoderReadoutB(int32_t delta, uint32_t invalidTransitions)
{
    g_encoderState.nextReadB.delta = delta;
    g_encoderState.nextReadB.invalidTransitions = invalidTransitions;
}

void HostStub_SetEncoderReadoutC(int32_t delta, uint32_t invalidTransitions)
{
    g_encoderState.nextReadC.delta = delta;
    g_encoderState.nextReadC.invalidTransitions = invalidTransitions;
}

void HostStub_SetEncoderReadoutD(int32_t delta, uint32_t invalidTransitions)
{
    g_encoderState.nextReadD.delta = delta;
    g_encoderState.nextReadD.invalidTransitions = invalidTransitions;
}

const HostStubPwmState *HostStub_GetPwmState(void)
{
    return &g_pwmState;
}

const HostStubEncoderState *HostStub_GetEncoderState(void)
{
    return &g_encoderState;
}

void BspPwm_Init(void)
{
    g_pwmState.initCalls++;
}

void BspPwm_SetDutyA(int32_t duty)
{
    g_pwmState.lastDutyA = duty;
    g_pwmState.setDutyACalls++;
}

void BspPwm_SetDutyB(int32_t duty)
{
    g_pwmState.lastDutyB = duty;
    g_pwmState.setDutyBCalls++;
}

void BspPwm_SetDutyC(int32_t duty)
{
    g_pwmState.lastDutyC = duty;
    g_pwmState.setDutyCCalls++;
}

void BspPwm_SetDutyD(int32_t duty)
{
    g_pwmState.lastDutyD = duty;
    g_pwmState.setDutyDCalls++;
}

void BspPwm_StopAll(void)
{
    g_pwmState.stopAllCalls++;
}

void BspEncoder_Init(void)
{
    g_encoderState.initCalls++;
}

BspEncoderReadout BspEncoder_ReadAndClearB(void)
{
    BspEncoderReadout readout = g_encoderState.nextReadB;

    g_encoderState.nextReadB.delta = 0;
    g_encoderState.nextReadB.invalidTransitions = 0U;
    g_encoderState.readBCalls++;
    return readout;
}

BspEncoderReadout BspEncoder_ReadAndClearA(void)
{
    BspEncoderReadout readout = g_encoderState.nextReadA;
    g_encoderState.nextReadA.delta = 0;
    g_encoderState.nextReadA.invalidTransitions = 0U;
    g_encoderState.readACalls++;
    return readout;
}

BspEncoderReadout BspEncoder_ReadAndClearC(void)
{
    BspEncoderReadout readout = g_encoderState.nextReadC;

    g_encoderState.nextReadC.delta = 0;
    g_encoderState.nextReadC.invalidTransitions = 0U;
    g_encoderState.readCCalls++;
    return readout;
}

BspEncoderReadout BspEncoder_ReadAndClearD(void)
{
    BspEncoderReadout readout = g_encoderState.nextReadD;
    g_encoderState.nextReadD.delta = 0;
    g_encoderState.nextReadD.invalidTransitions = 0U;
    g_encoderState.readDCalls++;
    return readout;
}

void BspEncoder_GPIOA_IRQHandler(void)
{
    g_encoderState.irqCalls++;
}
