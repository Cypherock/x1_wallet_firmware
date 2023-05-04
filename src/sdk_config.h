/**
 * @file    sdk_config.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#if USE_SIMULATOR == 0
#include "main.h"
#endif

#ifndef BSP_SDK_CONFIG_H_
#define BSP_SDK_CONFIG_H_

/** ***HARDWARE VERSION***
 *  Major (1byte):-  Change with MCU or major hardware change.
 *  Minor (1 byte):- Changes with memory map or with other ICs.
 *  Patch (2 byte):- Changes with hardware iterations like GPIO change,
 * peripheral change. While firmware upgrade, bootloader will ignore patch
 * changes and compare the major and minor only.
 */
#define DEVICE_HARDWARE_STM32_2 0x00010002
#define DEVICE_HARDWARE_STM32_3 0x00010003

#define ATECC_MODE_I2C2 1
#define ATECC_MODE_I2C2_ALT 2
#define ATECC_MODE_SWI 3

typedef uint8_t atecc_interface_type;

extern atecc_interface_type atecc_mode;

#define BSP_USB_VID 0x3503
#if (X1WALLET_INITIAL == 1)
#define BSP_USB_PID                                                            \
  0x0102    /// PID Wallet X1[8:15]:0x01, HARDWARE version Major[4:7]:0x0,
            /// Initial[0:3]:0x2
#elif (X1WALLET_MAIN == 1)
#define BSP_USB_PID                                                            \
  0x0103    /// PID Wallet X1[8:15]:0x01, HARDWARE version Major[4:7]:0x0,
            /// Main[0:3]:0x3
#endif

#define BSP_I2C_SPEED_FAST 0x00702991
#define BSP_I2C_SPEED_STANDARD 0x10909CEC

#define BSP_ATECC_I2C I2C2
#define BSP_ATECC_I2C_MODE_FAST BSP_I2C_SPEED_FAST
#define BSP_ATECC_I2C_MODE_STANDARD BSP_I2C_SPEED_STANDARD
#define BSP_ATECC_I2C_GPIO_PORT GPIOB
#define BSP_ATECC_I2C_SDA_PIN GPIO_PIN_14
#define BSP_ATECC_I2C_SCL_PIN GPIO_PIN_13
#define BSP_ATECC_I2C_SDA_PIN_ALT GPIO_PIN_11
#define BSP_ATECC_I2C_SCL_PIN_ALT GPIO_PIN_10
#define BSP_ATECC_I2C_GPIO_AF GPIO_AF4_I2C2
#define BSP_ATECC_I2C_CLK_ENABLE __HAL_RCC_I2C2_CLK_ENABLE()
#define BSP_ATECC_I2C_CLK_DISABLE __HAL_RCC_I2C2_CLK_DISABLE()
#define BSP_ATECC_SWI_UART LPUART1
#define BSP_ATECC_SWI_UART_GPIO_PORT GPIOB
#define BSP_ATECC_SWI_UART_GPIO_PIN GPIO_PIN_11
#define BSP_ATECC_SWI_UART_CLK_ENABLE __HAL_RCC_LPUART1_CLK_ENABLE()
#define BSP_ATECC_SWI_UART_CLK_DISABLE __HAL_RCC_LPUART1_CLK_DISABLE()

#define BSP_PN532_OLED_I2C I2C1
#define BSP_PN532_OLED_I2C_SPEED BSP_I2C_SPEED_STANDARD
#define BSP_PN532_OLED_I2C_GPIO_PORT GPIOB
#define BSP_PN532_OLED_I2C_SDA_PIN GPIO_PIN_7
#define BSP_PN532_OLED_I2C_SCL_PIN GPIO_PIN_6
#define BSP_PN532_OLED_I2C_GPIO_AF GPIO_AF4_I2C1
#define BSP_PN532_OLED_I2C_CLK_ENABLE __HAL_RCC_I2C1_CLK_ENABLE()
#define BSP_PN532_OLED_I2C_CLK_DISABLE __HAL_RCC_I2C1_CLK_DISABLE()

#define BSP_PN532_RST_PORT GPIOB
#define BSP_PN532_RST_PIN GPIO_PIN_5
#define BSP_PN532_INT_PORT GPIOB
#define BSP_PN532_INT_PIN GPIO_PIN_4

