#include "motor_control_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "app_init.h"
#include "encoder_ab.h"
#include "tb6612_motor.h"

#define MOTOR_CONTROL_TASK_STACK_WORDS (256U)
#define MOTOR_CONTROL_TASK_PRIORITY (2U)
#define MOTOR_CONTROL_ENCODER_COUNTS_PER_REVOLUTION (1024U)

static StaticTask_t g_motorControlTaskStorage;
static StackType_t g_motorControlTaskStack[MOTOR_CONTROL_TASK_STACK_WORDS];
static SpeedPid g_pidA;
static SpeedPid g_pidB;
static SpeedPid g_pidC;
static SpeedPid g_pidD;

static uint32_t MotorControl_NowMs(void)
{
    return (uint32_t) xTaskGetTickCount() * portTICK_PERIOD_MS;
}

bool MotorControl_CommandIsUsable(const AppMotorCommand *command, uint32_t nowMs)
{
    return (command != NULL) && (command->state == APP_STATE_RUN) &&
        ((uint32_t) (nowMs - command->issuedAtMs) <= MOTOR_CONTROL_COMMAND_TIMEOUT_MS);
}

static bool MotorControl_SafetyStatusBlocksDrive(const AppMotorStatus *safetyStatus)
{
    if (safetyStatus == NULL) {
        return true;
    }

    switch (safetyStatus->state) {
    case APP_STATE_RUN:
        return false;
    case APP_STATE_INIT:
    case APP_STATE_READY:
    case APP_STATE_ESTOP:
    case APP_STATE_FAULT:
    default:
        return true;
    }
}

void MotorControl_Process(SpeedPid *pidA, SpeedPid *pidB, SpeedPid *pidC,
    SpeedPid *pidD,
    const AppMotorCommand *command, uint32_t nowMs,
    const AppEncoderSnapshot *encoders, const AppMotorStatus *safetyStatus,
    AppMotorStatus *status)
{
    SpeedPidStatus pidStatusA;
    SpeedPidStatus pidStatusB;
    SpeedPidStatus pidStatusC;
    SpeedPidStatus pidStatusD;

    if ((pidA == NULL) || (pidB == NULL) || (pidC == NULL) || (pidD == NULL) ||
        (encoders == NULL) || (status == NULL)) {
        return;
    }

    *status = (AppMotorStatus) {
        .state = APP_STATE_READY,
        .commandTimedOut = 1U,
        .observedAtMs = nowMs,
        .rpmA = encoders->rpmA,
        .rpmB = encoders->rpmB,
        .rpmC = encoders->rpmC,
        .rpmD = encoders->rpmD,
        .invalidTransitionsA = encoders->invalidTransitionsA,
        .invalidTransitionsB = encoders->invalidTransitionsB,
        .invalidTransitionsC = encoders->invalidTransitionsC,
        .invalidTransitionsD = encoders->invalidTransitionsD,
        .menuItem = (command != NULL) ? command->menuItem : APP_MENU_MOTOR,
        .menuLevel = (command != NULL) ? command->menuLevel : 0U,
        .menuSubItem = (command != NULL) ? command->menuSubItem : 0U,
        .editMode = (command != NULL) ? command->editMode : 0U,
        .targetRpmA = (command != NULL) ? command->targetRpmA : 0,
        .buzzerMode = (command != NULL) ? command->buzzerMode : APP_BUZZER_MODE_OFF,
        .buzzerVolume = (command != NULL) ? command->buzzerVolume : 0U,
        .ledCount = (command != NULL) ? command->ledCount : 0U,
        .ledBrightness = (command != NULL) ? command->ledBrightness : 0U
    };
    if (command != NULL) {
        status->ledColor[0] = command->ledColor[0];
        status->ledColor[1] = command->ledColor[1];
        status->ledColor[2] = command->ledColor[2];
    } else {
        status->ledColor[0] = APP_LED_COLOR_RED;
        status->ledColor[1] = APP_LED_COLOR_RED;
        status->ledColor[2] = APP_LED_COLOR_RED;
    }
    if (MotorControl_SafetyStatusBlocksDrive(safetyStatus)) {
        SpeedPid_Reset(pidA);
        SpeedPid_Reset(pidB);
        SpeedPid_Reset(pidC);
        SpeedPid_Reset(pidD);
        status->state = (safetyStatus != NULL) ? safetyStatus->state : APP_STATE_FAULT;
        return;
    }
    if (!MotorControl_CommandIsUsable(command, nowMs)) {
        SpeedPid_Reset(pidA);
        SpeedPid_Reset(pidB);
        SpeedPid_Reset(pidC);
        SpeedPid_Reset(pidD);
        return;
    }

    pidStatusA = SpeedPid_Update(pidA, command->targetRpmA, encoders->rpmA);
    pidStatusB = SpeedPid_Update(pidB, command->targetRpmB, encoders->rpmB);
    pidStatusC = SpeedPid_Update(pidC, command->targetRpmC, encoders->rpmC);
    pidStatusD = SpeedPid_Update(pidD, command->targetRpmD, encoders->rpmD);
    status->state = APP_STATE_RUN;
    status->commandTimedOut = 0U;
    status->commandSequence = command->sequence;
    status->dutyA = pidStatusA.output;
    status->dutyB = pidStatusB.output;
    status->dutyC = pidStatusC.output;
    status->dutyD = pidStatusD.output;
}

