/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOARD_H
#define __BOARD_H

#include <stdint.h>

#include "mem_config.h"
#include "sdk_config.h"
#include "time.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum { COMM_OK, COMM_BUSY, COMM_ERR } comStatus_t;

typedef enum {
  BSP_OK,
  BSP_DEBUG_ERR,
  BSP_EEPROM_ERR,
  BSP_MPU_MAIN_ERR,
  BSP_MPU_BOARD_ERR,
  BSP_FLASH_ERR,
  BSP_FLASH_CHECK_ERR,
  BSP_USB_ERR,
  BSP_TIMEOUT_ERR,
  BSP_BUSY_ERR,
  BSP_RNG_ZERO_ERROR,
  BSP_RNG_HW_ERROR
} BSP_Status_t;

typedef enum {
  BSP_APPLICATION_TIMER,
  BSP_COM_TIMER,
  BSP_POW_TIMER,

} BSP_TimerId_t;

typedef struct {
  uint32_t timeOutValue;
  void (*appTimerCb)(void);
} BSP_App_Timer_Callback_t;

typedef struct {
  int32_t state;
  uint32_t fwVer;
  uint32_t fwSize;
  uint32_t
      auth_state;    ///< Device Authentication state; Interpreted as
                     ///< "whether device is authenticated or not?";
                     ///< Indicated by ENUM device_auth_state (32-bit value)
  uint32_t first_boot_on_update;    ///< 0 - not first boot after update, 1 -
                                    ///< first boot after update
  uint8_t flashHash[32];
} upgradeInfo_t;

typedef enum { SUCCESS = 0, ERROR = !SUCCESS } ErrorStatus;

extern uint8_t STM32_UID[12];

#define UID_BASE (&STM32_UID[0])
#define __IO
#define uwTick clock()
#define NVIC_EnableIRQ(a) 0
#define NVIC_DisableIRQ(a) 0
#define NVIC_GetEnableIRQ(a) 0
#define OTG_FS_IRQHandler()
#define FLASH_PAGE_SIZE ((uint32_t)0x800)

#define READ_JOYSTICK(PinMask)                                                 \
  (READ_BIT(BSP_JOYSTICK_GPIO_PORT->IDR, PinMask) == (PinMask))

#define BSP_APP_TIMER_MAX 10U

#define STM_ERROR_BASE_NUM (0x0)           ///< Global error base
#define STM_ERROR_SDM_BASE_NUM (0x1000)    ///< SDM error base
#define STM_ERROR_SOC_BASE_NUM (0x2000)    ///< SoC error base
#define STM_ERROR_STK_BASE_NUM (0x3000)    ///< STK error base
/** @} */

#define STM_SUCCESS (STM_ERROR_BASE_NUM + 0)    ///< Successful command
#define STM_ERROR_SVC_HANDLER_MISSING                                          \
  (STM_ERROR_BASE_NUM + 1)    ///< SVC handler is missing
#define STM_ERROR_SOFTDEVICE_NOT_ENABLED                                       \
  (STM_ERROR_BASE_NUM + 2)    ///< SoftDevice has not been enabled
#define STM_ERROR_INTERNAL (STM_ERROR_BASE_NUM + 3)    ///< Internal Error
#define STM_ERROR_NO_MEM                                                       \
  (STM_ERROR_BASE_NUM + 4)    ///< No Memory for operation
#define STM_ERROR_NOT_FOUND (STM_ERROR_BASE_NUM + 5)        ///< Not found
#define STM_ERROR_NOT_SUPPORTED (STM_ERROR_BASE_NUM + 6)    ///< Not supported
#define STM_ERROR_INVALID_PARAM                                                \
  (STM_ERROR_BASE_NUM + 7)    ///< Invalid Parameter
#define STM_ERROR_INVALID_STATE                                                \
  (STM_ERROR_BASE_NUM +                                                        \
   8)    ///< Invalid state, operation disallowed in this state
#define STM_ERROR_INVALID_LENGTH (STM_ERROR_BASE_NUM + 9)    ///< Invalid Length
#define STM_ERROR_INVALID_FLAGS (STM_ERROR_BASE_NUM + 10)    ///< Invalid Flags
#define STM_ERROR_INVALID_DATA (STM_ERROR_BASE_NUM + 11)     ///< Invalid Data
#define STM_ERROR_DATA_SIZE                                                    \
  (STM_ERROR_BASE_NUM + 12)    ///< Data size exceeds limit
#define STM_ERROR_TIMEOUT (STM_ERROR_BASE_NUM + 13)    ///< Operation timed out
#define STM_ERROR_NULL (STM_ERROR_BASE_NUM + 14)       ///< Null Pointer
#define STM_ERROR_FORBIDDEN                                                    \
  (STM_ERROR_BASE_NUM + 15)    ///< Forbidden Operation
