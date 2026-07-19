#include "st7735_ui.h"

#include <string.h>

#include "bsp_spi.h"

#define ST7735_UI_BYTES_PER_LINE (ST7735_UI_DISPLAY_WIDTH * 2U)
#define ST7735_UI_GLYPH_WIDTH (5U)
#define ST7735_UI_GLYPH_HEIGHT (7U)
#define ST7735_UI_GLYPH_SCALE (2U)
#define ST7735_UI_GLYPH_ADVANCE (12U)
#define ST7735_UI_GLYPH_LINE_ADVANCE (16U)

#define ST7735_UI_BLACK_HIGH (0x00U)
#define ST7735_UI_BLACK_LOW (0x00U)
#define ST7735_UI_WHITE_HIGH (0xFFU)
#define ST7735_UI_WHITE_LOW (0xFFU)

static char g_lastFrame[ST7735_UI_FRAME_BYTES];
static uint8_t g_clearLine[ST7735_UI_BYTES_PER_LINE];
static uint8_t g_pixelFrame[ST7735_UI_RENDER_BYTES];
static bool g_initialClearComplete;
static uint8_t g_scrollStart;

static bool St7735Ui_WriteCommand(uint8_t command)
{
    return BspSpi_Write(false, &command, 1U);
}

static bool St7735Ui_WriteCommandData(uint8_t command,
    const uint8_t *data, size_t length)
{
    return St7735Ui_WriteCommand(command) &&
        BspSpi_Write(true, data, length);
}

static bool St7735Ui_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    const uint8_t column[] = {
        (uint8_t)(x0 >> 8), (uint8_t)x0, (uint8_t)(x1 >> 8), (uint8_t)x1
    };
    const uint8_t row[] = {
        (uint8_t)(y0 >> 8), (uint8_t)y0, (uint8_t)(y1 >> 8), (uint8_t)y1
    };

    return St7735Ui_WriteCommand(0x2AU) &&
        BspSpi_Write(true, column, sizeof(column)) &&
        St7735Ui_WriteCommand(0x2BU) &&
        BspSpi_Write(true, row, sizeof(row)) &&
        St7735Ui_WriteCommand(0x2CU);
}

static bool St7735Ui_ClearDisplay(void)
{
    uint32_t row;

    (void) memset(g_clearLine, 0, sizeof(g_clearLine));
    if (!St7735Ui_SetWindow(0U, 0U, ST7735_UI_DISPLAY_WIDTH - 1U,
            ST7735_UI_DISPLAY_HEIGHT - 1U)) {
        return false;
    }
    for (row = 0U; row < ST7735_UI_DISPLAY_HEIGHT; row++) {
        if (!BspSpi_Write(true, g_clearLine, sizeof(g_clearLine))) {
            return false;
        }
    }
    return true;
}