#define BSP_JOYSTICK_GPIO_PORT GPIOC
#define BSP_JOYSTICK_UP_PIN GPIO_PIN_5
#define BSP_JOYSTICK_DOWN_PIN GPIO_PIN_6
#define BSP_JOYSTICK_RIGHT_PIN GPIO_PIN_4
#define BSP_JOYSTICK_LEFT_PIN GPIO_PIN_8
#define BSP_JOYSTICK_ENTER_PIN GPIO_PIN_7
#define BSP_JOYSTICK_GPIO_PULL(ver)                                            \
  (ver == DEVICE_HARDWARE_STM32_2 ? GPIO_PULLUP : GPIO_PULLDOWN)
#define BSP_JOYSTICK_GPIO_MODE(ver)                                            \
  (ver == DEVICE_HARDWARE_STM32_2 ? GPIO_MODE_IT_FALLING : GPIO_MODE_IT_RISING)
#define BSP_JOYSTICK_READ_STATE(ver)                                           \
  (ver == DEVICE_HARDWARE_STM32_2 ? GPIO_PIN_RESET : GPIO_PIN_SET)

#define BSP_BUZZER_TIMER TIM2
#define BSP_BUZZER_PWM_FREQ 4000
#define BSP_BUZZER_TIMER_PRESCALAR 0
#define BSP_BUZZER_TIMER_PERIOD 19999
#define BSP_BUZZER_TIMER_COUNTER 3799
#define BSP_BUZZER_GPIO_PORT GPIOA
#define BSP_BUZZER_GPIO_PIN GPIO_PIN_0
#define BSP_BUZZER_PWM_CHANNEL TIM_CHANNEL_1
#define BSP_BUZZER_TIMER_CLK_ENABLE __HAL_RCC_TIM2_CLK_ENABLE()
#define BSP_BUZZER_TIMER_CLK_DISABLE __HAL_RCC_TIM2_CLK_DISABLE()

