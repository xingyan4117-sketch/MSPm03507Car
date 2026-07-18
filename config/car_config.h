#ifndef CAR_CONFIG_H
#define CAR_CONFIG_H

#include "ti_msp_dl_config.h"

/* A/D motor contracts generated from generated/empty.syscfg. */
#define CAR_PWM_PERIOD_TICKS                 (800U)
#define CAR_MOTOR_A_PWM_TIMER                PWM_A_INST /* PB12 / TIMA0_C2 */
#define CAR_MOTOR_A_PWM_COMPARE_INDEX        GPIO_PWM_A_C2_IDX
#define CAR_MOTOR_A_PWM_PORT                 GPIO_PWM_A_C2_PORT
#define CAR_MOTOR_A_PWM_PIN                  GPIO_PWM_A_C2_PIN
#define CAR_MOTOR_A_PWM_IOMUX                GPIO_PWM_A_C2_IOMUX
#define CAR_MOTOR_A_PWM_IOMUX_FUNCTION       GPIO_PWM_A_C2_IOMUX_FUNC
#define CAR_MOTOR_A_DIRECTION_PORT           DIR_A_PORT
#define CAR_MOTOR_A_DIRECTION_A_PIN          DIR_A_AIN1_PIN /* PB0 */
#define CAR_MOTOR_A_DIRECTION_B_PIN          DIR_A_AIN2_PIN /* PB1 */
#define CAR_MOTOR_A_ENCODER_A_PORT           ENCODER_A_PORT
#define CAR_MOTOR_A_ENCODER_A_PIN            ENCODER_A_E1A_PIN /* PA7 */
#define CAR_MOTOR_A_ENCODER_B_PORT           ENCODER_A_PORT
#define CAR_MOTOR_A_ENCODER_B_PIN            ENCODER_A_E1B_PIN /* PA8 */

#define CAR_MOTOR_B_PWM_TIMER                PWM_A_INST /* PB13 / TIMA0_C3 */
#define CAR_MOTOR_B_PWM_COMPARE_INDEX        GPIO_PWM_A_C3_IDX
#define CAR_MOTOR_B_PWM_PORT                 GPIO_PWM_A_C3_PORT
#define CAR_MOTOR_B_PWM_PIN                  GPIO_PWM_A_C3_PIN
#define CAR_MOTOR_B_PWM_IOMUX                GPIO_PWM_A_C3_IOMUX
#define CAR_MOTOR_B_PWM_IOMUX_FUNCTION       GPIO_PWM_A_C3_IOMUX_FUNC
#define CAR_MOTOR_B_DIRECTION_PORT           DIR_B_PORT
#define CAR_MOTOR_B_DIRECTION_A_PIN          DIR_B_BIN1_PIN /* PB4 */
#define CAR_MOTOR_B_DIRECTION_B_PIN          DIR_B_BIN2_PIN /* PB5 */
#define CAR_MOTOR_B_ENCODER_A_PORT           ENCODER_B_PORT
#define CAR_MOTOR_B_ENCODER_A_PIN            ENCODER_B_E2A_PIN /* PA9 */
#define CAR_MOTOR_B_ENCODER_B_PORT           ENCODER_B_PORT
#define CAR_MOTOR_B_ENCODER_B_PIN            ENCODER_B_E2B_PIN /* PA12 */

#define CAR_MOTOR_C_PWM_TIMER                PWM_C_INST /* PA17 / TIMA1_C0 */
#define CAR_MOTOR_C_PWM_COMPARE_INDEX        GPIO_PWM_C_C0_IDX
#define CAR_MOTOR_C_PWM_PORT                 GPIO_PWM_C_C0_PORT
#define CAR_MOTOR_C_PWM_PIN                  GPIO_PWM_C_C0_PIN
#define CAR_MOTOR_C_PWM_IOMUX                GPIO_PWM_C_C0_IOMUX
#define CAR_MOTOR_C_PWM_IOMUX_FUNCTION       GPIO_PWM_C_C0_IOMUX_FUNC
#define CAR_MOTOR_C_DIRECTION_PORT           DIR_C_PORT
#define CAR_MOTOR_C_DIRECTION_A_PIN          DIR_C_CIN1_PIN /* PB15 */
#define CAR_MOTOR_C_DIRECTION_B_PIN          DIR_C_CIN2_PIN /* PB16 */
#define CAR_MOTOR_C_ENCODER_A_PORT           ENCODER_C_PORT
#define CAR_MOTOR_C_ENCODER_A_PIN            ENCODER_C_E3A_PIN /* PA21 */
#define CAR_MOTOR_C_ENCODER_B_PORT           ENCODER_C_PORT
#define CAR_MOTOR_C_ENCODER_B_PIN            ENCODER_C_E3B_PIN /* PA24 */

