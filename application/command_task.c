#include "command_task.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app_init.h"
#include "bsp_gpio.h"
#include "../bsp/buzzer/bsp_buzzer.h"
#include "../bsp/ws2812/bsp_ws2812.h"

#define COMMAND_TASK_STACK_WORDS (128U)
#define COMMAND_TASK_PRIORITY (1U)

static StaticTask_t g_commandTaskStorage;
static StackType_t g_commandTaskStack[COMMAND_TASK_STACK_WORDS];

static void CommandTask_SetRunState(AppMotorCommand *command, bool run)
{
    command->state = run ? APP_STATE_RUN : APP_STATE_READY;
    if (!run) {
        command->targetRpmA = 0;
        command->targetRpmB = 0;
        command->targetRpmC = 0;
        command->targetRpmD = 0;
    } else if ((command->targetRpmA == 0) && (command->targetRpmB == 0) &&
        (command->targetRpmC == 0) && (command->targetRpmD == 0)) {
        command->targetRpmA = COMMAND_START_RPM;
        command->targetRpmB = COMMAND_START_RPM;
        command->targetRpmC = COMMAND_START_RPM;
        command->targetRpmD = COMMAND_START_RPM;
    }
}

static int32_t CommandTask_IncreaseTarget(int32_t targetRpm)
{
    if (targetRpm >= (COMMAND_MAX_RPM - COMMAND_SPEED_STEP_RPM)) {
        return COMMAND_MAX_RPM;
    }
    return targetRpm + COMMAND_SPEED_STEP_RPM;
}

static int32_t CommandTask_DecreaseTarget(int32_t targetRpm)
{
    if (targetRpm <= COMMAND_SPEED_STEP_RPM) {
        return 0;
    }
    return targetRpm - COMMAND_SPEED_STEP_RPM;
}

void CommandTask_ApplyButtonEvents(AppMotorCommand *command, bool speedUp,
    bool speedDown, bool toggleRun)
{
    if (command == NULL) {
        return;
    }

    if (speedUp) {
        command->targetRpmA = CommandTask_IncreaseTarget(command->targetRpmA);
        command->targetRpmB = CommandTask_IncreaseTarget(command->targetRpmB);
        command->targetRpmC = CommandTask_IncreaseTarget(command->targetRpmC);
        command->targetRpmD = CommandTask_IncreaseTarget(command->targetRpmD);
    }
    if (speedDown) {
        command->targetRpmA = CommandTask_DecreaseTarget(command->targetRpmA);
        command->targetRpmB = CommandTask_DecreaseTarget(command->targetRpmB);
        command->targetRpmC = CommandTask_DecreaseTarget(command->targetRpmC);
        command->targetRpmD = CommandTask_DecreaseTarget(command->targetRpmD);
    }
    if (toggleRun && (command->state == APP_STATE_RUN)) {
        command->state = APP_STATE_READY;
        command->targetRpmA = 0;
        command->targetRpmB = 0;
        command->targetRpmC = 0;
        command->targetRpmD = 0;
    } else if (toggleRun && (command->state == APP_STATE_READY)) {
        command->state = APP_STATE_RUN;
        if ((command->targetRpmA == 0) && (command->targetRpmB == 0) &&
            (command->targetRpmC == 0) && (command->targetRpmD == 0)) {
            command->targetRpmA = COMMAND_START_RPM;
            command->targetRpmB = COMMAND_START_RPM;
            command->targetRpmC = COMMAND_START_RPM;
            command->targetRpmD = COMMAND_START_RPM;
        }
    }
}

static uint8_t CommandTask_SubItemCount(const AppMotorCommand *command)
{
    if (command->menuItem == APP_MENU_MOTOR) {
        return APP_MOTOR_SUBITEM_COUNT;
    }
    if (command->menuItem == APP_MENU_BUZZER) {
        return APP_BUZZER_SUBITEM_COUNT;
    }
    return APP_LED_SUBITEM_COUNT_TOTAL;
}

static bool CommandTask_IsBackItem(const AppMotorCommand *command)
{
    if (command->menuItem == APP_MENU_MOTOR) {
        return command->menuSubItem == APP_MOTOR_SUBITEM_BACK;
    }
    if (command->menuItem == APP_MENU_BUZZER) {
        return command->menuSubItem == APP_BUZZER_SUBITEM_BACK;
    }
    return command->menuSubItem == APP_LED_SUBITEM_BACK;
}

static uint8_t CommandTask_MotorEnableBit(uint8_t subItem)
{
    switch (subItem) {
    case APP_MOTOR_SUBITEM_ENABLE_A: return APP_MOTOR_ENABLE_A;
    case APP_MOTOR_SUBITEM_ENABLE_B: return APP_MOTOR_ENABLE_B;
    case APP_MOTOR_SUBITEM_ENABLE_C: return APP_MOTOR_ENABLE_C;
    case APP_MOTOR_SUBITEM_ENABLE_D: return APP_MOTOR_ENABLE_D;
    default: return 0U;
    }
}