#define Unused_Pin GPIO_PIN_3
#define Unused_GPIO_Port GPIOE
#define UnusedE1_Pin GPIO_PIN_1
#define UnusedE1_GPIO_Port GPIOE
#define UnusedB8_Pin GPIO_PIN_8
#define UnusedB8_GPIO_Port GPIOB
#define UnusedD7_Pin GPIO_PIN_7
#define UnusedD7_GPIO_Port GPIOD
#define UnusedD5_Pin GPIO_PIN_5
#define UnusedD5_GPIO_Port GPIOD
#define UnusedB3_Pin GPIO_PIN_3
#define UnusedB3_GPIO_Port GPIOB
#define UnusedA15_Pin GPIO_PIN_15
#define UnusedA15_GPIO_Port GPIOA
#define UnusedA14_Pin GPIO_PIN_14
#define UnusedA14_GPIO_Port GPIOA
#define UnusedA13_Pin GPIO_PIN_13
#define UnusedA13_GPIO_Port GPIOA
#define UnusedE4_Pin GPIO_PIN_4
#define UnusedE4_GPIO_Port GPIOE
#define UnusedE2_Pin GPIO_PIN_2
#define UnusedE2_GPIO_Port GPIOE
#define UnusedB9_Pin GPIO_PIN_9
#define UnusedB9_GPIO_Port GPIOB
#define UnusedD6_Pin GPIO_PIN_6
#define UnusedD6_GPIO_Port GPIOD
#define UnusedD4_Pin GPIO_PIN_4
#define UnusedD4_GPIO_Port GPIOD
#define UnusedD3_Pin GPIO_PIN_3
#define UnusedD3_GPIO_Port GPIOD
#define UnusedD1_Pin GPIO_PIN_1
#define UnusedD1_GPIO_Port GPIOD
#define UnusedC12_Pin GPIO_PIN_12
#define UnusedC12_GPIO_Port GPIOC
#define UnusedC10_Pin GPIO_PIN_10
#define UnusedC10_GPIO_Port GPIOC
#define UnusedC13_Pin GPIO_PIN_13
#define UnusedC13_GPIO_Port GPIOC
#define UnusedE5_Pin GPIO_PIN_5
#define UnusedE5_GPIO_Port GPIOE
#define UnusedE0_Pin GPIO_PIN_0
#define UnusedE0_GPIO_Port GPIOE
#define UnusedB5_Pin GPIO_PIN_5
#define UnusedB5_GPIO_Port GPIOB
#define UnusedG14_Pin GPIO_PIN_14
#define UnusedG14_GPIO_Port GPIOG
#define UnusedG13_Pin GPIO_PIN_13
#define UnusedG13_GPIO_Port GPIOG
#define UnusedD2_Pin GPIO_PIN_2
#define UnusedD2_GPIO_Port GPIOD
#define UnusedD0_Pin GPIO_PIN_0
#define UnusedD0_GPIO_Port GPIOD
#define UnusedC11_Pin GPIO_PIN_11
#define UnusedC11_GPIO_Port GPIOC
#define UnusedA10_Pin GPIO_PIN_10
#define UnusedA10_GPIO_Port GPIOA
#define UnusedC14_Pin GPIO_PIN_14
#define UnusedC14_GPIO_Port GPIOC
#define UnusedE6_Pin GPIO_PIN_6
#define UnusedE6_GPIO_Port GPIOE
#define UnusedF2_Pin GPIO_PIN_2
#define UnusedF2_GPIO_Port GPIOF
#define UnusedF1_Pin GPIO_PIN_1
#define UnusedF1_GPIO_Port GPIOF
#define UnusedF0_Pin GPIO_PIN_0
#define UnusedF0_GPIO_Port GPIOF
#define UnusedG12_Pin GPIO_PIN_12
#define UnusedG12_GPIO_Port GPIOG
#define UnusedG10_Pin GPIO_PIN_10
#define UnusedG10_GPIO_Port GPIOG
#define UnusedG9_Pin GPIO_PIN_9
#define UnusedG9_GPIO_Port GPIOG
#define UnusedA9_Pin GPIO_PIN_9
#define UnusedA9_GPIO_Port GPIOA
#define UnusedA8_Pin GPIO_PIN_8
#define UnusedA8_GPIO_Port GPIOA
#define UnusedC9_Pin GPIO_PIN_9
#define UnusedC9_GPIO_Port GPIOC
#define UnusedC15_Pin GPIO_PIN_15
#define UnusedC15_GPIO_Port GPIOC
#define UnusedF3_Pin GPIO_PIN_3
#define UnusedF3_GPIO_Port GPIOF
#define UnusedG5_Pin GPIO_PIN_5
#define UnusedG5_GPIO_Port GPIOG
#define UnusedH0_Pin GPIO_PIN_0
#define UnusedH0_GPIO_Port GPIOH
#define UnusedF4_Pin GPIO_PIN_4
#define UnusedF4_GPIO_Port GPIOF
#define UnusedF5_Pin GPIO_PIN_5
#define UnusedF5_GPIO_Port GPIOF
#define UnusedG3_Pin GPIO_PIN_3
#define UnusedG3_GPIO_Port GPIOG
#define UnusedG4_Pin GPIO_PIN_4
#define UnusedG4_GPIO_Port GPIOG
#define UnusedH1_Pin GPIO_PIN_1
#define UnusedH1_GPIO_Port GPIOH
#define UnusedG11_Pin GPIO_PIN_11
#define UnusedG11_GPIO_Port GPIOG
#define UnusedG6_Pin GPIO_PIN_6
#define UnusedG6_GPIO_Port GPIOG
#define UnusedG1_Pin GPIO_PIN_1
#define UnusedG1_GPIO_Port GPIOG
#define UnusedG2_Pin GPIO_PIN_2
#define UnusedG2_GPIO_Port GPIOG
#define UnusedC0_Pin GPIO_PIN_0
#define UnusedC0_GPIO_Port GPIOC
#define UnusedG7_Pin GPIO_PIN_7
#define UnusedG7_GPIO_Port GPIOG
#define UnusedG0_Pin GPIO_PIN_0
#define UnusedG0_GPIO_Port GPIOG
#define UnusedD15_Pin GPIO_PIN_15
#define UnusedD15_GPIO_Port GPIOD
#define UnusedD14_Pin GPIO_PIN_14
#define UnusedD14_GPIO_Port GPIOD
#define UnusedD13_Pin GPIO_PIN_13
#define UnusedD13_GPIO_Port GPIOD
#define UnusedC1_Pin GPIO_PIN_1
#define UnusedC1_GPIO_Port GPIOC
#define UnusedC2_Pin GPIO_PIN_2
#define UnusedC2_GPIO_Port GPIOC
#define UnusedA4_Pin GPIO_PIN_4
#define UnusedA4_GPIO_Port GPIOA
#define UnusedA7_Pin GPIO_PIN_7
#define UnusedA7_GPIO_Port GPIOA
#define UnusedG8_Pin GPIO_PIN_8
#define UnusedG8_GPIO_Port GPIOG
#define UnusedF12_Pin GPIO_PIN_12
#define UnusedF12_GPIO_Port GPIOF
#define UnusedF14_Pin GPIO_PIN_14
#define UnusedF14_GPIO_Port GPIOF
#define UnusedF15_Pin GPIO_PIN_15
#define UnusedF15_GPIO_Port GPIOF
#define UnusedD12_Pin GPIO_PIN_12
#define UnusedD12_GPIO_Port GPIOD
#define UnusedD11_Pin GPIO_PIN_11
#define UnusedD11_GPIO_Port GPIOD
#define UnusedD10_Pin GPIO_PIN_10
#define UnusedD10_GPIO_Port GPIOD
#define UnusedG15_Pin GPIO_PIN_15
#define UnusedG15_GPIO_Port GPIOG
#define UnusedC3_Pin GPIO_PIN_3
#define UnusedC3_GPIO_Port GPIOC
#define UnusedA2_Pin GPIO_PIN_2
#define UnusedA2_GPIO_Port GPIOA
#define UnusedA5_Pin GPIO_PIN_5
#define UnusedA5_GPIO_Port GPIOA
#define UnusedF11_Pin GPIO_PIN_11
#define UnusedF11_GPIO_Port GPIOF
#define UnusedF13_Pin GPIO_PIN_13
#define UnusedF13_GPIO_Port GPIOF
#define UnusedD9_Pin GPIO_PIN_9
#define UnusedD9_GPIO_Port GPIOD
#define UnusedD8_Pin GPIO_PIN_8
#define UnusedD8_GPIO_Port GPIOD
#define UnusedB15_Pin GPIO_PIN_15
#define UnusedB15_GPIO_Port GPIOB
#define UnusedB14_Pin GPIO_PIN_14
#define UnusedB14_GPIO_Port GPIOB
#define UnusedB13_Pin GPIO_PIN_13
#define UnusedB13_GPIO_Port GPIOB
#define UnusedA3_Pin GPIO_PIN_3
#define UnusedA3_GPIO_Port GPIOA
#define UnusedA6_Pin GPIO_PIN_6
#define UnusedA6_GPIO_Port GPIOA
#define UnusedB2_Pin GPIO_PIN_2
#define UnusedB2_GPIO_Port GPIOB
#define UnusedE8_Pin GPIO_PIN_8
#define UnusedE8_GPIO_Port GPIOE
#define UnusedE10_Pin GPIO_PIN_10
#define UnusedE10_GPIO_Port GPIOE
#define UnusedE12_Pin GPIO_PIN_12
#define UnusedE12_GPIO_Port GPIOE
#define UnusedB10_Pin GPIO_PIN_10
#define UnusedB10_GPIO_Port GPIOB
#define UnusedB12_Pin GPIO_PIN_12
#define UnusedB12_GPIO_Port GPIOB
#define UnusedA1_Pin GPIO_PIN_1
#define UnusedA1_GPIO_Port GPIOA
#define UnusedB0_Pin GPIO_PIN_0
#define UnusedB0_GPIO_Port GPIOB
#define UnusedB1_Pin GPIO_PIN_1
#define UnusedB1_GPIO_Port GPIOB
#define UnusedE7_Pin GPIO_PIN_7
#define UnusedE7_GPIO_Port GPIOE
#define UnusedE9_Pin GPIO_PIN_9
#define UnusedE9_GPIO_Port GPIOE
#define UnusedE11_Pin GPIO_PIN_11
#define UnusedE11_GPIO_Port GPIOE
#define UnusedE13_Pin GPIO_PIN_13
#define UnusedE13_GPIO_Port GPIOE
#define UnusedE14_Pin GPIO_PIN_14
#define UnusedE14_GPIO_Port GPIOE
#define UnusedE15_Pin GPIO_PIN_15
#define UnusedE15_GPIO_Port GPIOE

#endif /* BSP_SDK_CONFIG_H_ */
