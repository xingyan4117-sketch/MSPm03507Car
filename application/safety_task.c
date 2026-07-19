#include "safety_task.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app_init.h"
#include "../bsp/buzzer/bsp_buzzer.h"
#include "bsp_gpio.h"
#include "../bsp/ws2812/bsp_ws2812.h"
#include "tb6612_motor.h"

#define SAFETY_TASK_STACK_WORDS (192U)
#define SAFETY_TASK_PRIORITY (3U)
#define SAFETY_COMMAND_TIMEOUT_MS (100U)

static StaticTask_t g_safetyTaskStorage;
static StackType_t g_safetyTaskStack[SAFETY_TASK_STACK_WORDS];

static bool SafetyTask_PeripheralOutputsUnsafe(AppState state)
{
    return (state == APP_STATE_ESTOP) || (state == APP_STATE_FAULT);
}

static SafetyInput SafetyTask_MakeInput(const AppMotorCommand *command,
    const AppMotorStatus *motorStatus, bool keyPressed, bool externalFault,
    bool clearRequest, uint32_t nowMs)
{
    bool commandExternalFault = false;
    bool commandClearRequest = false;
    SafetyInput input = { false, false, false, false, false, true };

    input.keyPressed = keyPressed;
    if (command != NULL) {
        commandExternalFault = command->externalFault != 0U;
        commandClearRequest = command->clearRequest != 0U;
    }
    input.clearRequest = clearRequest || commandClearRequest;
    input.faultPresent = externalFault || commandExternalFault || ((motorStatus != NULL) &&
        ((motorStatus->invalidTransitionsA != 0U) ||
         (motorStatus->invalidTransitionsB != 0U) ||
         (motorStatus->invalidTransitionsC != 0U) ||
         (motorStatus->invalidTransitionsD != 0U)));
    if (command != NULL) {
        input.commandRequestsRun = command->state == APP_STATE_RUN;
        input.commandUsable = input.commandRequestsRun &&
            ((uint32_t) (nowMs - command->issuedAtMs) <= SAFETY_COMMAND_TIMEOUT_MS);
        input.commandIsZero = (command->targetRpmA == 0) &&
            (command->targetRpmB == 0) && (command->targetRpmC == 0) &&
            (command->targetRpmD == 0);
    }
    return input;
}

void SafetyTask_ProcessCycle(SafetyController *controller,
    const AppMotorCommand *command, const AppMotorStatus *motorStatus,
    bool keyPressed, bool externalFault, bool clearRequest, uint32_t nowMs)
{
    AppMotorStatus published = {
        .state = APP_STATE_INIT,
        .commandTimedOut = 1U,
        .observedAtMs = nowMs
    };
    SafetyInput input = SafetyTask_MakeInput(command, motorStatus, keyPressed,
        externalFault, clearRequest, nowMs);

    if (motorStatus != NULL) {
        published = *motorStatus;
        published.observedAtMs = nowMs;
    }
    published.state = Safety_Advance(controller, &input);
    published.commandTimedOut = input.commandUsable ? 0U : 1U;
    if (command != NULL) {
        published.commandSequence = command->sequence;
        published.menuItem = command->menuItem;
        published.menuLevel = command->menuLevel;
        published.menuSubItem = command->menuSubItem;
        published.editMode = command->editMode;
        published.targetRpmA = command->targetRpmA;
        published.buzzerMode = command->buzzerMode;
        published.buzzerVolume = command->buzzerVolume;
        published.ledCount = command->ledCount;
        published.ledBrightness = command->ledBrightness;
        published.ledColor[0] = command->ledColor[0];
        published.ledColor[1] = command->ledColor[1];
        published.ledColor[2] = command->ledColor[2];
    }
    if (Safety_StateIsUnsafe(published.state)) {
        Tb6612Motor_StopCoast();
        published.dutyA = 0;
        published.dutyB = 0;
        published.dutyC = 0;
        published.dutyD = 0;
        if (SafetyTask_PeripheralOutputsUnsafe(published.state)) {
            BspBuzzer_Stop();
            BspWs2812_Off();
        }
    }
    (void) App_PublishSafetyStatus(&published);
    (void) App_PublishStatus(&published);
}

static void SafetyTask(void *argument)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    SafetyController controller = Safety_ControllerInit();
    AppMotorCommand command = App_DefaultCommand();
    AppMotorStatus motorStatus = {
        .state = APP_STATE_INIT,
        .commandTimedOut = 1U
    };

    (void) argument;
    for (;;) {
        (void) App_ReadLatestSafetyCommand(&command);
        (void) App_ReadLatestMotorStatus(&motorStatus);
        SafetyTask_ProcessCycle(&controller, &command, &motorStatus,
            BspGpio_ReadKeyDebounced(), command.externalFault != 0U,
            command.clearRequest != 0U,
            (uint32_t) xTaskGetTickCount() * portTICK_PERIOD_MS);
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(SAFETY_TASK_PERIOD_MS));
    }
}

bool SafetyTask_Init(void)
{
    return xTaskCreateStatic(SafetyTask, "safety", SAFETY_TASK_STACK_WORDS, NULL,
        SAFETY_TASK_PRIORITY, g_safetyTaskStack, &g_safetyTaskStorage) != NULL;
}
