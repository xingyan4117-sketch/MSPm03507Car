#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BSP_GPIO_MOTOR_A,
    BSP_GPIO_MOTOR_B,
    BSP_GPIO_MOTOR_C,
    BSP_GPIO_MOTOR_D
} BspGpioMotor;

void BspGpio_Init(void);
void BspGpio_SetMotorDirection(BspGpioMotor motor, int32_t signedDuty);
bool BspGpio_ReadKeyDebounced(void);
bool BspGpio_ReadKey1Debounced(void);
bool BspGpio_ReadKey2Debounced(void);
bool BspGpio_ReadKey3Debounced(void);
void BspGpio_SetHeartbeat(bool enabled);
void BspGpio_ToggleHeartbeat(void);
void BspGpio_DisplaySetChipSelect(bool asserted);
void BspGpio_DisplaySetDataCommand(bool dataMode);
void BspGpio_DisplaySetReset(bool asserted);
void BspGpio_DisplaySetBacklight(bool enabled);

#endif
