#include "speed_pid.h"

#include <limits.h>
#include <stddef.h>

static int32_t SpeedPid_Clamp(int64_t value, int32_t minimum, int32_t maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return (int32_t) value;
}

static int32_t SpeedPid_SaturateInt32(int64_t value)
{
    return SpeedPid_Clamp(value, INT32_MIN, INT32_MAX);
}

static void SpeedPid_NormalizeBounds(int32_t *minimum, int32_t *maximum)
{
    int32_t value;

    if (*minimum > *maximum) {
        value = *minimum;
        *minimum = *maximum;
        *maximum = value;
    }
}

SpeedPidConfig SpeedPid_DefaultConfig(void)
{
    SpeedPidConfig config = {
        1, 0, 0, 1,
        INT32_MIN, INT32_MAX,
        SPEED_PID_DEFAULT_OUTPUT_MIN, SPEED_PID_DEFAULT_OUTPUT_MAX
    };

    return config;
}

void SpeedPid_Init(SpeedPid *pid, const SpeedPidConfig *config)
{
    if (pid == NULL) {
        return;
    }

    pid->config = config == NULL ? SpeedPid_DefaultConfig() : *config;
    if (pid->config.scale <= 0) {
        pid->config.scale = 1;
    }
    SpeedPid_NormalizeBounds(&pid->config.integralMin, &pid->config.integralMax);
    SpeedPid_NormalizeBounds(&pid->config.outputMin, &pid->config.outputMax);
    SpeedPid_Reset(pid);
}

void SpeedPid_Reset(SpeedPid *pid)
{
    if (pid == NULL) {
        return;
    }

    pid->integral = 0;
    pid->previousError = 0;
}

SpeedPidStatus SpeedPid_Update(SpeedPid *pid, int32_t setpoint, int32_t measurement)
{
    SpeedPidStatus status = { 0 };
    int64_t rawError;
    int64_t rawDerivative;
    int64_t rawOutput;

    if (pid == NULL) {
        return status;
    }

    rawError = (int64_t) setpoint - measurement;
    status.setpoint = setpoint;
    status.measurement = measurement;
    status.error = SpeedPid_SaturateInt32(rawError);
    pid->integral = SpeedPid_Clamp((int64_t) pid->integral + status.error,
        pid->config.integralMin, pid->config.integralMax);
    rawDerivative = (int64_t) status.error - pid->previousError;
    rawOutput = ((int64_t) pid->config.proportionalGain * status.error) +
        ((int64_t) pid->config.integralGain * pid->integral) +
        ((int64_t) pid->config.derivativeGain * rawDerivative);
    status.integral = pid->integral;
    status.output = SpeedPid_Clamp(rawOutput / pid->config.scale,
        pid->config.outputMin, pid->config.outputMax);
    pid->previousError = status.error;
    return status;
}
