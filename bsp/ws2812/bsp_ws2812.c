#include "bsp_ws2812.h"

#include "car_config.h"
#include "app_types.h"

#include <stdbool.h>
#include <stddef.h>

#define WS2812_LED_COUNT (3U)
#define WS2812_BITS_PER_LED (24U)
#define WS2812_FRAME_BITS (WS2812_LED_COUNT * WS2812_BITS_PER_LED)
#define WS2812_PWM_PERIOD_TICKS (40U) /* 32 MHz / 800 kHz */
#define WS2812_T0H_TICKS (10U)       /* 0.3125 us */
#define WS2812_T1H_TICKS (29U)       /* 0.90625 us */
#define WS2812_T0_COMPARE_TICKS \
    (WS2812_PWM_PERIOD_TICKS - WS2812_T0H_TICKS)
#define WS2812_T1_COMPARE_TICKS \
    (WS2812_PWM_PERIOD_TICKS - WS2812_T1H_TICKS)
#define WS2812_RESET_CYCLES (CPUCLK_FREQ / 4000U) /* 250 us */
#define WS2812_DMA_CHANNEL (0U)
#define WS2812_EVENT_CHANNEL (15U)

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Ws2812Rgb;

static uint8_t g_ledCount;
static uint8_t g_brightness;
static uint8_t g_ledColors[WS2812_LED_COUNT];
static bool g_stateInitialized;
static uint32_t g_pwmFrame[WS2812_FRAME_BITS + 1U];

static const DL_TimerG_ClockConfig g_ws2812ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U,
};

static const DL_TimerG_PWMConfig g_ws2812PwmConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = WS2812_PWM_PERIOD_TICKS,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_STOP,
};

static const DL_DMA_Config g_ws2812DmaConfig = {
    .transferMode = DL_DMA_SINGLE_TRANSFER_MODE,
    .extendedMode = DL_DMA_NORMAL_MODE,
    .destIncrement = DL_DMA_ADDR_UNCHANGED,
    .srcIncrement = DL_DMA_ADDR_INCREMENT,
    .destWidth = DL_DMA_WIDTH_WORD,
    .srcWidth = DL_DMA_WIDTH_WORD,
    .trigger = DMA_GENERIC_SUB0_TRIG,
    .triggerType = DL_DMA_TRIGGER_TYPE_EXTERNAL,
};

static void Ws2812_AppendByte(uint8_t value, uint8_t *bitIndex)
{
    uint8_t mask;

    for (mask = 0x80U; mask != 0U; mask >>= 1U) {
        g_pwmFrame[*bitIndex] = ((value & mask) != 0U) ?
            WS2812_T1_COMPARE_TICKS : WS2812_T0_COMPARE_TICKS;
        (*bitIndex)++;
    }
}

static void Ws2812_OutputFrame(void)
{
    uint32_t timeout = CPUCLK_FREQ / 100U;

    DL_TimerG_stopCounter(TIMG6);
    DL_TimerG_setTimerCount(TIMG6, WS2812_PWM_PERIOD_TICKS);
    DL_TimerG_setCaptureCompareValue(
        TIMG6, g_pwmFrame[0], DL_TIMER_CC_0_INDEX);
    DL_DMA_disableChannel(DMA, WS2812_DMA_CHANNEL);
    DL_DMA_setSrcAddr(DMA, WS2812_DMA_CHANNEL, (uint32_t) &g_pwmFrame[1]);
    DL_DMA_setDestAddr(DMA, WS2812_DMA_CHANNEL,
        (uint32_t) &TIMG6->COUNTERREGS.CC_01[0]);
    DL_DMA_setTransferSize(DMA, WS2812_DMA_CHANNEL, WS2812_FRAME_BITS);

    DL_GPIO_initPeripheralOutputFunction(
        CAR_WS2812_IOMUX, IOMUX_PINCM4_PF_TIMG6_CCP0);
    DL_GPIO_enableOutput(CAR_WS2812_PORT, CAR_WS2812_PIN);
    DL_DMA_enableChannel(DMA, WS2812_DMA_CHANNEL);
    DL_TimerG_startCounter(TIMG6);

    while ((DL_DMA_getTransferSize(DMA, WS2812_DMA_CHANNEL) != 0U) &&
        (timeout-- != 0U)) {
    }

    DL_TimerG_stopCounter(TIMG6);
    DL_GPIO_initDigitalOutput(CAR_WS2812_IOMUX);
    DL_GPIO_clearPins(CAR_WS2812_PORT, CAR_WS2812_PIN);
    delay_cycles(WS2812_RESET_CYCLES);
}

