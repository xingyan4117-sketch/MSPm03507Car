#include <stdint.h>
#include <stdio.h>

#include "encoder_ab.h"
#include "host_stub_support.h"
#include "speed_pid.h"
#include "tb6612_motor.h"

static int g_failures = 0;

static void ExpectInt32Equal(const char *label, int32_t actual, int32_t expected)
{
    if (actual != expected) {
        printf("FAIL: %s expected %ld but was %ld\n", label, (long) expected, (long) actual);
        g_failures++;
    }
}

static void ExpectUInt32Equal(const char *label, uint32_t actual, uint32_t expected)
{
    if (actual != expected) {
        printf("FAIL: %s expected %lu but was %lu\n", label, (unsigned long) expected,
            (unsigned long) actual);
        g_failures++;
    }
}

static void ExpectCondition(const char *label, int condition)
{
    if (!condition) {
        printf("FAIL: %s\n", label);
        g_failures++;
    }
}

static void TestTb6612MotorBehavior(void)
{
    const HostStubPwmState *pwmState;

    HostStub_ResetAll();
    Tb6612Motor_Init(600U);
    pwmState = HostStub_GetPwmState();
    ExpectUInt32Equal("motor init stops PWM once", pwmState->stopAllCalls, 1U);

    Tb6612Motor_SetDutyB(250);
    pwmState = HostStub_GetPwmState();
    ExpectUInt32Equal("motor B duty routed once", pwmState->setDutyBCalls, 1U);
    ExpectUInt32Equal("motor C untouched after B duty", pwmState->setDutyCCalls, 0U);
    ExpectInt32Equal("motor B positive duty preserved", pwmState->lastDutyB, 250);

    Tb6612Motor_SetDutyC(-125);
    pwmState = HostStub_GetPwmState();
    ExpectUInt32Equal("motor C duty routed once", pwmState->setDutyCCalls, 1U);
    ExpectInt32Equal("motor C negative duty preserved", pwmState->lastDutyC, -125);

    Tb6612Motor_SetDutyB(999);
    Tb6612Motor_SetDutyC(-999);
    pwmState = HostStub_GetPwmState();
    ExpectUInt32Equal("motor B duty call count after clamp", pwmState->setDutyBCalls, 2U);
    ExpectUInt32Equal("motor C duty call count after clamp", pwmState->setDutyCCalls, 2U);
    ExpectInt32Equal("motor B duty clamps to configured maximum", pwmState->lastDutyB, 600);
    ExpectInt32Equal("motor C duty clamps to configured negative maximum", pwmState->lastDutyC,
        -600);

    Tb6612Motor_StopCoast();
    pwmState = HostStub_GetPwmState();
    ExpectUInt32Equal("motor stop/coast routes to PWM stop", pwmState->stopAllCalls, 2U);
}

static void TestEncoderBehavior(void)
{
    EncoderAbSnapshot snapshot;
    EncoderAbRpmConfig rpmConfig;
    const HostStubEncoderState *encoderState;

    HostStub_ResetAll();

    HostStub_SetEncoderReadoutB(123, 7U);
    snapshot = EncoderAb_ReadB();
    encoderState = HostStub_GetEncoderState();
    ExpectUInt32Equal("encoder B read count", encoderState->readBCalls, 1U);
    ExpectUInt32Equal("encoder C read count stays zero after B read", encoderState->readCCalls,
        0U);
    ExpectInt32Equal("encoder B delta propagated", snapshot.deltaCounts, 123);
    ExpectUInt32Equal("encoder B invalid transitions propagated", snapshot.invalidTransitions, 7U);

    HostStub_SetEncoderReadoutC(-222, 4U);
    snapshot = EncoderAb_ReadC();
    encoderState = HostStub_GetEncoderState();
    ExpectUInt32Equal("encoder C read count", encoderState->readCCalls, 1U);
    ExpectInt32Equal("encoder C delta propagated", snapshot.deltaCounts, -222);
    ExpectUInt32Equal("encoder C invalid transitions propagated", snapshot.invalidTransitions, 4U);

    snapshot.deltaCounts = 20;
    snapshot.invalidTransitions = 0U;
    rpmConfig.countsPerRevolution = 20U;
    rpmConfig.samplePeriodMs = 100U;
    ExpectInt32Equal("encoder RPM positive conversion", EncoderAb_CountsToRpm(&snapshot, &rpmConfig),
        600);

    snapshot.deltaCounts = -10;
    ExpectInt32Equal("encoder RPM negative conversion", EncoderAb_CountsToRpm(&snapshot, &rpmConfig),
        -300);

    rpmConfig.countsPerRevolution = 0U;
    rpmConfig.samplePeriodMs = 100U;
    ExpectInt32Equal("encoder RPM with zero CPR returns zero",
        EncoderAb_CountsToRpm(&snapshot, &rpmConfig), 0);

    rpmConfig.countsPerRevolution = 20U;
    rpmConfig.samplePeriodMs = 0U;
    ExpectInt32Equal("encoder RPM with zero sample period returns zero",
        EncoderAb_CountsToRpm(&snapshot, &rpmConfig), 0);
}

