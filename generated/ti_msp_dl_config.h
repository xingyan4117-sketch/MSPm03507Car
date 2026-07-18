/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     32000000



/* Defines for PWM_A */
#define PWM_A_INST                                                         TIMA0
#define PWM_A_INST_IRQHandler                                   TIMA0_IRQHandler
#define PWM_A_INST_INT_IRQN                                     (TIMA0_INT_IRQn)
#define PWM_A_INST_CLK_FREQ                                             16000000
/* GPIO defines for channel 2 */
#define GPIO_PWM_A_C2_PORT                                                 GPIOB
#define GPIO_PWM_A_C2_PIN                                         DL_GPIO_PIN_12
#define GPIO_PWM_A_C2_IOMUX                                      (IOMUX_PINCM29)
#define GPIO_PWM_A_C2_IOMUX_FUNC                     IOMUX_PINCM29_PF_TIMA0_CCP2
#define GPIO_PWM_A_C2_IDX                                    DL_TIMER_CC_2_INDEX
/* GPIO defines for channel 3 */
#define GPIO_PWM_A_C3_PORT                                                 GPIOB
#define GPIO_PWM_A_C3_PIN                                         DL_GPIO_PIN_13
#define GPIO_PWM_A_C3_IOMUX                                      (IOMUX_PINCM30)
#define GPIO_PWM_A_C3_IOMUX_FUNC                     IOMUX_PINCM30_PF_TIMA0_CCP3
#define GPIO_PWM_A_C3_IDX                                    DL_TIMER_CC_3_INDEX

/* Defines for PWM_D */
#define PWM_D_INST                                                         TIMG0
#define PWM_D_INST_IRQHandler                                   TIMG0_IRQHandler
#define PWM_D_INST_INT_IRQN                                     (TIMG0_INT_IRQn)
#define PWM_D_INST_CLK_FREQ                                             16000000
/* GPIO defines for channel 1 */
#define GPIO_PWM_D_C1_PORT                                                 GPIOA
#define GPIO_PWM_D_C1_PIN                                         DL_GPIO_PIN_13
#define GPIO_PWM_D_C1_IOMUX                                      (IOMUX_PINCM35)
#define GPIO_PWM_D_C1_IOMUX_FUNC                     IOMUX_PINCM35_PF_TIMG0_CCP1
#define GPIO_PWM_D_C1_IDX                                    DL_TIMER_CC_1_INDEX

/* Defines for PWM_C */
#define PWM_C_INST                                                         TIMA1
#define PWM_C_INST_IRQHandler                                   TIMA1_IRQHandler
#define PWM_C_INST_INT_IRQN                                     (TIMA1_INT_IRQn)
#define PWM_C_INST_CLK_FREQ                                             16000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_C_C0_PORT                                                 GPIOA
#define GPIO_PWM_C_C0_PIN                                         DL_GPIO_PIN_17
#define GPIO_PWM_C_C0_IOMUX                                      (IOMUX_PINCM39)
#define GPIO_PWM_C_C0_IOMUX_FUNC                     IOMUX_PINCM39_PF_TIMA1_CCP0
#define GPIO_PWM_C_C0_IDX                                    DL_TIMER_CC_0_INDEX



/* Defines for SPI_LCD */
#define SPI_LCD_INST                                                       SPI1
#define SPI_LCD_INST_IRQHandler                                 SPI1_IRQHandler
#define SPI_LCD_INST_INT_IRQN                                     SPI1_INT_IRQn
#define GPIO_SPI_LCD_PICO_PORT                                            GPIOB
#define GPIO_SPI_LCD_PICO_PIN                                     DL_GPIO_PIN_8
#define GPIO_SPI_LCD_IOMUX_PICO                                 (IOMUX_PINCM25)
#define GPIO_SPI_LCD_IOMUX_PICO_FUNC                 IOMUX_PINCM25_PF_SPI1_PICO
#define GPIO_SPI_LCD_POCI_PORT                                            GPIOB
#define GPIO_SPI_LCD_POCI_PIN                                     DL_GPIO_PIN_7
#define GPIO_SPI_LCD_IOMUX_POCI                                 (IOMUX_PINCM24)
#define GPIO_SPI_LCD_IOMUX_POCI_FUNC                 IOMUX_PINCM24_PF_SPI1_POCI
/* GPIO configuration for SPI_LCD */
#define GPIO_SPI_LCD_SCLK_PORT                                            GPIOB
#define GPIO_SPI_LCD_SCLK_PIN                                     DL_GPIO_PIN_9
#define GPIO_SPI_LCD_IOMUX_SCLK                                 (IOMUX_PINCM26)
#define GPIO_SPI_LCD_IOMUX_SCLK_FUNC                 IOMUX_PINCM26_PF_SPI1_SCLK



/* Port definition for Pin Group LCD_RES */
#define LCD_RES_PORT                                                     (GPIOB)

