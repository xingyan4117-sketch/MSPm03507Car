#include "app_init.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "bsp_gpio.h"
#include "command_task.h"
#include "log_task.h"
#include "motor_control_task.h"
#include "safety_task.h"
#include "tb6612_motor.h"
#include "ui_task.h"

static StaticQueue_t g_commandQueueStorage;
static StaticQueue_t g_statusQueueStorage;
static StaticQueue_t g_safetyCommandQueueStorage;
static StaticQueue_t g_motorStatusQueueStorage;
static StaticQueue_t g_safetyStatusQueueStorage;
static uint8_t g_commandQueueBuffer[sizeof(AppMotorCommand)];
static uint8_t g_safetyCommandQueueBuffer[sizeof(AppMotorCommand)];
static uint8_t g_motorStatusQueueBuffer[sizeof(AppMotorStatus)];
static uint8_t g_safetyStatusQueueBuffer[sizeof(AppMotorStatus)];
static uint8_t g_statusQueueBuffer[sizeof(AppMotorStatus)];
static QueueHandle_t g_commandQueue;
static QueueHandle_t g_safetyCommandQueue;
static QueueHandle_t g_motorStatusQueue;
static QueueHandle_t g_safetyStatusQueue;
static QueueHandle_t g_statusQueue;

bool App_Init(void)
{
    g_commandQueue = xQueueCreateStatic(1U, sizeof(AppMotorCommand),
        g_commandQueueBuffer, &g_commandQueueStorage);
    g_statusQueue = xQueueCreateStatic(1U, sizeof(AppMotorStatus),
        g_statusQueueBuffer, &g_statusQueueStorage);
    g_safetyCommandQueue = xQueueCreateStatic(1U, sizeof(AppMotorCommand),
        g_safetyCommandQueueBuffer, &g_safetyCommandQueueStorage);
    g_motorStatusQueue = xQueueCreateStatic(1U, sizeof(AppMotorStatus),
        g_motorStatusQueueBuffer, &g_motorStatusQueueStorage);
    g_safetyStatusQueue = xQueueCreateStatic(1U, sizeof(AppMotorStatus),
        g_safetyStatusQueueBuffer, &g_safetyStatusQueueStorage);
    if ((g_commandQueue == NULL) || (g_safetyCommandQueue == NULL) ||
            (g_motorStatusQueue == NULL) || (g_safetyStatusQueue == NULL) ||
            (g_statusQueue == NULL) ||
            !MotorControl_Init() || !CommandTask_Init() || !SafetyTask_Init() ||
            !UiTask_Init() || !LogTask_Init()) {
        Tb6612Motor_StopCoast();
        return false;
    }

    return true;
}

AppMotorCommand App_DefaultCommand(void)
{
    AppMotorCommand command = {
        .state = APP_STATE_READY,
        .clearRequest = 0U,
        .externalFault = 0U,
        .reserved = 0U,
        .menuItem = APP_MENU_MOTOR,
        .menuLevel = 0U,
        .menuSubItem = APP_MOTOR_SUBITEM_RUN,
        .editMode = 0U,
        .buzzerMode = APP_BUZZER_MODE_OFF,
        .buzzerVolume = 60U,
        .ledCount = 0U,
        .ledBrightness = 30U,
        .ledColor = { APP_LED_COLOR_RED, APP_LED_COLOR_RED, APP_LED_COLOR_RED },
        .sequence = 0U,
        .issuedAtMs = 0U,
        .targetRpmA = 0,
        .targetRpmB = 0,
        .targetRpmC = 0,
        .targetRpmD = 0
    };

    return command;
}

bool App_PublishCommand(const AppMotorCommand *command)
{
    return (command != NULL) && (g_commandQueue != NULL) &&
        (g_safetyCommandQueue != NULL) &&
        (xQueueOverwrite(g_commandQueue, command) == pdPASS) &&
        (xQueueOverwrite(g_safetyCommandQueue, command) == pdPASS);
}

bool App_ReadLatestCommand(AppMotorCommand *command)
{
    return (command != NULL) && (g_commandQueue != NULL) &&
        (xQueueReceive(g_commandQueue, command, 0U) == pdPASS);
}

bool App_ReadLatestSafetyCommand(AppMotorCommand *command)
{
    return (command != NULL) && (g_safetyCommandQueue != NULL) &&
        (xQueueReceive(g_safetyCommandQueue, command, 0U) == pdPASS);
}

bool App_PublishMotorStatus(const AppMotorStatus *status)
{
    return (status != NULL) && (g_motorStatusQueue != NULL) &&
        (xQueueOverwrite(g_motorStatusQueue, status) == pdPASS);
}

bool App_ReadLatestMotorStatus(AppMotorStatus *status)
{
    return (status != NULL) && (g_motorStatusQueue != NULL) &&
        (xQueueReceive(g_motorStatusQueue, status, 0U) == pdPASS);
}

bool App_PublishSafetyStatus(const AppMotorStatus *status)
{
    return (status != NULL) && (g_safetyStatusQueue != NULL) &&
        (xQueueOverwrite(g_safetyStatusQueue, status) == pdPASS);
}

bool App_ReadLatestSafetyStatus(AppMotorStatus *status)
{
    return (status != NULL) && (g_safetyStatusQueue != NULL) &&
        (xQueueReceive(g_safetyStatusQueue, status, 0U) == pdPASS);
}

bool App_PublishStatus(const AppMotorStatus *status)
{
    return (status != NULL) && (g_statusQueue != NULL) &&
        (xQueueOverwrite(g_statusQueue, status) == pdPASS);
}

bool App_ReadLatestStatus(AppMotorStatus *status)
{
    return (status != NULL) && (g_statusQueue != NULL) &&
        (xQueueReceive(g_statusQueue, status, 0U) == pdPASS);
}
