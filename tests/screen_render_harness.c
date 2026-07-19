#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "app_types.h"
#include "bsp_spi.h"
#include "st7735_ui.h"

#define SCREEN_RENDER_BYTES (ST7735_UI_RENDER_BYTES)

static void AssertStatusWindow(void)
{
    size_t base = 54U;

    assert(ScreenStub_WriteCount() == 60U);
    assert(!ScreenStub_WriteDataMode(base));
    assert(ScreenStub_WriteFirstByte(base) == 0x2AU);
    assert(ScreenStub_WriteDataMode(base + 1U));
    assert(ScreenStub_WriteLength(base + 1U) == 4U);
    assert(!ScreenStub_WriteDataMode(base + 2U));
    assert(ScreenStub_WriteFirstByte(base + 2U) == 0x2BU);
    assert(ScreenStub_WriteDataMode(base + 3U));
    assert(ScreenStub_WriteLength(base + 3U) == 4U);
    assert(!ScreenStub_WriteDataMode(base + 4U));
    assert(ScreenStub_WriteFirstByte(base + 4U) == 0x2CU);
    assert(ScreenStub_WriteDataMode(base + 5U));
    assert(ScreenStub_WriteLength(base + 5U) == SCREEN_RENDER_BYTES);
}

static void AssertVisibleGlyphPixels(void)
{
    assert(ScreenStub_LastDataLength() == SCREEN_RENDER_BYTES);
    assert(ScreenStub_MaxWhiteBytes() > 20U);
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

    status.menuItem = APP_MENU_LED;
    status.menuLevel = 1U;
    status.menuSubItem = APP_LED_SUBITEM_LED1;
    status.ledColor[0] = APP_LED_COLOR_WHITE;
    ScreenStub_Reset();
    St7735Ui_RenderStatus(&status);
    frame = St7735Ui_GetLastFrame();
    assert(frame != NULL);
    assert(strstr(frame, ">LED1:WHT") != NULL);

    printf("PASS: ST7735S fixed-font screen rendering.\n");
    return 0;
}