/* Defines for RES: GPIOB.10 with pinCMx 27 on package pin 62 */
#define LCD_RES_RES_PIN                                         (DL_GPIO_PIN_10)
#define LCD_RES_RES_IOMUX                                        (IOMUX_PINCM27)
/* Port definition for Pin Group LCD_DC */
#define LCD_DC_PORT                                                      (GPIOB)

/* Defines for DC: GPIOB.11 with pinCMx 28 on package pin 63 */
#define LCD_DC_DC_PIN                                           (DL_GPIO_PIN_11)
#define LCD_DC_DC_IOMUX                                          (IOMUX_PINCM28)
/* Port definition for Pin Group LCD_CS */
#define LCD_CS_PORT                                                      (GPIOB)

/* Defines for CS: GPIOB.14 with pinCMx 31 on package pin 2 */
#define LCD_CS_CS_PIN                                           (DL_GPIO_PIN_14)
#define LCD_CS_CS_IOMUX                                          (IOMUX_PINCM31)
/* Port definition for Pin Group LCD_BLK */
#define LCD_BLK_PORT                                                     (GPIOB)

/* Defines for BLK: GPIOB.26 with pinCMx 57 on package pin 28 */
#define LCD_BLK_BLK_PIN                                         (DL_GPIO_PIN_26)
#define LCD_BLK_BLK_IOMUX                                        (IOMUX_PINCM57)
/* Port definition for Pin Group KEY_INPUT */
#define KEY_INPUT_PORT                                                   (GPIOB)

/* Defines for SW: GPIOB.21 with pinCMx 49 on package pin 20 */
#define KEY_INPUT_SW_PIN                                        (DL_GPIO_PIN_21)
#define KEY_INPUT_SW_IOMUX                                       (IOMUX_PINCM49)
/* Port definition for Pin Group KEY1_INPUT */
#define KEY1_INPUT_PORT                                                  (GPIOA)

/* Defines for KEY1: GPIOA.14 with pinCMx 36 on package pin 7 */
#define KEY1_INPUT_KEY1_PIN                                     (DL_GPIO_PIN_14)
#define KEY1_INPUT_KEY1_IOMUX                                    (IOMUX_PINCM36)
/* Port definition for Pin Group KEY2_INPUT */
#define KEY2_INPUT_PORT                                                  (GPIOA)

/* Defines for KEY2: GPIOA.15 with pinCMx 37 on package pin 8 */
#define KEY2_INPUT_KEY2_PIN                                     (DL_GPIO_PIN_15)
#define KEY2_INPUT_KEY2_IOMUX                                    (IOMUX_PINCM37)
/* Port definition for Pin Group KEY3_INPUT */
#define KEY3_INPUT_PORT                                                  (GPIOA)

/* Defines for KEY3: GPIOA.16 with pinCMx 38 on package pin 9 */
#define KEY3_INPUT_KEY3_PIN                                     (DL_GPIO_PIN_16)
#define KEY3_INPUT_KEY3_IOMUX                                    (IOMUX_PINCM38)
/* Port definition for Pin Group ENCODER_D_A */
#define ENCODER_D_A_PORT                                                 (GPIOA)

/* Defines for E4A: GPIOA.28 with pinCMx 3 on package pin 35 */
#define ENCODER_D_A_E4A_PIN                                     (DL_GPIO_PIN_28)
#define ENCODER_D_A_E4A_IOMUX                                     (IOMUX_PINCM3)
/* Port definition for Pin Group ENCODER_D_B */
#define ENCODER_D_B_PORT                                                 (GPIOB)

/* Defines for E4B: GPIOB.23 with pinCMx 51 on package pin 22 */
#define ENCODER_D_B_E4B_PIN                                     (DL_GPIO_PIN_23)
#define ENCODER_D_B_E4B_IOMUX                                    (IOMUX_PINCM51)
/* Port definition for Pin Group GPIO_HEARTBEAT */
#define GPIO_HEARTBEAT_PORT                                              (GPIOB)

/* Defines for HEARTBEAT: GPIOB.22 with pinCMx 50 on package pin 21 */
#define GPIO_HEARTBEAT_HEARTBEAT_PIN                            (DL_GPIO_PIN_22)
#define GPIO_HEARTBEAT_HEARTBEAT_IOMUX                           (IOMUX_PINCM50)
/* Port definition for Pin Group WS2812 */
#define WS2812_PORT                                                      (GPIOA)

/* Defines for DIN: GPIOA.29 with pinCMx 4 on package pin 36 */
#define WS2812_DIN_PIN                                          (DL_GPIO_PIN_29)
#define WS2812_DIN_IOMUX                                          (IOMUX_PINCM4)
/* Port definition for Pin Group BUZZER */
#define BUZZER_PORT                                                      (GPIOB)