void CommandTask_ApplyMenuEvents(AppMotorCommand *command, bool key1,
    bool key2, bool key3)
{
    uint8_t itemCount;

    if (command == NULL) {
        return;
    }

    if (command->menuLevel == 0U) {
        if (key1) {
            command->menuItem = (command->menuItem == 0U) ?
                (APP_MENU_COUNT - 1U) : (uint8_t)(command->menuItem - 1U);
        }
        if (key2) {
            command->menuItem = (uint8_t)((command->menuItem + 1U) % APP_MENU_COUNT);
        }
        if (key3) {
            command->menuLevel = 1U;
            command->menuSubItem = 0U;
        }
        return;
    }

    if (!command->editMode) {
        itemCount = CommandTask_SubItemCount(command);
        if (key1) {
            command->menuSubItem = (command->menuSubItem == 0U) ?
                (uint8_t)(itemCount - 1U) : (uint8_t)(command->menuSubItem - 1U);
        }
        if (key2) {
            command->menuSubItem = (uint8_t)((command->menuSubItem + 1U) % itemCount);
        }
        if (key3) {
            if (CommandTask_IsBackItem(command)) {
                command->menuLevel = 0U;
            } else {
                command->editMode = 1U;
            }
        }
        return;
    }

    if (command->menuItem == APP_MENU_MOTOR) {
        if (command->menuSubItem == APP_MOTOR_SUBITEM_RUN && key1) {
            CommandTask_SetRunState(command, command->state != APP_STATE_RUN);
        } else if ((command->menuSubItem == APP_MOTOR_SUBITEM_SPEED) && key1) {
            CommandTask_ApplyButtonEvents(command, true, false, false);
        } else if ((command->menuSubItem == APP_MOTOR_SUBITEM_SPEED) && key2) {
            CommandTask_ApplyButtonEvents(command, false, true, false);
        } else if (CommandTask_MotorEnableBit(command->menuSubItem) != 0U) {
            uint8_t bit = CommandTask_MotorEnableBit(command->menuSubItem);
            if (key1) {
                command->motorEnableMask |= bit;
            }
            if (key2) {
                command->motorEnableMask &= (uint8_t) ~bit;
            }
        }
    } else if (command->menuItem == APP_MENU_BUZZER) {
        if (command->menuSubItem == APP_BUZZER_SUBITEM_MODE) {
            if (key1) {
                command->buzzerMode = (uint8_t)((command->buzzerMode + 1U) % 4U);
            }
            if (key2) {
                command->buzzerMode = (command->buzzerMode == 0U) ? 3U :
                    (uint8_t)(command->buzzerMode - 1U);
            }
        } else if (command->menuSubItem == APP_BUZZER_SUBITEM_VOLUME) {
            if (key1 && (command->buzzerVolume < APP_BUZZER_VOLUME_MAX)) {
                command->buzzerVolume = (uint8_t)(command->buzzerVolume + 10U);
            }
            if (key2 && (command->buzzerVolume > APP_BUZZER_VOLUME_MIN)) {
                command->buzzerVolume = (uint8_t)(command->buzzerVolume - 10U);
            }
        }
    } else {
        if (command->menuSubItem == APP_LED_SUBITEM_COUNT) {
            if (key1 && (command->ledCount < 3U)) command->ledCount++;
            if (key2 && (command->ledCount > 0U)) command->ledCount--;
        } else if (command->menuSubItem == APP_LED_SUBITEM_BRIGHTNESS) {
            if (key1 && (command->ledBrightness < 100U)) command->ledBrightness += 10U;
            if (key2 && (command->ledBrightness >= 10U)) command->ledBrightness -= 10U;
        } else if (command->menuSubItem >= APP_LED_SUBITEM_LED1 &&
            command->menuSubItem <= APP_LED_SUBITEM_LED3) {
            uint8_t colorIndex = (uint8_t)(command->menuSubItem - APP_LED_SUBITEM_LED1);
            if (key1) command->ledColor[colorIndex] =
                (uint8_t)((command->ledColor[colorIndex] + 1U) % APP_LED_COLOR_COUNT);
            if (key2) command->ledColor[colorIndex] =
                (command->ledColor[colorIndex] == 0U) ? (APP_LED_COLOR_COUNT - 1U) :
                (uint8_t)(command->ledColor[colorIndex] - 1U);
        }
    }
    if (key3) {
        command->editMode = 0U;
    }
}

static void CommandTask(void *argument)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    AppMotorCommand command = App_DefaultCommand();
    bool previousKey1Pressed = false;
    bool previousKey2Pressed = false;
    bool previousKey3Pressed = false;

    (void) argument;
    for (;;) {
        bool key1Pressed = BspGpio_ReadKey1Debounced();
        bool key2Pressed = BspGpio_ReadKey2Debounced();
        bool key3Pressed = BspGpio_ReadKey3Debounced();

        bool key1Event = key1Pressed && !previousKey1Pressed;
        bool key2Event = key2Pressed && !previousKey2Pressed;
        bool key3Event = key3Pressed && !previousKey3Pressed;

        CommandTask_ApplyMenuEvents(&command, key1Event, key2Event, key3Event);
        previousKey1Pressed = key1Pressed;
        previousKey2Pressed = key2Pressed;
        previousKey3Pressed = key3Pressed;
        command.issuedAtMs = (uint32_t) xTaskGetTickCount() * portTICK_PERIOD_MS;
        command.sequence++;
        (void) App_PublishCommand(&command);
        BspBuzzer_SetConfig(command.buzzerMode, command.buzzerVolume,
            command.issuedAtMs);
        BspBuzzer_Tick(command.issuedAtMs);
        BspWs2812_SetConfig(command.ledCount, command.ledBrightness,
            command.ledColor);
        BspWs2812_Tick(command.issuedAtMs);
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(COMMAND_TASK_PERIOD_MS));
    }
}

bool CommandTask_Init(void)
{
    return xTaskCreateStatic(CommandTask, "command", COMMAND_TASK_STACK_WORDS, NULL,
        COMMAND_TASK_PRIORITY, g_commandTaskStack, &g_commandTaskStorage) != NULL;
}
