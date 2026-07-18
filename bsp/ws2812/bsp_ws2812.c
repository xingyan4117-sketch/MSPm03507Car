#include "bsp_ws2812.h"

#include "car_config.h"
#include "app_types.h"

#include <stdbool.h>
#include <stddef.h>

#define WS2812_LED_COUNT (3U)
#define WS2812_RESET_CYCLES (3000U)

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Ws2812Rgb;

static uint8_t g_ledCount;
static uint8_t g_brightness;
static uint8_t g_ledColors[WS2812_LED_COUNT];
static bool g_stateInitialized;

static void Ws2812_DelayCycles(uint32_t cycles)
{
    while (cycles-- > 0U) {
        __NOP();
    }
}

static void Ws2812_SendBit(bool one)
{
    DL_GPIO_setPins(CAR_WS2812_PORT, CAR_WS2812_PIN);
    Ws2812_DelayCycles(one ? 18U : 9U);
    DL_GPIO_clearPins(CAR_WS2812_PORT, CAR_WS2812_PIN);
    Ws2812_DelayCycles(one ? 22U : 31U);
}

static void Ws2812_SendByte(uint8_t value)
{
    uint8_t bit;

    for (bit = 0U; bit < 8U; bit++) {
        Ws2812_SendBit((value & (uint8_t)(0x80U >> bit)) != 0U);
    }
}

static uint8_t Ws2812_Scale(uint8_t value)
{
    return (uint8_t)(((uint16_t)value * g_brightness) / 100U);
}

static Ws2812Rgb Ws2812_Color(uint8_t color)
{
    static const Ws2812Rgb colors[APP_LED_COLOR_COUNT] = {
        { 255U, 0U, 0U }, { 0U, 255U, 0U }, { 0U, 0U, 255U },
        { 255U, 180U, 0U }, { 0U, 180U, 255U }, { 220U, 0U, 255U },
        { 255U, 255U, 255U }
    };

    return colors[color % APP_LED_COLOR_COUNT];
}

static void Ws2812_Render(void)
{
    uint32_t primask = __get_PRIMASK();
    uint8_t index;
    Ws2812Rgb rgb;

    __disable_irq();
    for (index = 0U; index < WS2812_LED_COUNT; index++) {
        if (index >= g_ledCount) {
            Ws2812_SendByte(0U);
            Ws2812_SendByte(0U);
            Ws2812_SendByte(0U);
        } else {
            rgb = Ws2812_Color(g_ledColors[index]);
            Ws2812_SendByte(Ws2812_Scale(rgb.green));
            Ws2812_SendByte(Ws2812_Scale(rgb.red));
            Ws2812_SendByte(Ws2812_Scale(rgb.blue));
        }
    }
    Ws2812_DelayCycles(WS2812_RESET_CYCLES);
    __set_PRIMASK(primask);
}

void BspWs2812_Init(void)
{
    DL_GPIO_initDigitalOutput(CAR_WS2812_IOMUX);
    DL_GPIO_clearPins(CAR_WS2812_PORT, CAR_WS2812_PIN);
    DL_GPIO_enableOutput(CAR_WS2812_PORT, CAR_WS2812_PIN);
    g_ledCount = 0U;
    g_brightness = 30U;
    g_ledColors[0] = APP_LED_COLOR_RED;
    g_ledColors[1] = APP_LED_COLOR_RED;
    g_ledColors[2] = APP_LED_COLOR_RED;
    g_stateInitialized = false;
    Ws2812_Render();
}

void BspWs2812_SetConfig(uint8_t count, uint8_t brightness,
    const uint8_t colors[3])
{
    uint8_t index;
    uint8_t nextCount = count > WS2812_LED_COUNT ? WS2812_LED_COUNT : count;
    uint8_t nextBrightness = brightness > 100U ? 100U : brightness;
    bool changed = !g_stateInitialized || (g_ledCount != nextCount) ||
        (g_brightness != nextBrightness);

    for (index = 0U; index < WS2812_LED_COUNT; index++) {
        uint8_t nextColor = (colors == NULL) ? APP_LED_COLOR_RED :
            (uint8_t)(colors[index] % APP_LED_COLOR_COUNT);
        if (g_ledColors[index] != nextColor) {
            changed = true;
            g_ledColors[index] = nextColor;
        }
    }
    if (!changed) return;
    g_stateInitialized = true;
    g_ledCount = nextCount;
    g_brightness = nextBrightness;
    Ws2812_Render();
}

void BspWs2812_Tick(uint32_t nowMs)
{
    (void) nowMs;
}

void BspWs2812_Off(void)
{
    g_ledCount = 0U;
    Ws2812_Render();
}