static uint8_t Ws2812_Scale(uint8_t value)
{
    return (uint8_t)(((uint16_t)value * g_brightness) / 100U);
}

static Ws2812Rgb Ws2812_Color(uint8_t color)
{
    static const Ws2812Rgb colors[APP_LED_COLOR_COUNT] = {
        { 255U, 0U, 0U }, { 0U, 255U, 0U }, { 0U, 0U, 255U },
        { 255U, 255U, 255U }
    };

    return colors[color % APP_LED_COLOR_COUNT];
}

static void Ws2812_Render(void)
{
    uint8_t index;
    uint8_t bitIndex = 0U;
    Ws2812Rgb rgb;

    for (index = 0U; index < WS2812_LED_COUNT; index++) {
        if (index >= g_ledCount) {
            Ws2812_AppendByte(0U, &bitIndex);
            Ws2812_AppendByte(0U, &bitIndex);
            Ws2812_AppendByte(0U, &bitIndex);
        } else {
            rgb = Ws2812_Color(g_ledColors[index]);
            Ws2812_AppendByte(Ws2812_Scale(rgb.green), &bitIndex);
            Ws2812_AppendByte(Ws2812_Scale(rgb.red), &bitIndex);
            Ws2812_AppendByte(Ws2812_Scale(rgb.blue), &bitIndex);
        }
    }
    g_pwmFrame[WS2812_FRAME_BITS] = WS2812_PWM_PERIOD_TICKS;
    Ws2812_OutputFrame();
}

void BspWs2812_Init(void)
{
    DL_TimerG_reset(TIMG6);
    DL_TimerG_enablePower(TIMG6);
    delay_cycles(POWER_STARTUP_DELAY);
    DL_TimerG_setClockConfig(TIMG6,
        (DL_TimerG_ClockConfig *) &g_ws2812ClockConfig);
    DL_TimerG_initPWMMode(TIMG6,
        (DL_TimerG_PWMConfig *) &g_ws2812PwmConfig);
    DL_TimerG_setCounterControl(TIMG6, DL_TIMER_CZC_CCCTL0_ZCOND,
        DL_TIMER_CAC_CCCTL0_ACOND, DL_TIMER_CLC_CCCTL0_LCOND);
    DL_TimerG_setCaptureCompareOutCtl(TIMG6, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
        DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
        DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptCompUpdateMethod(TIMG6,
        DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE,
        DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCCPDirection(TIMG6, DL_TIMER_CC0_OUTPUT);
    DL_TimerG_enableEvent(
        TIMG6, DL_TIMERG_EVENT_ROUTE_1, DL_TIMERG_EVENT_ZERO_EVENT);
    DL_TimerG_setPublisherChanID(TIMG6,
        DL_TIMERG_PUBLISHER_INDEX_0, WS2812_EVENT_CHANNEL);
    DL_TimerG_enableClock(TIMG6);

    DL_DMA_setSubscriberChanID(
        DMA, DL_DMA_SUBSCRIBER_INDEX_0, WS2812_EVENT_CHANNEL);
    DL_DMA_initChannel(
        DMA, WS2812_DMA_CHANNEL, (DL_DMA_Config *) &g_ws2812DmaConfig);

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
