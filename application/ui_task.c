#include "ui_task.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app_init.h"
#include "st7735_ui.h"

#define UI_TASK_STACK_WORDS (256U)
#define UI_TASK_PRIORITY (1U)

static StaticTask_t g_uiTaskStorage;
static StackType_t g_uiTaskStack[UI_TASK_STACK_WORDS];

static void UiTask(void *argument)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    AppMotorStatus status = {
        .state = APP_STATE_INIT,
        .commandTimedOut = 1U
    };

    (void) argument;
    St7735Ui_Init();
    for (;;) {
        (void) App_ReadLatestStatus(&status);
        St7735Ui_RenderStatus(&status);
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(UI_TASK_PERIOD_MS));
    }
}

bool UiTask_Init(void)
{
    return xTaskCreateStatic(UiTask, "ui", UI_TASK_STACK_WORDS, NULL,
        UI_TASK_PRIORITY, g_uiTaskStack, &g_uiTaskStorage) != NULL;
}

const char *UiTask_GetLastFrame(void)
{
    return St7735Ui_GetLastFrame();
}
