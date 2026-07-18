#include "bsp_encoder.h"

#include "car_config.h"

typedef struct {
    volatile int32_t delta;
    volatile uint32_t invalidTransitions;
    uint8_t state;
} BspEncoderState;

static BspEncoderState g_encoderA;
static BspEncoderState g_encoderB;
static BspEncoderState g_encoderC;
static BspEncoderState g_encoderD;

static uint32_t BspEncoder_BuildPolarityMask(uint32_t pins, uint8_t firstPin)
{
    uint32_t polarity = 0U;

    for (uint8_t pin = firstPin; pin < (uint8_t) (firstPin + 16U); ++pin) {
        uint32_t pinMask = (1UL << pin);

        if ((pins & pinMask) != 0U) {
            polarity |= (0x03UL << ((pin - firstPin) * 2U));
        }
    }

    return polarity;
}

static uint8_t BspEncoder_ReadState(GPIO_Regs *portA, uint32_t pinA,
    GPIO_Regs *portB, uint32_t pinB)
{
    uint8_t state = 0U;

    if (DL_GPIO_readPins(portA, pinA) != 0U) {
        state |= 0x02U;
    }
    if (DL_GPIO_readPins(portB, pinB) != 0U) {
        state |= 0x01U;
    }
    return state;
}

static int8_t BspEncoder_QuadratureStep(uint8_t previous, uint8_t current)
{
    static const int8_t table[16] = {
        0, -1, 1, 0,
        1, 0, 0, -1,
        -1, 0, 0, 1,
        0, 1, -1, 0
    };

    return table[((previous & 0x03U) << 2) | (current & 0x03U)];
}

static void BspEncoder_Process(BspEncoderState *encoder,
    GPIO_Regs *portA, uint32_t pinA, GPIO_Regs *portB, uint32_t pinB)
{
    uint8_t current = BspEncoder_ReadState(portA, pinA, portB, pinB);
    int8_t step = BspEncoder_QuadratureStep(encoder->state, current);

    if ((current != encoder->state) && (step == 0)) {
        encoder->invalidTransitions++;
    } else {
        encoder->delta += step;
    }
    encoder->state = current;
}

static BspEncoderReadout BspEncoder_ReadAndClear(BspEncoderState *encoder)
{
    BspEncoderReadout readout;
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    readout.delta = encoder->delta;
    readout.invalidTransitions = encoder->invalidTransitions;
    encoder->delta = 0;
    encoder->invalidTransitions = 0U;
    __set_PRIMASK(primask);
    return readout;
}

void BspEncoder_Init(void)
{
    uint32_t gpioALowerPins = CAR_ENCODER_GPIOA_INTERRUPT_PINS & 0x0000FFFFU;
    uint32_t gpioAUpperPins = CAR_ENCODER_GPIOA_INTERRUPT_PINS & 0xFFFF0000U;
    uint32_t gpioBLowerPins = CAR_ENCODER_GPIOB_INTERRUPT_PINS & 0x0000FFFFU;
    uint32_t gpioBUpperPins = CAR_ENCODER_GPIOB_INTERRUPT_PINS & 0xFFFF0000U;

    g_encoderA.state = BspEncoder_ReadState(CAR_MOTOR_A_ENCODER_A_PORT,
        CAR_MOTOR_A_ENCODER_A_PIN, CAR_MOTOR_A_ENCODER_B_PORT,
        CAR_MOTOR_A_ENCODER_B_PIN);
    g_encoderB.state = BspEncoder_ReadState(CAR_MOTOR_B_ENCODER_A_PORT,
        CAR_MOTOR_B_ENCODER_A_PIN, CAR_MOTOR_B_ENCODER_B_PORT,
        CAR_MOTOR_B_ENCODER_B_PIN);
    g_encoderC.state = BspEncoder_ReadState(CAR_MOTOR_C_ENCODER_A_PORT,
        CAR_MOTOR_C_ENCODER_A_PIN, CAR_MOTOR_C_ENCODER_B_PORT,
        CAR_MOTOR_C_ENCODER_B_PIN);
    g_encoderD.state = BspEncoder_ReadState(CAR_MOTOR_D_ENCODER_A_PORT,
        CAR_MOTOR_D_ENCODER_A_PIN, CAR_MOTOR_D_ENCODER_B_PORT,
        CAR_MOTOR_D_ENCODER_B_PIN);
    g_encoderA.delta = 0;
    g_encoderA.invalidTransitions = 0U;
    g_encoderB.delta = 0;
    g_encoderB.invalidTransitions = 0U;
    g_encoderC.delta = 0;
    g_encoderC.invalidTransitions = 0U;
    g_encoderD.delta = 0;
    g_encoderD.invalidTransitions = 0U;

    if (gpioALowerPins != 0U) {
        DL_GPIO_setLowerPinsPolarity(GPIOA,
            BspEncoder_BuildPolarityMask(gpioALowerPins, 0U));
    }
    if (gpioAUpperPins != 0U) {
        DL_GPIO_setUpperPinsPolarity(GPIOA,
            BspEncoder_BuildPolarityMask(gpioAUpperPins, 16U));
    }
    if (gpioBLowerPins != 0U) {
        DL_GPIO_setLowerPinsPolarity(GPIOB,
            BspEncoder_BuildPolarityMask(gpioBLowerPins, 0U));
    }
    if (gpioBUpperPins != 0U) {
        DL_GPIO_setUpperPinsPolarity(GPIOB,
            BspEncoder_BuildPolarityMask(gpioBUpperPins, 16U));
    }

    DL_GPIO_clearInterruptStatus(GPIOA, CAR_ENCODER_GPIOA_INTERRUPT_PINS);
    DL_GPIO_clearInterruptStatus(GPIOB, CAR_ENCODER_GPIOB_INTERRUPT_PINS);
    DL_GPIO_enableInterrupt(GPIOA, CAR_ENCODER_GPIOA_INTERRUPT_PINS);
    DL_GPIO_enableInterrupt(GPIOB, CAR_ENCODER_GPIOB_INTERRUPT_PINS);
    NVIC_EnableIRQ(GPIOA_INT_IRQn);
}