static const uint8_t *St7735Ui_Glyph(char character)
{
    static const uint8_t blank[5] = { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static const uint8_t glyph_0[5] = { 0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU };
    static const uint8_t glyph_1[5] = { 0x00U, 0x42U, 0x7FU, 0x40U, 0x00U };
    static const uint8_t glyph_2[5] = { 0x42U, 0x61U, 0x51U, 0x49U, 0x46U };
    static const uint8_t glyph_3[5] = { 0x21U, 0x41U, 0x45U, 0x4BU, 0x31U };
    static const uint8_t glyph_4[5] = { 0x18U, 0x14U, 0x12U, 0x7FU, 0x10U };
    static const uint8_t glyph_5[5] = { 0x27U, 0x45U, 0x45U, 0x45U, 0x39U };
    static const uint8_t glyph_6[5] = { 0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U };
    static const uint8_t glyph_7[5] = { 0x01U, 0x71U, 0x09U, 0x05U, 0x03U };
    static const uint8_t glyph_8[5] = { 0x36U, 0x49U, 0x49U, 0x49U, 0x36U };
    static const uint8_t glyph_9[5] = { 0x06U, 0x49U, 0x49U, 0x29U, 0x1EU };
    static const uint8_t glyph_a[5] = { 0x7EU, 0x11U, 0x11U, 0x11U, 0x7EU };
    static const uint8_t glyph_b[5] = { 0x7FU, 0x49U, 0x49U, 0x49U, 0x36U };
    static const uint8_t glyph_c[5] = { 0x3EU, 0x41U, 0x41U, 0x41U, 0x22U };
    static const uint8_t glyph_d[5] = { 0x7FU, 0x41U, 0x41U, 0x22U, 0x1CU };
    static const uint8_t glyph_e[5] = { 0x7FU, 0x49U, 0x49U, 0x49U, 0x41U };
    static const uint8_t glyph_f[5] = { 0x7FU, 0x09U, 0x09U, 0x09U, 0x01U };
    static const uint8_t glyph_g[5] = { 0x3EU, 0x41U, 0x49U, 0x49U, 0x7AU };
    static const uint8_t glyph_i[5] = { 0x00U, 0x41U, 0x7FU, 0x41U, 0x00U };
    static const uint8_t glyph_l[5] = { 0x7FU, 0x40U, 0x40U, 0x40U, 0x40U };
    static const uint8_t glyph_m[5] = { 0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU };
    static const uint8_t glyph_n[5] = { 0x7FU, 0x02U, 0x04U, 0x08U, 0x7FU };
    static const uint8_t glyph_o[5] = { 0x3EU, 0x41U, 0x41U, 0x41U, 0x3EU };
    static const uint8_t glyph_p[5] = { 0x7FU, 0x09U, 0x09U, 0x09U, 0x06U };
    static const uint8_t glyph_r[5] = { 0x7FU, 0x09U, 0x19U, 0x29U, 0x46U };
    static const uint8_t glyph_s[5] = { 0x46U, 0x49U, 0x49U, 0x49U, 0x31U };
    static const uint8_t glyph_t[5] = { 0x01U, 0x01U, 0x7FU, 0x01U, 0x01U };
    static const uint8_t glyph_u[5] = { 0x3FU, 0x40U, 0x40U, 0x40U, 0x3FU };
    static const uint8_t glyph_y[5] = { 0x07U, 0x08U, 0x70U, 0x08U, 0x07U };
    static const uint8_t glyph_colon[5] = { 0x00U, 0x36U, 0x36U, 0x00U, 0x00U };
    static const uint8_t glyph_minus[5] = { 0x08U, 0x08U, 0x08U, 0x08U, 0x08U };
    static const uint8_t glyph_slash[5] = { 0x40U, 0x30U, 0x0CU, 0x03U, 0x00U };
    static const uint8_t glyph_greater[5] = { 0x08U, 0x14U, 0x22U, 0x41U, 0x00U };
    static const uint8_t glyph_k[5] = { 0x7FU, 0x08U, 0x14U, 0x22U, 0x41U };
    static const uint8_t glyph_v[5] = { 0x07U, 0x38U, 0x40U, 0x38U, 0x07U };
    static const uint8_t glyph_z[5] = { 0x41U, 0x63U, 0x55U, 0x49U, 0x41U };

    switch (character) {
    case '0': return glyph_0;
    case '1': return glyph_1;
    case '2': return glyph_2;
    case '3': return glyph_3;
    case '4': return glyph_4;
    case '5': return glyph_5;
    case '6': return glyph_6;
    case '7': return glyph_7;
    case '8': return glyph_8;
    case '9': return glyph_9;
    case 'A': return glyph_a;
    case 'B': return glyph_b;
    case 'C': return glyph_c;
    case 'D': return glyph_d;
    case 'E': return glyph_e;
    case 'F': return glyph_f;
    case 'G': return glyph_g;
    case 'I': return glyph_i;
    case 'K': return glyph_k;
    case 'L': return glyph_l;
    case 'M': return glyph_m;
    case 'N': return glyph_n;
    case 'O': return glyph_o;
    case 'P': return glyph_p;
    case 'R': return glyph_r;
    case 'S': return glyph_s;
    case 'T': return glyph_t;
    case 'U': return glyph_u;
    case 'V': return glyph_v;
    case 'Y': return glyph_y;
    case 'Z': return glyph_z;
    case ':': return glyph_colon;
    case '-': return glyph_minus;
    case '/': return glyph_slash;
    case '>': return glyph_greater;
    default: return blank;
    }
}

static void St7735Ui_PutPixel(uint16_t x, uint16_t y, bool foreground)
{
    size_t index;

    if ((x >= ST7735_UI_DISPLAY_WIDTH) ||
        (y >= ST7735_UI_RENDER_LINE_HEIGHT)) {
        return;
    }
    index = ((size_t)y * ST7735_UI_DISPLAY_WIDTH + x) * 2U;
    g_pixelFrame[index] = foreground ? ST7735_UI_WHITE_HIGH : ST7735_UI_BLACK_HIGH;
    g_pixelFrame[index + 1U] = foreground ? ST7735_UI_WHITE_LOW : ST7735_UI_BLACK_LOW;
}

static void St7735Ui_DrawChar(uint16_t x, uint16_t y, char character)
{
    const uint8_t *glyph = St7735Ui_Glyph(character);
    uint16_t row;
    uint16_t column;

    for (row = 0U; row < ST7735_UI_GLYPH_HEIGHT; row++) {
        for (column = 0U; column < ST7735_UI_GLYPH_WIDTH; column++) {
            bool foreground = (glyph[column] & (uint8_t)(1U << row)) != 0U;
            uint16_t scaledX = (uint16_t)(x + column * ST7735_UI_GLYPH_SCALE);
            uint16_t scaledY = (uint16_t)(y + row * ST7735_UI_GLYPH_SCALE);
            uint8_t scaleRow;
            uint8_t scaleColumn;

            for (scaleRow = 0U; scaleRow < ST7735_UI_GLYPH_SCALE; scaleRow++) {
                for (scaleColumn = 0U; scaleColumn < ST7735_UI_GLYPH_SCALE; scaleColumn++) {
                    St7735Ui_PutPixel((uint16_t)(scaledX + scaleColumn),
                        (uint16_t)(scaledY + scaleRow), foreground);
                }
            }
        }
    }
}

static void St7735Ui_DrawLineText(const char *text)
{
    uint16_t x = 0U;
    size_t index;

    (void) memset(g_pixelFrame, 0, sizeof(g_pixelFrame));
    for (index = 0U; (text[index] != '\0') &&
            (x + ST7735_UI_GLYPH_WIDTH * ST7735_UI_GLYPH_SCALE <=
                ST7735_UI_DISPLAY_WIDTH); index++) {
        St7735Ui_DrawChar(x, 0U, text[index]);
        x = (uint16_t)(x + ST7735_UI_GLYPH_ADVANCE);
    }
}

static void St7735Ui_AppendChar(size_t *length, char character)
{
    if (*length + 1U < sizeof(g_lastFrame)) {
        g_lastFrame[*length] = character;
        (*length)++;
        g_lastFrame[*length] = '\0';
    }
}

static void St7735Ui_AppendText(size_t *length, const char *text)
{
    while (*text != '\0') {
        St7735Ui_AppendChar(length, *text++);
    }
}

static void St7735Ui_AppendUnsigned(size_t *length, uint32_t value)
{
    char digits[10];
    size_t digitCount = 0U;

    do {
        digits[digitCount++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while ((value != 0U) && (digitCount < sizeof(digits)));
    while (digitCount > 0U) {
        St7735Ui_AppendChar(length, digits[--digitCount]);
    }
}

static void St7735Ui_AppendSigned(size_t *length, int32_t value)
{
    uint32_t magnitude;

    if (value < 0) {
        St7735Ui_AppendChar(length, '-');
        magnitude = (uint32_t)(-(value + 1)) + 1U;
    } else {
        magnitude = (uint32_t)value;
    }
    St7735Ui_AppendUnsigned(length, magnitude);
}

static const char *St7735Ui_StateName(AppState state)
{
    switch (state) {
    case APP_STATE_INIT: return "INIT";
    case APP_STATE_READY: return "READY";
    case APP_STATE_RUN: return "RUN";
    case APP_STATE_ESTOP: return "ESTOP";
    case APP_STATE_FAULT: return "FAULT";
    default: return "FAULT";
    }
}

static const char *St7735Ui_BuzzerModeName(uint8_t mode)
{
    switch (mode) {
    case APP_BUZZER_MODE_CONT: return "CONT";
    case APP_BUZZER_MODE_SLOW: return "SLOW";
    case APP_BUZZER_MODE_FAST: return "FAST";
    default: return "OFF";
    }
}

static const char *St7735Ui_ColorName(uint8_t color)
{
    static const char *const names[APP_LED_COLOR_COUNT] = {
        "RED", "GRN", "BLU", "YEL", "CYN", "MAG", "WHT"
    };
    return names[color % APP_LED_COLOR_COUNT];
}

static void St7735Ui_AppendMenuLine(size_t *length, bool selected,
    const char *label, const char *value)
{
    St7735Ui_AppendChar(length, selected ? '>' : ' ');
    St7735Ui_AppendText(length, label);
    if (value[0] != '\0') {
        St7735Ui_AppendChar(length, ':');
        St7735Ui_AppendText(length, value);
    }
    St7735Ui_AppendChar(length, '\n');
}

static void St7735Ui_AppendModeLine(size_t *length, bool editMode)
{
    St7735Ui_AppendText(length, "MODE:");
    St7735Ui_AppendText(length, editMode ? "EDIT" : "SEL");
}

static void St7735Ui_FormatStatus(const AppMotorStatus *status)
{
    AppMotorStatus faultStatus = { .state = APP_STATE_FAULT };
    size_t length = 0U;
    uint8_t selected;
    uint8_t start;

    if (status == NULL) {
        status = &faultStatus;
    }
    g_lastFrame[0] = '\0';
    g_scrollStart = 0U;
    if (status->menuLevel == 0U) {
        St7735Ui_AppendMenuLine(&length, status->menuItem == APP_MENU_MOTOR,
            "MOTOR", "");
        St7735Ui_AppendMenuLine(&length, status->menuItem == APP_MENU_BUZZER,
            "BUZZER", "");
        St7735Ui_AppendMenuLine(&length, status->menuItem == APP_MENU_LED,
            "LED", "");
        St7735Ui_AppendModeLine(&length, status->editMode != 0U);
        return;
    }

    if (status->menuItem == APP_MENU_MOTOR) {
        selected = status->menuSubItem;
        St7735Ui_AppendMenuLine(&length, selected == APP_MOTOR_SUBITEM_RUN,
            "RUN", status->state == APP_STATE_RUN ? "ON" : "OFF");
        St7735Ui_AppendChar(&length, selected == APP_MOTOR_SUBITEM_SPEED ? '>' : ' ');
        St7735Ui_AppendText(&length, "SPEED:");
        St7735Ui_AppendSigned(&length, status->targetRpmA);
        St7735Ui_AppendChar(&length, '\n');
        St7735Ui_AppendMenuLine(&length, selected == APP_MOTOR_SUBITEM_BACK,
            "BACK", "");
        St7735Ui_AppendModeLine(&length, status->editMode != 0U);
        return;
    }

    if (status->menuItem == APP_MENU_BUZZER) {
        selected = status->menuSubItem;
        St7735Ui_AppendMenuLine(&length, selected == APP_BUZZER_SUBITEM_MODE,
            "MODE", St7735Ui_BuzzerModeName(status->buzzerMode));
        St7735Ui_AppendChar(&length, selected == APP_BUZZER_SUBITEM_VOLUME ? '>' : ' ');
        St7735Ui_AppendText(&length, "VOL:");
        St7735Ui_AppendUnsigned(&length, status->buzzerVolume);
        St7735Ui_AppendChar(&length, '\n');
        St7735Ui_AppendMenuLine(&length, selected == APP_BUZZER_SUBITEM_BACK,
            "BACK", "");
        St7735Ui_AppendModeLine(&length, status->editMode != 0U);
        return;
    }

    selected = status->menuSubItem;
    St7735Ui_AppendChar(&length, selected == APP_LED_SUBITEM_COUNT ? '>' : ' ');
    St7735Ui_AppendText(&length, "COUNT:");
    St7735Ui_AppendUnsigned(&length, status->ledCount);
    St7735Ui_AppendChar(&length, '\n');
    St7735Ui_AppendChar(&length, selected == APP_LED_SUBITEM_BRIGHTNESS ? '>' : ' ');
    St7735Ui_AppendText(&length, "BRI:");
    St7735Ui_AppendUnsigned(&length, status->ledBrightness);
    St7735Ui_AppendChar(&length, '\n');
    St7735Ui_AppendMenuLine(&length, selected == APP_LED_SUBITEM_LED1,
        "LED1", St7735Ui_ColorName(status->ledColor[0]));
    St7735Ui_AppendMenuLine(&length, selected == APP_LED_SUBITEM_LED2,
        "LED2", St7735Ui_ColorName(status->ledColor[1]));
    St7735Ui_AppendMenuLine(&length, selected == APP_LED_SUBITEM_LED3,
        "LED3", St7735Ui_ColorName(status->ledColor[2]));
    St7735Ui_AppendMenuLine(&length, selected == APP_LED_SUBITEM_BACK,
        "BACK", "");
    St7735Ui_AppendModeLine(&length, status->editMode != 0U);
    start = 0U;
    g_scrollStart = start;
}

static bool St7735Ui_WriteRenderLine(uint16_t y)
{
    return St7735Ui_SetWindow(0U, y, ST7735_UI_DISPLAY_WIDTH - 1U,
        (uint16_t)(y + ST7735_UI_RENDER_LINE_HEIGHT - 1U)) &&
        BspSpi_Write(true, g_pixelFrame, sizeof(g_pixelFrame));
}

static bool St7735Ui_WriteAllLines(void)
{
    size_t cursor = 0U;
    uint16_t line;
    uint16_t lineCount = (uint16_t)(ST7735_UI_RENDER_HEIGHT /
        ST7735_UI_RENDER_LINE_HEIGHT);
    char lineText[ST7735_UI_FRAME_BYTES];

    for (line = 0U; line < lineCount; line++) {
        size_t length = 0U;

        while ((g_lastFrame[cursor] != '\0') &&
            (g_lastFrame[cursor] != '\n') &&
            (length + 1U < sizeof(lineText))) {
            lineText[length++] = g_lastFrame[cursor++];
        }
        lineText[length] = '\0';
        if (g_lastFrame[cursor] == '\n') {
            cursor++;
        }
        St7735Ui_DrawLineText(lineText);
        if (!St7735Ui_WriteRenderLine((uint16_t)(line *
                ST7735_UI_RENDER_LINE_HEIGHT))) {
            return false;
        }
    }
    return true;
}

void St7735Ui_Init(void)
{
    static const uint8_t frameRateNormal[] = { 0x01U, 0x2CU, 0x2DU };
    static const uint8_t frameRateIdle[] = { 0x01U, 0x2CU, 0x2DU };
    static const uint8_t frameRatePartial[] = {
        0x01U, 0x2CU, 0x2DU, 0x01U, 0x2CU, 0x2DU
    };
    static const uint8_t inversion[] = { 0x07U };
    static const uint8_t power1[] = { 0xA2U, 0x02U, 0x84U };
    static const uint8_t power2[] = { 0xC5U };
    static const uint8_t power3[] = { 0x0AU, 0x00U };
    static const uint8_t power4[] = { 0x8AU, 0x2AU };
    static const uint8_t power5[] = { 0x8AU, 0xEEU };
    static const uint8_t vcom[] = { 0x0EU };
    static const uint8_t madctl[] = { 0xC0U };
    static const uint8_t gammaPositive[] = {
        0x0FU, 0x1AU, 0x0FU, 0x18U, 0x2FU, 0x28U, 0x20U, 0x22U,
        0x1FU, 0x1BU, 0x23U, 0x37U, 0x00U, 0x07U, 0x02U, 0x10U
    };
    static const uint8_t gammaNegative[] = {
        0x0FU, 0x1BU, 0x0FU, 0x17U, 0x33U, 0x2CU, 0x29U, 0x2EU,
        0x30U, 0x30U, 0x39U, 0x3FU, 0x00U, 0x07U, 0x03U, 0x10U
    };
    static const uint8_t columnWindow[] = { 0x00U, 0x00U, 0x00U, 0x7FU };
    static const uint8_t rowWindow[] = { 0x00U, 0x00U, 0x00U, 0x9FU };
    static const uint8_t interfaceControl[] = { 0x01U };
    static const uint8_t displayFunction[] = { 0x00U };
    static const uint8_t colorMode[] = { 0x05U };
    bool initOk;

    BspSpi_Init();
    BspSpi_SetBacklight(false);
    BspSpi_SetReset(true);
    BspSpi_DelayMs(100U);
    BspSpi_SetReset(false);
    BspSpi_DelayMs(50U);
    initOk = St7735Ui_WriteCommand(0x01U);
    BspSpi_DelayMs(150U);
    initOk = initOk && St7735Ui_WriteCommand(0x11U);
    BspSpi_DelayMs(120U);
    initOk = initOk && St7735Ui_WriteCommandData(0xB1U,
        frameRateNormal, sizeof(frameRateNormal));
    initOk = initOk && St7735Ui_WriteCommandData(0xB2U,
        frameRateIdle, sizeof(frameRateIdle));
    initOk = initOk && St7735Ui_WriteCommandData(0xB3U,
        frameRatePartial, sizeof(frameRatePartial));
    initOk = initOk && St7735Ui_WriteCommandData(0xB4U,
        inversion, sizeof(inversion));
    initOk = initOk && St7735Ui_WriteCommandData(0xC0U,
        power1, sizeof(power1));
    initOk = initOk && St7735Ui_WriteCommandData(0xC1U,
        power2, sizeof(power2));
    initOk = initOk && St7735Ui_WriteCommandData(0xC2U,
        power3, sizeof(power3));
    initOk = initOk && St7735Ui_WriteCommandData(0xC3U,
        power4, sizeof(power4));
    initOk = initOk && St7735Ui_WriteCommandData(0xC4U,
        power5, sizeof(power5));
    initOk = initOk && St7735Ui_WriteCommandData(0xC5U,
        vcom, sizeof(vcom));
    initOk = initOk && St7735Ui_WriteCommandData(0x36U,
        madctl, sizeof(madctl));
    initOk = initOk && St7735Ui_WriteCommandData(0xE0U,
        gammaPositive, sizeof(gammaPositive));
    initOk = initOk && St7735Ui_WriteCommandData(0xE1U,
        gammaNegative, sizeof(gammaNegative));
    initOk = initOk && St7735Ui_WriteCommandData(0x2AU,
        columnWindow, sizeof(columnWindow));
    initOk = initOk && St7735Ui_WriteCommandData(0x2BU,
        rowWindow, sizeof(rowWindow));
    initOk = initOk && St7735Ui_WriteCommandData(0xF0U,
        interfaceControl, sizeof(interfaceControl));
    initOk = initOk && St7735Ui_WriteCommandData(0xF6U,
        displayFunction, sizeof(displayFunction));
    initOk = initOk && St7735Ui_WriteCommandData(0x3AU,
        colorMode, sizeof(colorMode));
    initOk = initOk && St7735Ui_WriteCommand(0x29U);
    if (initOk && !g_initialClearComplete) {
        initOk = St7735Ui_ClearDisplay();
        g_initialClearComplete = initOk;
    }
    if (initOk) {
        BspSpi_SetBacklight(true);
    }
}

void St7735Ui_Clear(void)
{
    g_lastFrame[0] = '\0';
    (void) St7735Ui_ClearDisplay();
}

void St7735Ui_RenderStatus(const AppMotorStatus *status)
{
    St7735Ui_FormatStatus(status);
    (void) St7735Ui_WriteAllLines();
}

const char *St7735Ui_GetLastFrame(void)
{
    return g_lastFrame;
}