#define STM_ERROR_INVALID_ADDR                                                 \
  (STM_ERROR_BASE_NUM + 16)                         ///< Bad Memory Address
#define STM_ERROR_BUSY (STM_ERROR_BASE_NUM + 17)    ///< Busy

#define FLASH_END (0x080FFFFFUL) /*!< FLASH END address               */

#define BSP_GET_IRQ_STATUS()                                                   \
  ((READ_BIT(BSP_PN532_INT_PORT->IDR, BSP_PN532_INT_PIN) ==                    \
    (BSP_PN532_INT_PIN))                                                       \
       ? 1UL                                                                   \
       : 0UL)    // HAL_GPIO_ReadPin(BSP_PN532_INT_PORT, BSP_PN532_INT_PIN)

#define DEVICE_HARDWARE_VERSION ((BSP_USB_PID & 0xFF00) >> 8)

#define UPGRADE_DEVICE_EMPTY 0xFFFFFFFF
#define UPGRADE_INITIATED 0x55555555
#define UPGRADE_COMPLETED 0x5A5A5A5A

#define MODE_DEVICE_MISCONFIGURED -1
#define MODE_DEVICE_UPDATE 0

#define SEC_TRUE 0xAAAAAAAAU
#define SEC_FALSE 0x00000000U

#define UPGRADE_INFO_SIZE sizeof(upgradeInfo_t)
#define UPGRADE_INFO_WORD_SIZE ((UPGRADE_INFO_SIZE / sizeof(int32_t)) + 1)

comStatus_t com_write(uint8_t *pData, uint16_t size);
comStatus_t com_read(uint8_t *pData, uint16_t size);

extern uint32_t joystick_pressed;
typedef uint32_t ret_code_t;
void SystemClock_Config(void);
void BSP_GPIO_Init(uint32_t hardware_version);
void BSP_I2C1_Init(void);
void BSP_I2C2_Init(uint32_t timing);
void BSP_I2C2_DeInit();
void BSP_TIM2_Init(void);
void BSP_TIM3_Init(void);

void BSP_TIM3_Base_Start_IT(void);
void BSP_TIM2_PWM_Start(uint32_t buzzer_on_time);
void BSP_TIM6_Init(void);
void BSP_DelayCounterDec();
void BSP_USB_Clock_Init();

void BSP_LPUART1_UART_Init(uint32_t baudrate);
void BSP_LPUART1_UART_DeInit();
uint8_t BSP_LPUART1_UART_Write(uint8_t *data, uint16_t size);
uint8_t BSP_LPUART1_UART_Read(uint8_t *data, uint16_t size);
uint8_t BSP_LPUART1_UART_ClearReadBuff();
static inline uint8_t BSP_LPUART1_UART_SendByte(uint8_t byte) {
  return BSP_LPUART1_UART_Write(&byte, 1);
}
uint8_t BSP_I2C1_IO_Write(uint16_t devAddress, uint8_t *pData, uint8_t size);
uint8_t BSP_I2C1_IO_Read(uint16_t devAddress, uint8_t *pData, uint8_t size);
uint8_t BSP_I2C2_IO_Write(uint16_t devAddress, uint8_t *pData, uint8_t size);
uint8_t BSP_I2C2_IO_Read(uint16_t devAddress, uint8_t *pData, uint8_t size);
void BSP_I2C1_IO_Read_MEM(uint16_t devAddress,
                          uint16_t memAddress,
                          uint8_t *pData,
                          uint8_t size);
void BSP_I2C1_AddressScan(void);
void BSP_I2C2_AddressScan(void);
void BSP_ClearKeyPressed(void);
uint32_t BSP_GetKeyPressed(void);

void BSP_DelayMs(uint32_t delayValue);
void BSP_DelayUs(uint32_t delay);

void BSP_NonVolatileRead(uint32_t addr, uint32_t *srcAddr, uint32_t length);
BSP_Status_t BSP_FlashSectorWrite(uint32_t *dstAddr,
                                  const uint32_t *srcAddr,
                                  uint32_t noOfbytes);
BSP_Status_t BSP_FlashSectorErase(uint32_t page_address, uint32_t noOfpages);

/* #### Random Number Generator ###  */
void BSP_RNG_Init(void);
BSP_Status_t BSP_RNG_Generate(uint32_t *random32bit);
void BSP_RNG_End(void);

uint32_t BSP_App_Timer_Create(uint8_t TimerId, void appTimerHandler(void));
void BSP_App_Timer_Start(uint8_t TimerId, uint32_t timeOutValue);
void BSP_App_Timer_Init();
void BSP_App_Timer_Run(void);
void BSP_App_Timer_Stop(uint8_t TimerId);

void BSP_reset(void);
void BSP_sysClkDisable(void);
uint32_t read_hw_gpio_config();
#define IVT_OFFSET 0x10000
#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H */