BspEncoderReadout BspEncoder_ReadAndClearA(void)
{
    return BspEncoder_ReadAndClear(&g_encoderA);
}

BspEncoderReadout BspEncoder_ReadAndClearB(void)
{
    return BspEncoder_ReadAndClear(&g_encoderB);
}

BspEncoderReadout BspEncoder_ReadAndClearC(void)
{
    return BspEncoder_ReadAndClear(&g_encoderC);
}

BspEncoderReadout BspEncoder_ReadAndClearD(void)
{
    return BspEncoder_ReadAndClear(&g_encoderD);
}

void BspEncoder_GPIOA_IRQHandler(void)
{
    uint32_t pending = DL_GPIO_getEnabledInterruptStatus(GPIOA,
        CAR_ENCODER_GPIOA_INTERRUPT_PINS);

    DL_GPIO_clearInterruptStatus(GPIOA, pending);
    if ((pending & CAR_ENCODER_GPIOA_MOTOR_A_PINS_MASK) != 0U) {
        BspEncoder_Process(&g_encoderA, CAR_MOTOR_A_ENCODER_A_PORT,
            CAR_MOTOR_A_ENCODER_A_PIN, CAR_MOTOR_A_ENCODER_B_PORT,
            CAR_MOTOR_A_ENCODER_B_PIN);
    }
    if ((pending & CAR_ENCODER_GPIOA_MOTOR_B_PINS_MASK) != 0U) {
        BspEncoder_Process(&g_encoderB, CAR_MOTOR_B_ENCODER_A_PORT,
            CAR_MOTOR_B_ENCODER_A_PIN, CAR_MOTOR_B_ENCODER_B_PORT,
            CAR_MOTOR_B_ENCODER_B_PIN);
    }
    if ((pending & CAR_ENCODER_GPIOA_MOTOR_C_PINS_MASK) != 0U) {
        BspEncoder_Process(&g_encoderC, CAR_MOTOR_C_ENCODER_A_PORT,
            CAR_MOTOR_C_ENCODER_A_PIN, CAR_MOTOR_C_ENCODER_B_PORT,
            CAR_MOTOR_C_ENCODER_B_PIN);
    }
    if ((pending & CAR_ENCODER_GPIOA_MOTOR_D_PINS_MASK) != 0U) {
        BspEncoder_Process(&g_encoderD, CAR_MOTOR_D_ENCODER_A_PORT,
            CAR_MOTOR_D_ENCODER_A_PIN, CAR_MOTOR_D_ENCODER_B_PORT,
            CAR_MOTOR_D_ENCODER_B_PIN);
    }
}

void BspEncoder_GPIOB_IRQHandler(void)
{
    uint32_t pending = DL_GPIO_getEnabledInterruptStatus(GPIOB,
        CAR_ENCODER_GPIOB_INTERRUPT_PINS);

    DL_GPIO_clearInterruptStatus(GPIOB, pending);
    if ((pending & CAR_ENCODER_GPIOB_MOTOR_D_PINS_MASK) != 0U) {
        BspEncoder_Process(&g_encoderD, CAR_MOTOR_D_ENCODER_A_PORT,
            CAR_MOTOR_D_ENCODER_A_PIN, CAR_MOTOR_D_ENCODER_B_PORT,
            CAR_MOTOR_D_ENCODER_B_PIN);
    }
}

void GPIOA_IRQHandler(void)
{
    BspEncoder_GPIOA_IRQHandler();
}

void GPIOB_IRQHandler(void)
{
    BspEncoder_GPIOB_IRQHandler();
}

void GROUP1_IRQHandler(void)
{
    uint32_t pending = DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1);

    if (pending == DL_INTERRUPT_GROUP1_IIDX_GPIOA) {
        GPIOA_IRQHandler();
    } else if (pending == DL_INTERRUPT_GROUP1_IIDX_GPIOB) {
        GPIOB_IRQHandler();
    }
}
