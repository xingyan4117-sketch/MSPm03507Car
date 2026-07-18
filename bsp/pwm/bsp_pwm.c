#include "bsp_pwm.h"

#include "bsp_gpio.h"
#include "car_config.h"

#include <stdbool.h>

static bool g_pwmArmed;

static uint32_t BspPwm_ClampDuty(int32_t duty)
{
    uint32_t magnitude;

    if (duty < 0) {
        magnitude = (uint32_t)(-(duty + 1)) + 1U;
    } else {
        magnitude = (uint32_t)duty;
    }
    return magnitude > CAR_PWM_PERIOD_TICKS ? CAR_PWM_PERIOD_TICKS : magnitude;
}

static uint32_t BspPwm_CompareForDuty(int32_t duty)
{
    return CAR_PWM_PERIOD_TICKS - BspPwm_ClampDuty(duty);
}

static void BspPwm_Arm(void)
{
    if (g_pwmArmed) {
        return;
    }

    DL_GPIO_initPeripheralOutputFunction(CAR_MOTOR_A_PWM_IOMUX,
        CAR_MOTOR_A_PWM_IOMUX_FUNCTION);
    DL_GPIO_enableOutput(CAR_MOTOR_A_PWM_PORT, CAR_MOTOR_A_PWM_PIN);
    DL_GPIO_initPeripheralOutputFunction(CAR_MOTOR_B_PWM_IOMUX,
        CAR_MOTOR_B_PWM_IOMUX_FUNCTION);
    DL_GPIO_enableOutput(CAR_MOTOR_B_PWM_PORT, CAR_MOTOR_B_PWM_PIN);
    DL_GPIO_initPeripheralOutputFunction(CAR_MOTOR_C_PWM_IOMUX,
        CAR_MOTOR_C_PWM_IOMUX_FUNCTION);
    DL_GPIO_enableOutput(CAR_MOTOR_C_PWM_PORT, CAR_MOTOR_C_PWM_PIN);
    DL_GPIO_initPeripheralOutputFunction(CAR_MOTOR_D_PWM_IOMUX,
        CAR_MOTOR_D_PWM_IOMUX_FUNCTION);
    DL_GPIO_enableOutput(CAR_MOTOR_D_PWM_PORT, CAR_MOTOR_D_PWM_PIN);
    DL_TimerA_startCounter(CAR_MOTOR_A_PWM_TIMER);
    DL_TimerA_startCounter(CAR_MOTOR_C_PWM_TIMER);
    DL_TimerG_startCounter(CAR_MOTOR_D_PWM_TIMER);
    g_pwmArmed = true;
}

static void BspPwm_ForceSafePins(void)
{
    DL_GPIO_initDigitalOutput(CAR_MOTOR_A_PWM_IOMUX);
    DL_GPIO_initDigitalOutput(CAR_MOTOR_B_PWM_IOMUX);
    DL_GPIO_initDigitalOutput(CAR_MOTOR_C_PWM_IOMUX);
    DL_GPIO_initDigitalOutput(CAR_MOTOR_D_PWM_IOMUX);
    DL_GPIO_clearPins(CAR_MOTOR_A_PWM_PORT, CAR_MOTOR_A_PWM_PIN);
    DL_GPIO_clearPins(CAR_MOTOR_B_PWM_PORT, CAR_MOTOR_B_PWM_PIN);
    DL_GPIO_clearPins(CAR_MOTOR_C_PWM_PORT, CAR_MOTOR_C_PWM_PIN);
    DL_GPIO_clearPins(CAR_MOTOR_D_PWM_PORT, CAR_MOTOR_D_PWM_PIN);
    DL_GPIO_enableOutput(CAR_MOTOR_A_PWM_PORT, CAR_MOTOR_A_PWM_PIN);
    DL_GPIO_enableOutput(CAR_MOTOR_B_PWM_PORT, CAR_MOTOR_B_PWM_PIN);
    DL_GPIO_enableOutput(CAR_MOTOR_C_PWM_PORT, CAR_MOTOR_C_PWM_PIN);
    DL_GPIO_enableOutput(CAR_MOTOR_D_PWM_PORT, CAR_MOTOR_D_PWM_PIN);
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_A, 0);
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_B, 0);
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_C, 0);
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_D, 0);
}

void BspPwm_Init(void)
{
    BspPwm_StopAll();
}

void BspPwm_SetDutyA(int32_t duty)
{
    BspPwm_Arm();
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_A, duty);
    DL_TimerA_setCaptureCompareValue(CAR_MOTOR_A_PWM_TIMER,
        BspPwm_CompareForDuty(duty), CAR_MOTOR_A_PWM_COMPARE_INDEX);
}

void BspPwm_SetDutyB(int32_t duty)
{
    BspPwm_Arm();
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_B, duty);
    DL_TimerA_setCaptureCompareValue(CAR_MOTOR_B_PWM_TIMER,
        BspPwm_CompareForDuty(duty), CAR_MOTOR_B_PWM_COMPARE_INDEX);
}

void BspPwm_SetDutyC(int32_t duty)
{
    BspPwm_Arm();
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_C, duty);
    DL_TimerA_setCaptureCompareValue(CAR_MOTOR_C_PWM_TIMER,
        BspPwm_CompareForDuty(duty), CAR_MOTOR_C_PWM_COMPARE_INDEX);
}

void BspPwm_SetDutyD(int32_t duty)
{
    BspPwm_Arm();
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_D, duty);
    DL_TimerG_setCaptureCompareValue(CAR_MOTOR_D_PWM_TIMER,
        BspPwm_CompareForDuty(duty), CAR_MOTOR_D_PWM_COMPARE_INDEX);
}

void BspPwm_StopAll(void)
{
    DL_TimerA_setCaptureCompareValue(CAR_MOTOR_A_PWM_TIMER,
        CAR_PWM_PERIOD_TICKS, CAR_MOTOR_A_PWM_COMPARE_INDEX);
    DL_TimerA_setCaptureCompareValue(CAR_MOTOR_B_PWM_TIMER,
        CAR_PWM_PERIOD_TICKS,
        CAR_MOTOR_B_PWM_COMPARE_INDEX);
    DL_TimerA_setCaptureCompareValue(CAR_MOTOR_C_PWM_TIMER,
        CAR_PWM_PERIOD_TICKS,
        CAR_MOTOR_C_PWM_COMPARE_INDEX);
    DL_TimerG_setCaptureCompareValue(CAR_MOTOR_D_PWM_TIMER,
        CAR_PWM_PERIOD_TICKS, CAR_MOTOR_D_PWM_COMPARE_INDEX);
    DL_TimerA_stopCounter(CAR_MOTOR_A_PWM_TIMER);
    DL_TimerA_stopCounter(CAR_MOTOR_C_PWM_TIMER);
    DL_TimerG_stopCounter(CAR_MOTOR_D_PWM_TIMER);
    g_pwmArmed = false;
    BspPwm_ForceSafePins();
}
