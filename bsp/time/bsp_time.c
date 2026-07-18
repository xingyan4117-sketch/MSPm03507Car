#include "bsp_time.h"

#include "FreeRTOS.h"
#include "task.h"

uint32_t BspTime_GetMs(void)
{
    return (uint32_t)xTaskGetTickCount();
}
