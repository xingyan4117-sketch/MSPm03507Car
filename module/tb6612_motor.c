#include "tb6612_motor.h"

#include "bsp_pwm.h"

typedef void (*Tb6612MotorApplyDuty)(int32_t signedDuty);

typedef struct {
    Tb6612MotorApplyDuty applyDuty;
    int32_t maximumDuty;
} Tb6612MotorInstance;

static Tb6612MotorInstance g_motorA = { BspPwm_SetDutyA, TB6612_MOTOR_DEFAULT_MAX_DUTY };
static Tb6612MotorInstance g_motorB = { BspPwm_SetDutyB, TB6612_MOTOR_DEFAULT_MAX_DUTY };
static Tb6612MotorInstance g_motorC = { BspPwm_SetDutyC, TB6612_MOTOR_DEFAULT_MAX_DUTY };
static Tb6612MotorInstance g_motorD = { BspPwm_SetDutyD, TB6612_MOTOR_DEFAULT_MAX_DUTY };

static int32_t Tb6612Motor_ClampDuty(int32_t signedDuty, int32_t maximumDuty)
{
    if (signedDuty > maximumDuty) {
        return maximumDuty;
    }
    if (signedDuty < -maximumDuty) {
        return -maximumDuty;
    }
    return signedDuty;
}

static void Tb6612Motor_Apply(Tb6612MotorInstance *motor, int32_t signedDuty)
{
    motor->applyDuty(Tb6612Motor_ClampDuty(signedDuty, motor->maximumDuty));
}

void Tb6612Motor_Init(uint16_t maximumDuty)
{
    g_motorA.maximumDuty = (int32_t) maximumDuty;
    g_motorB.maximumDuty = (int32_t) maximumDuty;
    g_motorC.maximumDuty = (int32_t) maximumDuty;
    g_motorD.maximumDuty = (int32_t) maximumDuty;
    Tb6612Motor_StopCoast();
}

void Tb6612Motor_SetDutyA(int32_t signedDuty)
{
    Tb6612Motor_Apply(&g_motorA, signedDuty);
}

void Tb6612Motor_SetDutyB(int32_t signedDuty)
{
    Tb6612Motor_Apply(&g_motorB, signedDuty);
}

void Tb6612Motor_SetDutyC(int32_t signedDuty)
{
    Tb6612Motor_Apply(&g_motorC, signedDuty);
}

void Tb6612Motor_SetDutyD(int32_t signedDuty)
{
    Tb6612Motor_Apply(&g_motorD, signedDuty);
}

void Tb6612Motor_StopCoast(void)
{
    BspPwm_StopAll();
}