/* Defines for PWM: GPIOB.27 with pinCMx 58 on package pin 29 */
#define BUZZER_PWM_PIN                                          (DL_GPIO_PIN_27)
#define BUZZER_PWM_IOMUX                                         (IOMUX_PINCM58)
/* Port definition for Pin Group DIR_A */
#define DIR_A_PORT                                                       (GPIOB)

/* Defines for AIN1: GPIOB.0 with pinCMx 12 on package pin 47 */
#define DIR_A_AIN1_PIN                                           (DL_GPIO_PIN_0)
#define DIR_A_AIN1_IOMUX                                         (IOMUX_PINCM12)
/* Defines for AIN2: GPIOB.1 with pinCMx 13 on package pin 48 */
#define DIR_A_AIN2_PIN                                           (DL_GPIO_PIN_1)
#define DIR_A_AIN2_IOMUX                                         (IOMUX_PINCM13)
/* Port definition for Pin Group DIR_B */
#define DIR_B_PORT                                                       (GPIOB)

/* Defines for BIN1: GPIOB.4 with pinCMx 17 on package pin 52 */
#define DIR_B_BIN1_PIN                                           (DL_GPIO_PIN_4)
#define DIR_B_BIN1_IOMUX                                         (IOMUX_PINCM17)
/* Defines for BIN2: GPIOB.5 with pinCMx 18 on package pin 53 */
#define DIR_B_BIN2_PIN                                           (DL_GPIO_PIN_5)
#define DIR_B_BIN2_IOMUX                                         (IOMUX_PINCM18)
/* Port definition for Pin Group DIR_C */
#define DIR_C_PORT                                                       (GPIOB)

/* Defines for CIN1: GPIOB.15 with pinCMx 32 on package pin 3 */
#define DIR_C_CIN1_PIN                                          (DL_GPIO_PIN_15)
#define DIR_C_CIN1_IOMUX                                         (IOMUX_PINCM32)
/* Defines for CIN2: GPIOB.16 with pinCMx 33 on package pin 4 */
#define DIR_C_CIN2_PIN                                          (DL_GPIO_PIN_16)
#define DIR_C_CIN2_IOMUX                                         (IOMUX_PINCM33)
/* Port definition for Pin Group DIR_D */
#define DIR_D_PORT                                                       (GPIOA)

/* Defines for DIN1: GPIOA.0 with pinCMx 1 on package pin 33 */
#define DIR_D_DIN1_PIN                                           (DL_GPIO_PIN_0)
#define DIR_D_DIN1_IOMUX                                          (IOMUX_PINCM1)
/* Defines for DIN2: GPIOA.1 with pinCMx 2 on package pin 34 */
#define DIR_D_DIN2_PIN                                           (DL_GPIO_PIN_1)
#define DIR_D_DIN2_IOMUX                                          (IOMUX_PINCM2)
/* Port definition for Pin Group ENCODER_A */
#define ENCODER_A_PORT                                                   (GPIOA)

/* Defines for E1A: GPIOA.7 with pinCMx 14 on package pin 49 */
#define ENCODER_A_E1A_PIN                                        (DL_GPIO_PIN_7)
#define ENCODER_A_E1A_IOMUX                                      (IOMUX_PINCM14)
/* Defines for E1B: GPIOA.8 with pinCMx 19 on package pin 54 */
#define ENCODER_A_E1B_PIN                                        (DL_GPIO_PIN_8)
#define ENCODER_A_E1B_IOMUX                                      (IOMUX_PINCM19)
/* Port definition for Pin Group ENCODER_B */
#define ENCODER_B_PORT                                                   (GPIOA)

/* Defines for E2A: GPIOA.9 with pinCMx 20 on package pin 55 */
#define ENCODER_B_E2A_PIN                                        (DL_GPIO_PIN_9)
#define ENCODER_B_E2A_IOMUX                                      (IOMUX_PINCM20)
/* Defines for E2B: GPIOA.12 with pinCMx 34 on package pin 5 */
#define ENCODER_B_E2B_PIN                                       (DL_GPIO_PIN_12)
#define ENCODER_B_E2B_IOMUX                                      (IOMUX_PINCM34)
/* Port definition for Pin Group ENCODER_C */
#define ENCODER_C_PORT                                                   (GPIOA)

/* Defines for E3A: GPIOA.21 with pinCMx 46 on package pin 17 */
#define ENCODER_C_E3A_PIN                                       (DL_GPIO_PIN_21)
#define ENCODER_C_E3A_IOMUX                                      (IOMUX_PINCM46)
/* Defines for E3B: GPIOA.24 with pinCMx 54 on package pin 25 */
#define ENCODER_C_E3B_PIN                                       (DL_GPIO_PIN_24)
#define ENCODER_C_E3B_IOMUX                                      (IOMUX_PINCM54)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_PWM_A_init(void);
void SYSCFG_DL_PWM_D_init(void);
void SYSCFG_DL_PWM_C_init(void);
void SYSCFG_DL_SPI_LCD_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