static AppEncoderSnapshot MotorControl_ReadEncoders(void)
{
    const EncoderAbRpmConfig rpmConfig = {
        MOTOR_CONTROL_ENCODER_COUNTS_PER_REVOLUTION, MOTOR_CONTROL_SAMPLE_PERIOD_MS
    };
    EncoderAbSnapshot encoderA = EncoderAb_ReadA();
    EncoderAbSnapshot encoderB = EncoderAb_ReadB();
    EncoderAbSnapshot encoderC = EncoderAb_ReadC();
    EncoderAbSnapshot encoderD = EncoderAb_ReadD();
    AppEncoderSnapshot encoders = {
        .deltaCountsA = encoderA.deltaCounts,
        .deltaCountsB = encoderB.deltaCounts,
        .deltaCountsC = encoderC.deltaCounts,
        .deltaCountsD = encoderD.deltaCounts,
        .rpmA = EncoderAb_CountsToRpm(&encoderA, &rpmConfig),
        .rpmB = EncoderAb_CountsToRpm(&encoderB, &rpmConfig),
        .rpmC = EncoderAb_CountsToRpm(&encoderC, &rpmConfig),
        .rpmD = EncoderAb_CountsToRpm(&encoderD, &rpmConfig),
        .invalidTransitionsA = encoderA.invalidTransitions,
        .invalidTransitionsB = encoderB.invalidTransitions,
        .invalidTransitionsC = encoderC.invalidTransitions,
        .invalidTransitionsD = encoderD.invalidTransitions
    };

    return encoders;
}

static void MotorControlTask(void *argument)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    AppMotorCommand command = App_DefaultCommand();
    AppMotorStatus safetyStatus = { .state = APP_STATE_READY };

    (void) argument;
    for (;;) {
        AppEncoderSnapshot encoders = MotorControl_ReadEncoders();
        AppMotorStatus status;

        if (App_ReadLatestCommand(&command)) {
            /* Retain the latest command between queue updates until it expires. */
        }
        (void) App_ReadLatestSafetyStatus(&safetyStatus);
        MotorControl_Process(&g_pidA, &g_pidB, &g_pidC, &g_pidD,
            &command, MotorControl_NowMs(),
            &encoders, &safetyStatus, &status);
        if (status.state == APP_STATE_RUN) {
            Tb6612Motor_SetDutyA(status.dutyA);
            Tb6612Motor_SetDutyB(status.dutyB);
            Tb6612Motor_SetDutyC(status.dutyC);
            Tb6612Motor_SetDutyD(status.dutyD);
        } else {
            Tb6612Motor_StopCoast();
        }
        (void) App_PublishMotorStatus(&status);
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1U));
    }
}

bool MotorControl_Init(void)
{
    SpeedPidConfig config = SpeedPid_DefaultConfig();

    config.outputMin = -((int32_t) TB6612_MOTOR_DEFAULT_MAX_DUTY);
    config.outputMax = (int32_t) TB6612_MOTOR_DEFAULT_MAX_DUTY;
    Tb6612Motor_Init(TB6612_MOTOR_DEFAULT_MAX_DUTY);
    SpeedPid_Init(&g_pidA, &config);
    SpeedPid_Init(&g_pidB, &config);
    SpeedPid_Init(&g_pidC, &config);
    SpeedPid_Init(&g_pidD, &config);
    if (xTaskCreateStatic(MotorControlTask, "motorctl", MOTOR_CONTROL_TASK_STACK_WORDS,
            NULL, MOTOR_CONTROL_TASK_PRIORITY, g_motorControlTaskStack,
            &g_motorControlTaskStorage) == NULL) {
        Tb6612Motor_StopCoast();
        return false;
    }
    return true;
}
