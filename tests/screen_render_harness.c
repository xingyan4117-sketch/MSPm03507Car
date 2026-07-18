#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "app_types.h"
#include "bsp_spi.h"
#include "st7735_ui.h"

#define SCREEN_RENDER_BYTES (ST7735_UI_RENDER_BYTES)

static void AssertStatusWindow(void)
{
    assert(ScreenStub_WriteCount() == 6U);
    assert(!ScreenStub_WriteDataMode(0U));
    assert(ScreenStub_WriteFirstByte(0U) == 0x2AU);
    assert(ScreenStub_WriteDataMode(1U));
    assert(ScreenStub_WriteLength(1U) == 4U);
    assert(!ScreenStub_WriteDataMode(2U));
    assert(ScreenStub_WriteFirstByte(2U) == 0x2BU);
    assert(ScreenStub_WriteDataMode(3U));
    assert(ScreenStub_WriteLength(3U) == 4U);
    assert(!ScreenStub_WriteDataMode(4U));
    assert(ScreenStub_WriteFirstByte(4U) == 0x2CU);
    assert(ScreenStub_WriteDataMode(5U));
    assert(ScreenStub_WriteLength(5U) == SCREEN_RENDER_BYTES);
}

static void AssertVisibleGlyphPixels(void)
{
    size_t index;
    size_t whiteBytes = 0U;

    assert(ScreenStub_LastDataLength() == SCREEN_RENDER_BYTES);
    for (index = 0U; index < ScreenStub_LastDataLength(); index++) {
        if (ScreenStub_LastDataByte(index) == 0xFFU) {
            whiteBytes++;
        }
    }
    assert(whiteBytes > 400U);
    assert(whiteBytes < SCREEN_RENDER_BYTES);
}

int main(void)
{
    AppMotorStatus status = {
        .state = APP_STATE_RUN,
        .commandTimedOut = 0U,
        .reserved = 0U,
        .menuItem = APP_MENU_MOTOR,
        .menuLevel = 0U,
        .menuSubItem = APP_MOTOR_SUBITEM_RUN,
        .editMode = 0U,
        .commandSequence = 3U,
        .observedAtMs = 100U,
        .targetRpmA = 100,
        .rpmA = 100,
        .rpmB = 120,
        .rpmC = -80,
        .rpmD = 60,
        .dutyA = 10,
        .dutyB = 20,
        .dutyC = -15,
        .dutyD = -5,
        .invalidTransitionsA = 3U,
        .invalidTransitionsB = 7U,
        .invalidTransitionsC = 9U,
        .invalidTransitionsD = 11U,
        .buzzerMode = APP_BUZZER_MODE_CONT,
        .buzzerVolume = 60U,
        .ledCount = 3U,
        .ledBrightness = 30U,
        .ledColor = { APP_LED_COLOR_GREEN, APP_LED_COLOR_BLUE, APP_LED_COLOR_RED }
    };
    const char *frame;

    ScreenStub_Reset();
    ScreenStub_FailWriteCall(1U);
    St7735Ui_Init();
    assert(!ScreenStub_BacklightEnabled());

    ScreenStub_Reset();
    St7735Ui_Init();
    assert(ScreenStub_BacklightEnabled());

    ScreenStub_Reset();
    St7735Ui_Init();
    assert(ScreenStub_BacklightEnabled());
    assert(ScreenStub_ResetAssertCalls() == 1U);
    assert(ScreenStub_ResetReleaseCalls() == 1U);
    assert(ScreenStub_DelayCalls() >= 3U);
    assert(ScreenStub_WriteCount() >= 12U);
    assert(!ScreenStub_WriteDataMode(0U));
    assert(ScreenStub_WriteFirstByte(0U) == 0x01U);
    assert(!ScreenStub_WriteDataMode(1U));
    assert(ScreenStub_WriteFirstByte(1U) == 0x11U);

    ScreenStub_Reset();
    St7735Ui_RenderStatus(&status);
    AssertStatusWindow();
    AssertVisibleGlyphPixels();
    frame = St7735Ui_GetLastFrame();
    assert(frame != NULL);
    assert(strstr(frame, ">MOTOR") != NULL);
    assert(strstr(frame, " BUZZER") != NULL);
    assert(strstr(frame, " LED") != NULL);

    printf("PASS: ST7735S fixed-font screen rendering.\n");
    return 0;
}
