#ifndef TASK5_HOST_TB6612_MOTOR_H
#define TASK5_HOST_TB6612_MOTOR_H

#include <stdint.h>

#define TB6612_MOTOR_DEFAULT_MAX_DUTY (800U)

void Tb6612Motor_Init(uint16_t maximumDuty);
void Tb6612Motor_SetDutyA(int32_t signedDuty);
void Tb6612Motor_SetDutyB(int32_t signedDuty);
void Tb6612Motor_SetDutyC(int32_t signedDuty);
void Tb6612Motor_SetDutyD(int32_t signedDuty);
void Tb6612Motor_StopCoast(void);

#endif
