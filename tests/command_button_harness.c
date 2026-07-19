#include <assert.h>
#include <stdio.h>

#include "app_types.h"
#include "command_task.h"

int main(void)
{
    AppMotorCommand command = {
        .state = APP_STATE_READY,
        .clearRequest = 0U,
        .externalFault = 0U,
        .reserved = 0U,
        .motorEnableMask = APP_MOTOR_ENABLE_ALL,
        .sequence = 0U,
        .issuedAtMs = 0U,
        .targetRpmA = COMMAND_MAX_RPM,
        .targetRpmB = COMMAND_MAX_RPM,
        .targetRpmC = COMMAND_MAX_RPM,
        .targetRpmD = COMMAND_MAX_RPM
    };

    assert(APP_LED_COLOR_COUNT == 4U);
    assert(APP_MOTOR_ENABLE_ALL == (APP_MOTOR_ENABLE_A |
        APP_MOTOR_ENABLE_B | APP_MOTOR_ENABLE_C | APP_MOTOR_ENABLE_D));

    CommandTask_ApplyButtonEvents(&command, true, false, false);
    assert(command.targetRpmA == COMMAND_MAX_RPM);
    assert(command.targetRpmB == COMMAND_MAX_RPM);
    assert(command.targetRpmC == COMMAND_MAX_RPM);
    assert(command.targetRpmD == COMMAND_MAX_RPM);

    command.targetRpmA = 0;
    command.targetRpmB = 0;
    command.targetRpmC = 0;
    command.targetRpmD = 0;
    CommandTask_ApplyButtonEvents(&command, false, true, false);
    assert(command.targetRpmA == 0);
    assert(command.targetRpmB == 0);
    assert(command.targetRpmC == 0);
    assert(command.targetRpmD == 0);

    CommandTask_ApplyButtonEvents(&command, false, false, true);
    assert(command.state == APP_STATE_RUN);
    assert(command.targetRpmA == COMMAND_START_RPM);
    assert(command.targetRpmB == COMMAND_START_RPM);
    assert(command.targetRpmC == COMMAND_START_RPM);
    assert(command.targetRpmD == COMMAND_START_RPM);

    CommandTask_ApplyButtonEvents(&command, false, false, true);
    assert(command.state == APP_STATE_READY);
    assert(command.targetRpmA == 0);
    assert(command.targetRpmB == 0);
    assert(command.targetRpmC == 0);
    assert(command.targetRpmD == 0);

    CommandTask_ApplyButtonEvents(&command, true, false, false);
    assert(command.targetRpmA == COMMAND_SPEED_STEP_RPM);
    assert(command.targetRpmB == COMMAND_SPEED_STEP_RPM);
    assert(command.targetRpmC == COMMAND_SPEED_STEP_RPM);
    assert(command.targetRpmD == COMMAND_SPEED_STEP_RPM);
    CommandTask_ApplyButtonEvents(&command, false, true, false);
    assert(command.targetRpmA == 0);
    assert(command.targetRpmB == 0);
    assert(command.targetRpmC == 0);
    assert(command.targetRpmD == 0);

    CommandTask_ApplyButtonEvents(&command, false, false, true);
    assert(command.targetRpmA == COMMAND_START_RPM);
    assert(command.targetRpmB == COMMAND_START_RPM);
    assert(command.targetRpmC == COMMAND_START_RPM);
    assert(command.targetRpmD == COMMAND_START_RPM);

    command.menuItem = APP_MENU_MOTOR;
    command.menuLevel = 0U;
    command.menuSubItem = APP_MOTOR_SUBITEM_RUN;
    command.editMode = 0U;
    command.targetRpmA = COMMAND_SPEED_STEP_RPM;
    command.targetRpmB = COMMAND_SPEED_STEP_RPM;
    command.targetRpmC = COMMAND_SPEED_STEP_RPM;
    command.targetRpmD = COMMAND_SPEED_STEP_RPM;
    CommandTask_ApplyMenuEvents(&command, false, true, false);
    assert(command.menuItem == APP_MENU_BUZZER);
    CommandTask_ApplyMenuEvents(&command, false, true, false);
    assert(command.menuItem == APP_MENU_LED);
    CommandTask_ApplyMenuEvents(&command, false, true, false);
    assert(command.menuItem == APP_MENU_MOTOR);

    command.menuItem = APP_MENU_MOTOR;
    command.menuLevel = 1U;
    command.editMode = 1U;
    command.menuSubItem = APP_MOTOR_SUBITEM_SPEED;
    command.targetRpmA = 590;
    command.targetRpmB = 590;
    command.targetRpmC = 590;
    command.targetRpmD = 590;
    CommandTask_ApplyMenuEvents(&command, true, false, false);
    assert(command.targetRpmA == COMMAND_MAX_RPM);
    assert(command.targetRpmB == COMMAND_MAX_RPM);

    command.menuItem = APP_MENU_BUZZER;
    command.menuSubItem = APP_BUZZER_SUBITEM_MODE;
    command.buzzerMode = APP_BUZZER_MODE_OFF;
    command.buzzerVolume = APP_BUZZER_VOLUME_MIN;
    CommandTask_ApplyMenuEvents(&command, true, false, false);
    assert(command.buzzerMode == APP_BUZZER_MODE_CONT);
    command.menuSubItem = APP_BUZZER_SUBITEM_VOLUME;
    CommandTask_ApplyMenuEvents(&command, true, false, false);
    assert(command.buzzerVolume == 10U);

    command.menuItem = APP_MENU_LED;
    command.menuLevel = 1U;
    command.editMode = 0U;
    command.menuSubItem = APP_LED_SUBITEM_LED1;
    CommandTask_ApplyMenuEvents(&command, false, false, true);
    assert(command.menuLevel == 1U);
    assert(command.editMode == 1U);
    command.ledColor[0] = APP_LED_COLOR_RED;
    command.ledColor[1] = APP_LED_COLOR_BLUE;
    command.ledColor[2] = APP_LED_COLOR_GREEN;
    CommandTask_ApplyMenuEvents(&command, true, false, false);
    assert(command.ledColor[0] == APP_LED_COLOR_GREEN);
    assert(command.ledColor[1] == APP_LED_COLOR_BLUE);
    assert(command.ledColor[2] == APP_LED_COLOR_GREEN);
    command.ledColor[0] = APP_LED_COLOR_WHITE;
    CommandTask_ApplyMenuEvents(&command, true, false, false);
    assert(command.ledColor[0] == APP_LED_COLOR_RED);
    CommandTask_ApplyMenuEvents(&command, false, true, false);
    assert(command.ledColor[0] == APP_LED_COLOR_WHITE);

    command.menuItem = APP_MENU_MOTOR;
    command.menuLevel = 1U;
    command.editMode = 0U;
    command.menuSubItem = APP_MOTOR_SUBITEM_ENABLE_B;
    command.motorEnableMask = APP_MOTOR_ENABLE_ALL;
    CommandTask_ApplyMenuEvents(&command, false, false, true);
    assert(command.editMode == 1U);
    CommandTask_ApplyMenuEvents(&command, false, true, false);
    assert((command.motorEnableMask & APP_MOTOR_ENABLE_B) == 0U);
    assert((command.motorEnableMask & (APP_MOTOR_ENABLE_A |
        APP_MOTOR_ENABLE_C | APP_MOTOR_ENABLE_D)) ==
        (APP_MOTOR_ENABLE_A | APP_MOTOR_ENABLE_C | APP_MOTOR_ENABLE_D));
    CommandTask_ApplyMenuEvents(&command, true, false, false);
    assert(command.motorEnableMask == APP_MOTOR_ENABLE_ALL);
    CommandTask_ApplyMenuEvents(&command, false, false, true);
    assert(command.editMode == 0U);

    puts("PASS: CommandTask button state machine behavior.");
    return 0;
}
