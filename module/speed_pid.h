#ifndef SPEED_PID_H
#define SPEED_PID_H

#include <stdint.h>

#define SPEED_PID_DEFAULT_OUTPUT_MIN (-800)
#define SPEED_PID_DEFAULT_OUTPUT_MAX (800)

typedef struct {
    int32_t proportionalGain;
    int32_t integralGain;
    int32_t derivativeGain;
    int32_t scale;
    int32_t integralMin;
    int32_t integralMax;
    int32_t outputMin;
    int32_t outputMax;
} SpeedPidConfig;

typedef struct {
    SpeedPidConfig config;
    int32_t integral;
    int32_t previousError;
} SpeedPid;

typedef struct {
    int32_t setpoint;
    int32_t measurement;
    int32_t error;
    int32_t integral;
    int32_t output;
} SpeedPidStatus;

SpeedPidConfig SpeedPid_DefaultConfig(void);
void SpeedPid_Init(SpeedPid *pid, const SpeedPidConfig *config);
void SpeedPid_Reset(SpeedPid *pid);
SpeedPidStatus SpeedPid_Update(SpeedPid *pid, int32_t setpoint, int32_t measurement);

#endif
