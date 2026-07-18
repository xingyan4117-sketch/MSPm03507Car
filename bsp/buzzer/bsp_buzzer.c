#include "bsp_buzzer.h"

#include "car_config.h"
#include "app_types.h"

#define BUZZER_GATE_PERIOD_MS (100U)
#define BUZZER_SLOW_PERIOD_MS (500U)
#define BUZZER_FAST_PERIOD_MS (100U)

static uint8_t g_mode;
static uint8_t g_volume;

static void BspBuzzer_Write(bool enabled)
{
    if (enabled) {
        DL_GPIO_setPins(CAR_BUZZER_PORT, CAR_BUZZER_PIN);
    } else {
        DL_GPIO_clearPins(CAR_BUZZER_PORT, CAR_BUZZER_PIN);
    }
}

void BspBuzzer_Init(void)
{
    g_mode = APP_BUZZER_MODE_OFF;
    g_volume = 0U;
    BspBuzzer_Write(false);
    DL_GPIO_enableOutput(CAR_BUZZER_PORT, CAR_BUZZER_PIN);
}

void BspBuzzer_SetConfig(uint8_t mode, uint8_t volume, uint32_t nowMs)
{
    g_mode = mode > APP_BUZZER_MODE_FAST ? APP_BUZZER_MODE_OFF : mode;
    g_volume = volume > APP_BUZZER_VOLUME_MAX ? APP_BUZZER_VOLUME_MAX : volume;
}

void BspBuzzer_Tick(uint32_t nowMs)
{
    uint32_t periodMs;
    uint32_t phaseMs;
    uint32_t onTimeMs;

    if (g_mode == APP_BUZZER_MODE_OFF || g_volume == 0U) {
        BspBuzzer_Write(false);
        return;
    }
    periodMs = (g_mode == APP_BUZZER_MODE_SLOW) ? BUZZER_SLOW_PERIOD_MS :
        (g_mode == APP_BUZZER_MODE_FAST) ? BUZZER_FAST_PERIOD_MS :
        BUZZER_GATE_PERIOD_MS;
    phaseMs = nowMs % periodMs;
    onTimeMs = (periodMs * g_volume) / 100U;
    BspBuzzer_Write(phaseMs < onTimeMs);
}

void BspBuzzer_Stop(void)
{
    g_mode = APP_BUZZER_MODE_OFF;
    g_volume = 0U;
    BspBuzzer_Write(false);
}
