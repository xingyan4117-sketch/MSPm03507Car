#ifndef TASK5_HOST_BSP_GPIO_H
#define TASK5_HOST_BSP_GPIO_H

#include <stdbool.h>

bool BspGpio_ReadKeyDebounced(void);
bool BspGpio_ReadKey1Debounced(void);
bool BspGpio_ReadKey2Debounced(void);
bool BspGpio_ReadKey3Debounced(void);

#endif
