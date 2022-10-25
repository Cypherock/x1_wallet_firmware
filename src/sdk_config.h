/**
 * @file    sdk_config.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
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
 *  Patch (2 byte):- Changes with hardware iterations like GPIO change, peripheral change.
 *  While firmware upgrade, bootloader will ignore patch changes and compare the major and minor only.
 */
#define DEVICE_HARDWARE_STM32_2     0x00010002
#define DEVICE_HARDWARE_STM32_3     0x00010003

#define ATECC_MODE_I2C2  1
#define ATECC_MODE_I2C2_ALT  2
#define ATECC_MODE_SWI  3

typedef uint8_t atecc_interface_type;

extern atecc_interface_type atecc_mode; 

#define BSP_USB_VID		0x3503
#if (X1WALLET_INITIAL == 1)
#define BSP_USB_PID		0x0102  ///PID Wallet X1[8:15]:0x01, HARDWARE version Major[4:7]:0x0, Initial[0:3]:0x2
#elif (X1WALLET_MAIN == 1)
#define BSP_USB_PID		0x0103  ///PID Wallet X1[8:15]:0x01, HARDWARE version Major[4:7]:0x0, Main[0:3]:0x3
#endif

#define BSP_I2C_SPEED_FAST				0x00702991
#define BSP_I2C_SPEED_STANDARD			0x10909CEC

#define BSP_ATECC_I2C					I2C2
#define BSP_ATECC_I2C_MODE_FAST			BSP_I2C_SPEED_FAST
#define BSP_ATECC_I2C_MODE_STANDARD		BSP_I2C_SPEED_STANDARD
#define BSP_ATECC_I2C_GPIO_PORT			GPIOB
#define BSP_ATECC_I2C_SDA_PIN			GPIO_PIN_14
#define BSP_ATECC_I2C_SCL_PIN			GPIO_PIN_13
#define BSP_ATECC_I2C_SDA_PIN_ALT       GPIO_PIN_11
#define BSP_ATECC_I2C_SCL_PIN_ALT		GPIO_PIN_10
#define BSP_ATECC_I2C_GPIO_AF			GPIO_AF4_I2C2
#define BSP_ATECC_I2C_CLK_ENABLE		__HAL_RCC_I2C2_CLK_ENABLE()
#define BSP_ATECC_I2C_CLK_DISABLE		__HAL_RCC_I2C2_CLK_DISABLE()
#define BSP_ATECC_SWI_UART              LPUART1
#define BSP_ATECC_SWI_UART_GPIO_PORT    GPIOB
#define BSP_ATECC_SWI_UART_GPIO_PIN     GPIO_PIN_11
#define BSP_ATECC_SWI_UART_CLK_ENABLE   __HAL_RCC_LPUART1_CLK_ENABLE()
#define BSP_ATECC_SWI_UART_CLK_DISABLE  __HAL_RCC_LPUART1_CLK_DISABLE()

#define BSP_PN532_OLED_I2C				I2C1
#define BSP_PN532_OLED_I2C_SPEED		BSP_I2C_SPEED_STANDARD
#define BSP_PN532_OLED_I2C_GPIO_PORT	GPIOB
#define BSP_PN532_OLED_I2C_SDA_PIN		GPIO_PIN_7
#define BSP_PN532_OLED_I2C_SCL_PIN		GPIO_PIN_6
#define BSP_PN532_OLED_I2C_GPIO_AF		GPIO_AF4_I2C1
#define BSP_PN532_OLED_I2C_CLK_ENABLE	__HAL_RCC_I2C1_CLK_ENABLE()
#define BSP_PN532_OLED_I2C_CLK_DISABLE	__HAL_RCC_I2C1_CLK_DISABLE()

#define BSP_PN532_RST_PORT				GPIOB
#define BSP_PN532_RST_PIN				GPIO_PIN_5
#define BSP_PN532_INT_PORT				GPIOB
#define BSP_PN532_INT_PIN				GPIO_PIN_4

#define BSP_JOYSTICK_GPIO_PORT			GPIOC
#define BSP_JOYSTICK_UP_PIN				GPIO_PIN_5
#define BSP_JOYSTICK_DOWN_PIN			GPIO_PIN_6
#define BSP_JOYSTICK_RIGHT_PIN			GPIO_PIN_4
#define BSP_JOYSTICK_LEFT_PIN			GPIO_PIN_8
#define BSP_JOYSTICK_ENTER_PIN			GPIO_PIN_7
#define BSP_JOYSTICK_GPIO_PULL(ver)     (ver == DEVICE_HARDWARE_STM32_2?GPIO_PULLUP:GPIO_PULLDOWN)
#define BSP_JOYSTICK_GPIO_MODE(ver)     (ver == DEVICE_HARDWARE_STM32_2?GPIO_MODE_IT_FALLING:GPIO_MODE_IT_RISING)
#define BSP_JOYSTICK_READ_STATE(ver)    (ver == DEVICE_HARDWARE_STM32_2?GPIO_PIN_RESET:GPIO_PIN_SET)

#define BSP_BUZZER_TIMER				TIM2
#define BSP_BUZZER_PWM_FREQ				4000
#define BSP_BUZZER_TIMER_PRESCALAR		0
#define BSP_BUZZER_TIMER_PERIOD			19999
#define BSP_BUZZER_TIMER_COUNTER		9999
#define BSP_BUZZER_GPIO_PORT			GPIOA
#define BSP_BUZZER_GPIO_PIN				GPIO_PIN_0
#define BSP_BUZZER_PWM_CHANNEL			TIM_CHANNEL_1
#define BSP_BUZZER_TIMER_CLK_ENABLE		__HAL_RCC_TIM2_CLK_ENABLE()
#define BSP_BUZZER_TIMER_CLK_DISABLE	__HAL_RCC_TIM2_CLK_DISABLE()



#endif /* BSP_SDK_CONFIG_H_ */
