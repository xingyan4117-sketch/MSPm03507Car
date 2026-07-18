#include "FreeRTOS.h"
#include "task.h"
#include "ti_msp_dl_config.h"
#include "app_init.h"
#include "bsp_encoder.h"
#include "bsp_freertos.h"
#include "bsp_gpio.h"
#include "bsp_pwm.h"
#include "bsp/buzzer/bsp_buzzer.h"
#include "bsp/ws2812/bsp_ws2812.h"

int main(void)
{
    SYSCFG_DL_init();
    BspPwm_Init();
    BspGpio_Init();
    BspBuzzer_Init();
    BspWs2812_Init();
    BspEncoder_Init();

    if (!BspFreeRTOS_Init() || !App_Init()) {
        BspPwm_StopAll();
        for (;;) {}
    }

    vTaskStartScheduler();
    BspPwm_StopAll();
    for (;;) {}
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *taskName)
{
    (void)task;
    (void)taskName;
    BspPwm_StopAll();
    for (;;) {}
}
