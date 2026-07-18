#include "bsp_gpio.h"

#include "car_config.h"

#define BSP_GPIO_KEY_DEBOUNCE_SAMPLES (3U)

typedef struct {
    bool candidate;
    bool stable;
    uint8_t samples;
} BspGpioDebounceState;

static BspGpioDebounceState g_keyDebounce;
static BspGpioDebounceState g_key1Debounce;
static BspGpioDebounceState g_key2Debounce;
static BspGpioDebounceState g_key3Debounce;

static bool BspGpio_ReadRawPressed(GPIO_Regs *port, uint32_t pin)
{
    return DL_GPIO_readPins(port, pin) == 0U;
}

static void BspGpio_InitDebounce(BspGpioDebounceState *state, GPIO_Regs *port,
    uint32_t pin)
{
    state->candidate = BspGpio_ReadRawPressed(port, pin);
    state->stable = state->candidate;
    state->samples = 0U;
}

static bool BspGpio_ReadDebounced(BspGpioDebounceState *state,
    GPIO_Regs *port, uint32_t pin)
{
    bool rawPressed = BspGpio_ReadRawPressed(port, pin);

    if (rawPressed != state->candidate) {
        state->candidate = rawPressed;
        state->samples = 1U;
    } else if (state->samples < BSP_GPIO_KEY_DEBOUNCE_SAMPLES) {
        state->samples++;
    }
    if (state->samples >= BSP_GPIO_KEY_DEBOUNCE_SAMPLES) {
        state->stable = state->candidate;
    }
    return state->stable;
}

void BspGpio_Init(void)
{
    BspGpio_InitDebounce(&g_keyDebounce, CAR_KEY_PORT, CAR_KEY_PIN);
    BspGpio_InitDebounce(&g_key1Debounce, CAR_KEY1_PORT, CAR_KEY1_PIN);
    BspGpio_InitDebounce(&g_key2Debounce, CAR_KEY2_PORT, CAR_KEY2_PIN);
    BspGpio_InitDebounce(&g_key3Debounce, CAR_KEY3_PORT, CAR_KEY3_PIN);
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_A, 0);
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_B, 0);
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_C, 0);
    BspGpio_SetMotorDirection(BSP_GPIO_MOTOR_D, 0);
    BspGpio_SetHeartbeat(false);
    BspGpio_DisplaySetChipSelect(false);
    BspGpio_DisplaySetDataCommand(true);
    BspGpio_DisplaySetReset(false);
    BspGpio_DisplaySetBacklight(false);
}

void BspGpio_SetMotorDirection(BspGpioMotor motor, int32_t signedDuty)
{
    GPIO_Regs *port;
    uint32_t pinA;
    uint32_t pinB;

    if (motor == BSP_GPIO_MOTOR_A) {
        port = CAR_MOTOR_A_DIRECTION_PORT;
        pinA = CAR_MOTOR_A_DIRECTION_A_PIN;
        pinB = CAR_MOTOR_A_DIRECTION_B_PIN;
    } else if (motor == BSP_GPIO_MOTOR_B) {
        port = CAR_MOTOR_B_DIRECTION_PORT;
        pinA = CAR_MOTOR_B_DIRECTION_A_PIN;
        pinB = CAR_MOTOR_B_DIRECTION_B_PIN;
    } else if (motor == BSP_GPIO_MOTOR_C) {
        port = CAR_MOTOR_C_DIRECTION_PORT;
        pinA = CAR_MOTOR_C_DIRECTION_A_PIN;
        pinB = CAR_MOTOR_C_DIRECTION_B_PIN;
    } else {
        port = CAR_MOTOR_D_DIRECTION_PORT;
        pinA = CAR_MOTOR_D_DIRECTION_A_PIN;
        pinB = CAR_MOTOR_D_DIRECTION_B_PIN;
    }

    if (signedDuty > 0) {
        DL_GPIO_setPins(port, pinA);
        DL_GPIO_clearPins(port, pinB);
    } else if (signedDuty < 0) {
        DL_GPIO_clearPins(port, pinA);
        DL_GPIO_setPins(port, pinB);
    } else {
        DL_GPIO_clearPins(port, pinA | pinB);
    }
}

bool BspGpio_ReadKeyDebounced(void)
{
    return BspGpio_ReadDebounced(&g_keyDebounce, CAR_KEY_PORT, CAR_KEY_PIN);
}

bool BspGpio_ReadKey1Debounced(void)
{
    return BspGpio_ReadDebounced(&g_key1Debounce, CAR_KEY1_PORT, CAR_KEY1_PIN);
}

bool BspGpio_ReadKey2Debounced(void)
{
    return BspGpio_ReadDebounced(&g_key2Debounce, CAR_KEY2_PORT, CAR_KEY2_PIN);
}

bool BspGpio_ReadKey3Debounced(void)
{
    return BspGpio_ReadDebounced(&g_key3Debounce, CAR_KEY3_PORT, CAR_KEY3_PIN);
}

void BspGpio_SetHeartbeat(bool enabled)
{
    if (enabled) {
        DL_GPIO_setPins(CAR_HEARTBEAT_PORT, CAR_HEARTBEAT_PIN);
    } else {
        DL_GPIO_clearPins(CAR_HEARTBEAT_PORT, CAR_HEARTBEAT_PIN);
    }
}

void BspGpio_ToggleHeartbeat(void)
{
    DL_GPIO_togglePins(CAR_HEARTBEAT_PORT, CAR_HEARTBEAT_PIN);
}

void BspGpio_DisplaySetChipSelect(bool asserted)
{
    if (asserted) {
        DL_GPIO_clearPins(CAR_ST7735_CS_PORT, CAR_ST7735_CS_PIN);
    } else {
        DL_GPIO_setPins(CAR_ST7735_CS_PORT, CAR_ST7735_CS_PIN);
    }
}

void BspGpio_DisplaySetDataCommand(bool dataMode)
{
    if (dataMode) {
        DL_GPIO_setPins(CAR_ST7735_DC_PORT, CAR_ST7735_DC_PIN);
    } else {
        DL_GPIO_clearPins(CAR_ST7735_DC_PORT, CAR_ST7735_DC_PIN);
    }
}

void BspGpio_DisplaySetReset(bool asserted)
{
    if (asserted) {
        DL_GPIO_clearPins(CAR_ST7735_RES_PORT, CAR_ST7735_RES_PIN);
    } else {
        DL_GPIO_setPins(CAR_ST7735_RES_PORT, CAR_ST7735_RES_PIN);
    }
}

void BspGpio_DisplaySetBacklight(bool enabled)
{
    if (enabled) {
        DL_GPIO_setPins(CAR_ST7735_BLK_PORT, CAR_ST7735_BLK_PIN);
    } else {
        DL_GPIO_clearPins(CAR_ST7735_BLK_PORT, CAR_ST7735_BLK_PIN);
    }
}
