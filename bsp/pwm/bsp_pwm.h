#ifndef BSP_PWM_H
#define BSP_PWM_H

#include <stdint.h>

void BspPwm_Init(void);
void BspPwm_SetDutyA(int32_t duty);
void BspPwm_SetDutyB(int32_t duty);
void BspPwm_SetDutyC(int32_t duty);
void BspPwm_SetDutyD(int32_t duty);
void BspPwm_StopAll(void);

#endif