#define CAR_MOTOR_D_PWM_TIMER                PWM_D_INST /* PA13 / TIMG0_C1 */
#define CAR_MOTOR_D_PWM_COMPARE_INDEX        GPIO_PWM_D_C1_IDX
#define CAR_MOTOR_D_PWM_PORT                 GPIO_PWM_D_C1_PORT
#define CAR_MOTOR_D_PWM_PIN                  GPIO_PWM_D_C1_PIN
#define CAR_MOTOR_D_PWM_IOMUX                GPIO_PWM_D_C1_IOMUX
#define CAR_MOTOR_D_PWM_IOMUX_FUNCTION       GPIO_PWM_D_C1_IOMUX_FUNC
#define CAR_MOTOR_D_DIRECTION_PORT           DIR_D_PORT
#define CAR_MOTOR_D_DIRECTION_A_PIN          DIR_D_DIN1_PIN /* PA0 */
#define CAR_MOTOR_D_DIRECTION_B_PIN          DIR_D_DIN2_PIN /* PA1 */
#define CAR_MOTOR_D_ENCODER_A_PORT           ENCODER_D_A_PORT
#define CAR_MOTOR_D_ENCODER_A_PIN            ENCODER_D_A_E4A_PIN /* PA28 */
#define CAR_MOTOR_D_ENCODER_B_PORT           ENCODER_D_B_PORT
#define CAR_MOTOR_D_ENCODER_B_PIN            ENCODER_D_B_E4B_PIN /* PB23 */

#define CAR_ENCODER_GPIOA_MOTOR_A_PINS_MASK  (CAR_MOTOR_A_ENCODER_A_PIN | CAR_MOTOR_A_ENCODER_B_PIN)
#define CAR_ENCODER_GPIOA_MOTOR_B_PINS_MASK  (CAR_MOTOR_B_ENCODER_A_PIN | CAR_MOTOR_B_ENCODER_B_PIN)
#define CAR_ENCODER_GPIOA_MOTOR_C_PINS_MASK  (CAR_MOTOR_C_ENCODER_A_PIN | CAR_MOTOR_C_ENCODER_B_PIN)
#define CAR_ENCODER_GPIOA_MOTOR_D_PINS_MASK  (CAR_MOTOR_D_ENCODER_A_PIN)
#define CAR_ENCODER_GPIOA_INTERRUPT_PINS      (CAR_ENCODER_GPIOA_MOTOR_A_PINS_MASK | CAR_ENCODER_GPIOA_MOTOR_B_PINS_MASK | CAR_ENCODER_GPIOA_MOTOR_C_PINS_MASK | CAR_ENCODER_GPIOA_MOTOR_D_PINS_MASK)
#define CAR_ENCODER_GPIOB_MOTOR_D_PINS_MASK  (CAR_MOTOR_D_ENCODER_B_PIN)
#define CAR_ENCODER_GPIOB_INTERRUPT_PINS      (CAR_ENCODER_GPIOB_MOTOR_D_PINS_MASK)

/* Board controls and ST7735S display contracts. */
#define CAR_KEY_PORT                          KEY_INPUT_PORT
#define CAR_KEY_PIN                           KEY_INPUT_SW_PIN /* PB21 */
#define CAR_KEY1_PORT                         KEY1_INPUT_PORT
#define CAR_KEY1_PIN                          KEY1_INPUT_KEY1_PIN /* PA14 */
#define CAR_KEY2_PORT                         KEY2_INPUT_PORT
#define CAR_KEY2_PIN                          KEY2_INPUT_KEY2_PIN /* PA15 */
#define CAR_KEY3_PORT                         KEY3_INPUT_PORT
#define CAR_KEY3_PIN                          KEY3_INPUT_KEY3_PIN /* PA16 */
#define CAR_HEARTBEAT_PORT                    GPIO_HEARTBEAT_PORT /* PB22 */
#define CAR_HEARTBEAT_PIN                     GPIO_HEARTBEAT_HEARTBEAT_PIN /* PB22 */
#define CAR_WS2812_PORT                       WS2812_PORT /* PA29 / WS2812 DIN */
#define CAR_WS2812_PIN                        WS2812_DIN_PIN
#define CAR_WS2812_IOMUX                      WS2812_DIN_IOMUX
#define CAR_BUZZER_PORT                       BUZZER_PORT /* PB27 */
#define CAR_BUZZER_PIN                        BUZZER_PWM_PIN
#define CAR_ST7735_SPI_INSTANCE               SPI_LCD_INST /* SPI1: PB8/PB9 */
#define CAR_ST7735_DC_PORT                    LCD_DC_PORT
#define CAR_ST7735_DC_PIN                     LCD_DC_DC_PIN /* PB11 */
#define CAR_ST7735_CS_PORT                    LCD_CS_PORT
#define CAR_ST7735_CS_PIN                     LCD_CS_CS_PIN /* PB14 */
#define CAR_ST7735_RES_PORT                   LCD_RES_PORT
#define CAR_ST7735_RES_PIN                    LCD_RES_RES_PIN /* PB10 */
#define CAR_ST7735_BLK_PORT                   LCD_BLK_PORT
#define CAR_ST7735_BLK_PIN                    LCD_BLK_BLK_PIN /* PB26 */

#endif