static void TestSpeedPidBehavior(void)
{
    SpeedPid pid;
    SpeedPidConfig config;
    SpeedPidStatus status;

    config = SpeedPid_DefaultConfig();
    config.proportionalGain = 2;
    config.integralGain = 1;
    config.derivativeGain = 1;
    config.scale = 1;
    config.integralMin = -100;
    config.integralMax = 100;
    config.outputMin = -500;
    config.outputMax = 500;

    SpeedPid_Init(&pid, &config);
    status = SpeedPid_Update(&pid, 100, 90);
    ExpectInt32Equal("PID positive error", status.error, 10);
    ExpectInt32Equal("PID positive integral accumulation", status.integral, 10);
    ExpectInt32Equal("PID positive output", status.output, 40);

    status = SpeedPid_Update(&pid, 50, 80);
    ExpectInt32Equal("PID negative error", status.error, -30);
    ExpectInt32Equal("PID integral tracks signed error", status.integral, -20);
    ExpectInt32Equal("PID negative output", status.output, -120);

    config = SpeedPid_DefaultConfig();
    config.proportionalGain = 1000;
    config.integralGain = 1000;
    config.derivativeGain = 0;
    config.scale = 1;
    config.integralMin = -5;
    config.integralMax = 5;
    config.outputMin = -200;
    config.outputMax = 200;

    SpeedPid_Init(&pid, &config);
    status = SpeedPid_Update(&pid, 100, 0);
    ExpectInt32Equal("PID positive integral saturation", status.integral, 5);
    ExpectInt32Equal("PID positive output saturation", status.output, 200);

    status = SpeedPid_Update(&pid, -100, 0);
    ExpectInt32Equal("PID negative integral saturation", status.integral, -5);
    ExpectInt32Equal("PID negative output saturation", status.output, -200);

    SpeedPid_Reset(&pid);
    ExpectInt32Equal("PID reset clears integral", pid.integral, 0);
    ExpectInt32Equal("PID reset clears previous error", pid.previousError, 0);

    config = SpeedPid_DefaultConfig();
    config.proportionalGain = 10;
    config.integralGain = 0;
    config.derivativeGain = 0;
    config.scale = 0;
    config.outputMin = -1000;
    config.outputMax = 1000;

    SpeedPid_Init(&pid, &config);
    ExpectInt32Equal("PID normalizes non-positive scale to one", pid.config.scale, 1);
    status = SpeedPid_Update(&pid, 5, 0);
    ExpectInt32Equal("PID update uses normalized scale", status.output, 50);
    ExpectCondition("PID default output bounds remain signed", SPEED_PID_DEFAULT_OUTPUT_MIN < 0);
}

int main(void)
{
    TestTb6612MotorBehavior();
    TestEncoderBehavior();
    TestSpeedPidBehavior();

    if (g_failures != 0) {
        printf("FAIL: %d host-side behavior checks failed.\n", g_failures);
        return 1;
    }

    printf("PASS: host-side module behavior checks passed.\n");
    return 0;
}
