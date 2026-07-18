/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerA_backupConfig gPWM_ABackup;
DL_TimerA_backupConfig gPWM_CBackup;
DL_SPI_backupConfig gSPI_LCDBackup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_PWM_A_init();
    SYSCFG_DL_PWM_D_init();
    SYSCFG_DL_PWM_C_init();
    SYSCFG_DL_SPI_LCD_init();
    /* Ensure backup structures have no valid state */
	gPWM_ABackup.backupRdy 	= false;
	gPWM_CBackup.backupRdy 	= false;
	gSPI_LCDBackup.backupRdy 	= false;

}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_saveConfiguration(PWM_A_INST, &gPWM_ABackup);
	retStatus &= DL_TimerA_saveConfiguration(PWM_C_INST, &gPWM_CBackup);
	retStatus &= DL_SPI_saveConfiguration(SPI_LCD_INST, &gSPI_LCDBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_restoreConfiguration(PWM_A_INST, &gPWM_ABackup, false);
	retStatus &= DL_TimerA_restoreConfiguration(PWM_C_INST, &gPWM_CBackup, false);
	retStatus &= DL_SPI_restoreConfiguration(SPI_LCD_INST, &gSPI_LCDBackup);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerA_reset(PWM_A_INST);
    DL_TimerG_reset(PWM_D_INST);
    DL_TimerA_reset(PWM_C_INST);
    DL_SPI_reset(SPI_LCD_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerA_enablePower(PWM_A_INST);
    DL_TimerG_enablePower(PWM_D_INST);
    DL_TimerA_enablePower(PWM_C_INST);
    DL_SPI_enablePower(SPI_LCD_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_A_C2_IOMUX,GPIO_PWM_A_C2_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_A_C2_PORT, GPIO_PWM_A_C2_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_A_C3_IOMUX,GPIO_PWM_A_C3_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_A_C3_PORT, GPIO_PWM_A_C3_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_D_C1_IOMUX,GPIO_PWM_D_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_D_C1_PORT, GPIO_PWM_D_C1_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_C_C0_IOMUX,GPIO_PWM_C_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_C_C0_PORT, GPIO_PWM_C_C0_PIN);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_LCD_IOMUX_SCLK, GPIO_SPI_LCD_IOMUX_SCLK_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_LCD_IOMUX_PICO, GPIO_SPI_LCD_IOMUX_PICO_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_SPI_LCD_IOMUX_POCI, GPIO_SPI_LCD_IOMUX_POCI_FUNC);

    DL_GPIO_initDigitalOutput(LCD_RES_RES_IOMUX);

    DL_GPIO_initDigitalOutput(LCD_DC_DC_IOMUX);

    DL_GPIO_initDigitalOutput(LCD_CS_CS_IOMUX);

    DL_GPIO_initDigitalOutput(LCD_BLK_BLK_IOMUX);

    DL_GPIO_initDigitalInputFeatures(KEY_INPUT_SW_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(KEY1_INPUT_KEY1_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(KEY2_INPUT_KEY2_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(KEY3_INPUT_KEY3_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ENCODER_D_A_E4A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ENCODER_D_B_E4B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(GPIO_HEARTBEAT_HEARTBEAT_IOMUX);

    DL_GPIO_initDigitalOutput(WS2812_DIN_IOMUX);

    DL_GPIO_initDigitalOutput(BUZZER_PWM_IOMUX);

    DL_GPIO_initDigitalOutput(DIR_A_AIN1_IOMUX);

    DL_GPIO_initDigitalOutput(DIR_A_AIN2_IOMUX);

    DL_GPIO_initDigitalOutput(DIR_B_BIN1_IOMUX);

    DL_GPIO_initDigitalOutput(DIR_B_BIN2_IOMUX);

    DL_GPIO_initDigitalOutput(DIR_C_CIN1_IOMUX);

    DL_GPIO_initDigitalOutput(DIR_C_CIN2_IOMUX);

    DL_GPIO_initDigitalOutput(DIR_D_DIN1_IOMUX);

    DL_GPIO_initDigitalOutput(DIR_D_DIN2_IOMUX);

    DL_GPIO_initDigitalInputFeatures(ENCODER_A_E1A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ENCODER_A_E1B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ENCODER_B_E2A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ENCODER_B_E2B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ENCODER_C_E3A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ENCODER_C_E3B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_clearPins(GPIOA, WS2812_DIN_PIN |
		DIR_D_DIN1_PIN |
		DIR_D_DIN2_PIN);
    DL_GPIO_enableOutput(GPIOA, WS2812_DIN_PIN |
		DIR_D_DIN1_PIN |
		DIR_D_DIN2_PIN);
    DL_GPIO_clearPins(GPIOB, LCD_BLK_BLK_PIN |
		GPIO_HEARTBEAT_HEARTBEAT_PIN |
		BUZZER_PWM_PIN |
		DIR_A_AIN1_PIN |
		DIR_A_AIN2_PIN |
		DIR_B_BIN1_PIN |
		DIR_B_BIN2_PIN |
		DIR_C_CIN1_PIN |
		DIR_C_CIN2_PIN);
    DL_GPIO_setPins(GPIOB, LCD_RES_RES_PIN |
		LCD_DC_DC_PIN |
		LCD_CS_CS_PIN);
    DL_GPIO_enableOutput(GPIOB, LCD_RES_RES_PIN |
		LCD_DC_DC_PIN |
		LCD_CS_CS_PIN |
		LCD_BLK_BLK_PIN |
		GPIO_HEARTBEAT_HEARTBEAT_PIN |
		BUZZER_PWM_PIN |
		DIR_A_AIN1_PIN |
		DIR_A_AIN2_PIN |
		DIR_B_BIN1_PIN |
		DIR_B_BIN2_PIN |
		DIR_C_CIN1_PIN |
		DIR_C_CIN2_PIN);

}



SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);

}


/*
 * Timer clock configuration to be sourced by  / 2 (16000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   16000000 Hz = 16000000 Hz / (2 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gPWM_AClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_2,
    .prescale = 0U
};

static const DL_TimerA_PWMConfig gPWM_AConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 800,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_A_init(void) {

    DL_TimerA_setClockConfig(
        PWM_A_INST, (DL_TimerA_ClockConfig *) &gPWM_AClockConfig);

    DL_TimerA_initPWMMode(
        PWM_A_INST, (DL_TimerA_PWMConfig *) &gPWM_AConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerA_setCounterControl(PWM_A_INST,DL_TIMER_CZC_CCCTL2_ZCOND,DL_TIMER_CAC_CCCTL2_ACOND,DL_TIMER_CLC_CCCTL2_LCOND);

    DL_TimerA_setCaptureCompareOutCtl(PWM_A_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_2_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_A_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_2_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_A_INST, 800, DL_TIMER_CC_2_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(PWM_A_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_3_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_A_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_3_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_A_INST, 800, DL_TIMER_CC_3_INDEX);

    DL_TimerA_enableClock(PWM_A_INST);


    
    DL_TimerA_setCCPDirection(PWM_A_INST , DL_TIMER_CC2_OUTPUT | DL_TIMER_CC3_OUTPUT );


}
/*
 * Timer clock configuration to be sourced by  / 2 (16000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   16000000 Hz = 16000000 Hz / (2 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gPWM_DClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_2,
    .prescale = 0U
};

static const DL_TimerG_PWMConfig gPWM_DConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 800,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_D_init(void) {

    DL_TimerG_setClockConfig(
        PWM_D_INST, (DL_TimerG_ClockConfig *) &gPWM_DClockConfig);

    DL_TimerG_initPWMMode(
        PWM_D_INST, (DL_TimerG_PWMConfig *) &gPWM_DConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerG_setCounterControl(PWM_D_INST,DL_TIMER_CZC_CCCTL1_ZCOND,DL_TIMER_CAC_CCCTL1_ACOND,DL_TIMER_CLC_CCCTL1_LCOND);

    DL_TimerG_setCaptureCompareOutCtl(PWM_D_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_1_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(PWM_D_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_D_INST, 800, DL_TIMER_CC_1_INDEX);

    DL_TimerG_enableClock(PWM_D_INST);


    
    DL_TimerG_setCCPDirection(PWM_D_INST , DL_TIMER_CC1_OUTPUT );


}
/*
 * Timer clock configuration to be sourced by  / 2 (16000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   16000000 Hz = 16000000 Hz / (2 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gPWM_CClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_2,
    .prescale = 0U
};

static const DL_TimerA_PWMConfig gPWM_CConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 800,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_C_init(void) {

    DL_TimerA_setClockConfig(
        PWM_C_INST, (DL_TimerA_ClockConfig *) &gPWM_CClockConfig);

    DL_TimerA_initPWMMode(
        PWM_C_INST, (DL_TimerA_PWMConfig *) &gPWM_CConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerA_setCounterControl(PWM_C_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerA_setCaptureCompareOutCtl(PWM_C_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_C_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_C_INST, 800, DL_TIMER_CC_0_INDEX);

    DL_TimerA_enableClock(PWM_C_INST);


    
    DL_TimerA_setCCPDirection(PWM_C_INST , DL_TIMER_CC0_OUTPUT );


}


static const DL_SPI_Config gSPI_LCD_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_8,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
};

static const DL_SPI_ClockConfig gSPI_LCD_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_SPI_LCD_init(void) {
    DL_SPI_setClockConfig(SPI_LCD_INST, (DL_SPI_ClockConfig *) &gSPI_LCD_clockConfig);

    DL_SPI_init(SPI_LCD_INST, (DL_SPI_Config *) &gSPI_LCD_config);

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     8000000 = (32000000)/((1 + 1) * 2)
     */
    DL_SPI_setBitRateSerialClockDivider(SPI_LCD_INST, 1);
    /* Set RX and TX FIFO threshold levels */
    DL_SPI_setFIFOThreshold(SPI_LCD_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);

    /* Enable module */
    DL_SPI_enable(SPI_LCD_INST);
}

