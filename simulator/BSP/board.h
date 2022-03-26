/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOARD_H
#define __BOARD_H

#include <stdint.h>
#include "board.h"
#ifdef __cplusplus
 extern "C" {
#endif


typedef enum {
	COMM_OK,
	COMM_BUSY,
	COMM_ERR
}comStatus_t;

comStatus_t com_write(uint8_t *pData, uint16_t size);
comStatus_t com_read(uint8_t *pData, uint16_t size);


typedef enum {
	BSP_OK,
	BSP_DEBUG_ERR,
	BSP_EEPROM_ERR,
	BSP_MPU_MAIN_ERR,
	BSP_MPU_BOARD_ERR,
	BSP_FLASH_ERR,
	BSP_FLASH_CHECK_ERR,
} BSP_Status_t;

typedef struct
{
  uint32_t timeOutValue;
  void (* appTimerCb)(void);
} BSP_App_Timer_Callback_t;

typedef enum
{
  BSP_APPLICATION_TIMER,
  BSP_COM_TIMER,
  BSP_POW_TIMER,

} BSP_TimerId_t;

#define BSP_I2C_SPEED_FAST				0x00702991
#define BSP_I2C_SPEED_STANDARD			0x10909CEC

#define BSP_ATECC_I2C					I2C2
#define BSP_ATECC_I2C_MODE_FAST			BSP_I2C_SPEED_FAST
#define BSP_ATECC_I2C_MODE_STANDARD		BSP_I2C_SPEED_STANDARD
#define BSP_ATECC_I2C_GPIO_PORT			GPIOB
#define BSP_ATECC_I2C_SDA_PIN			GPIO_PIN_14
#define BSP_ATECC_I2C_SCL_PIN			GPIO_PIN_13
#define BSP_ATECC_I2C_GPIO_AF			GPIO_AF4_I2C2
#define BSP_ATECC_I2C_CLK_ENABLE		__HAL_RCC_I2C2_CLK_ENABLE()
#define BSP_ATECC_I2C_CLK_DISABLE		__HAL_RCC_I2C2_CLK_DISABLE()

#define BSP_APP_TIMER_MAX                   10U

//#define SS_Pin GPIO_PIN_4
//#define SS_GPIO_Port GPIOA
//#define PN532_RST_Pin GPIO_PIN_0
//#define PN532_RST_GPIO_Port GPIOB
//#define PN532_REQ_Pin GPIO_PIN_1
//#define PN532_REQ_GPIO_Port GPIOB


#define STM_ERROR_BASE_NUM      (0x0)       ///< Global error base
#define STM_ERROR_SDM_BASE_NUM  (0x1000)    ///< SDM error base
#define STM_ERROR_SOC_BASE_NUM  (0x2000)    ///< SoC error base
#define STM_ERROR_STK_BASE_NUM  (0x3000)    ///< STK error base
/** @} */

#define STM_SUCCESS                           (STM_ERROR_BASE_NUM + 0)  ///< Successful command
#define STM_ERROR_SVC_HANDLER_MISSING         (STM_ERROR_BASE_NUM + 1)  ///< SVC handler is missing
#define STM_ERROR_SOFTDEVICE_NOT_ENABLED      (STM_ERROR_BASE_NUM + 2)  ///< SoftDevice has not been enabled
#define STM_ERROR_INTERNAL                    (STM_ERROR_BASE_NUM + 3)  ///< Internal Error
#define STM_ERROR_NO_MEM                      (STM_ERROR_BASE_NUM + 4)  ///< No Memory for operation
#define STM_ERROR_NOT_FOUND                   (STM_ERROR_BASE_NUM + 5)  ///< Not found
#define STM_ERROR_NOT_SUPPORTED               (STM_ERROR_BASE_NUM + 6)  ///< Not supported
#define STM_ERROR_INVALID_PARAM               (STM_ERROR_BASE_NUM + 7)  ///< Invalid Parameter
#define STM_ERROR_INVALID_STATE               (STM_ERROR_BASE_NUM + 8)  ///< Invalid state, operation disallowed in this state
#define STM_ERROR_INVALID_LENGTH              (STM_ERROR_BASE_NUM + 9)  ///< Invalid Length
#define STM_ERROR_INVALID_FLAGS               (STM_ERROR_BASE_NUM + 10) ///< Invalid Flags
#define STM_ERROR_INVALID_DATA                (STM_ERROR_BASE_NUM + 11) ///< Invalid Data
#define STM_ERROR_DATA_SIZE                   (STM_ERROR_BASE_NUM + 12) ///< Data size exceeds limit
#define STM_ERROR_TIMEOUT                     (STM_ERROR_BASE_NUM + 13) ///< Operation timed out
#define STM_ERROR_NULL                        (STM_ERROR_BASE_NUM + 14) ///< Null Pointer
#define STM_ERROR_FORBIDDEN                   (STM_ERROR_BASE_NUM + 15) ///< Forbidden Operation
#define STM_ERROR_INVALID_ADDR                (STM_ERROR_BASE_NUM + 16) ///< Bad Memory Address
#define STM_ERROR_BUSY                        (STM_ERROR_BASE_NUM + 17) ///< Busy

#define BSP_GET_IRQ_STATUS()		HAL_GPIO_ReadPin(BSP_PN532_INT_PORT, BSP_PN532_INT_PIN)

typedef uint32_t ret_code_t;
void SystemClock_Config(void);
void BSP_GPIO_Init(uint32_t hardware_version);
void BSP_I2C1_Init(void);
void BSP_I2C2_Init(uint32_t timing);
void BSP_IWDG_Init(void);
void BSP_TIM2_Init(void);
void BSP_USART1_UART_Init(void);
void BSP_TIM3_Init(void);

void BSP_TIM3_Base_Start_IT(void);
void BSP_TIM2_PWM_Start(uint32_t buzzer_on_time);
void BSP_TIM6_Init(void);
void BSP_DelayCounterDec();

void BSP_LPUART1_UART_Init(uint32_t baudrate);
void BSP_LPUART1_UART_DeInit();
uint8_t BSP_LPUART1_UART_Write(uint8_t* data, uint16_t size);
uint8_t BSP_LPUART1_UART_Read(uint8_t* data, uint16_t size);
uint8_t BSP_LPUART1_UART_ClearReadBuff();
static inline uint8_t BSP_LPUART1_UART_SendByte(uint8_t byte){
	return BSP_LPUART1_UART_Write(&byte, 1);
}

uint8_t BSP_I2C1_IO_Write(uint16_t devAddress, uint8_t *pData , uint8_t size);
uint8_t BSP_I2C1_IO_Read(uint16_t devAddress, uint8_t *pData , uint8_t size);
uint8_t BSP_I2C2_IO_Write(uint16_t devAddress, uint8_t *pData , uint8_t size);
uint8_t BSP_I2C2_IO_Read(uint16_t devAddress, uint8_t *pData , uint8_t size);
void BSP_I2C1_IO_Read_MEM(uint16_t devAddress, uint16_t memAddress, uint8_t *pData , uint8_t size);
void BSP_I2C1_AddressScan(void);
void BSP_I2C2_AddressScan(void);
void BSP_ClearKeyPressed(void);
uint32_t BSP_GetKeyPressed(void);

void BSP_DelayMs(uint32_t delayValue);
void BSP_DelayUs(uint32_t delay);
/* ##### Node Non-volatile data Updation ##### */
#define NV_DATA_SECTOR_ADDR			((uint32_t*)0x08008000)

#define DEVICE_HARDWARE_VERSION		0x0000002

#define FIREWALL_CODE_SEGMENT_ADDR		0x08009004
#define FIREWALL_CODE_SEGMENT_SIZE		0xE000
#define FIREWALL_NVDATA_SEGMENT_ADDR	0x08017000
#define FIREWALL_NVDATA_SEGMENT_SIZE	0x1800

#define FIREWALL_BOOT_FLAGS_SECTOR_ADDR		0x08017000
#define FIREWALL_NVDATA_PUBLIC_KEYS_ADDR	0x08017800
#define FIREWALL_NVDATA_APP_KEYS_ADDR		0x08018000

//#define DEBUGGING_WITHOUT_BOOTLOADER

#ifndef	DEBUGGING_WITHOUT_BOOTLOADER
#define BOOT_FLAGS_SECTOR_ADDR_BASE		FIREWALL_BOOT_FLAGS_SECTOR_ADDR
#else
#define BOOT_FLAGS_SECTOR_ADDR_BASE	(0x080FF800)
#endif
#define NV_SYS_SECTOR_ADDR			((uint32_t*)0x0800C000)

/* Boot Flags Locations */
//#define BOOT_FLAGS_SECTOR_ADDR_BASE	0x0801F800
#define BOOT_FLAGS_SECTOR_ADDR	(uint32_t*)FIREWALL_BOOT_FLAGS_SECTOR_ADDR
#define FIRMWARE_UPGRADE_STATE	(__IO uint32_t)BOOT_FLAGS_SECTOR_ADDR
#define FIRMWARE_UPGRADE_MODE	(__IO uint32_t)(BOOT_FLAGS_SECTOR_ADDR + 1)
#define FW_VERSION				(__IO uint32_t)(BOOT_FLAGS_SECTOR_ADDR + 2)
#define HW_VERSION				(__IO uint32_t)(BOOT_FLAGS_SECTOR_ADDR + 3)
#define FW_START				(__IO uint32_t)(BOOT_FLAGS_SECTOR_ADDR + 4)
#define FW_SIZE					(__IO uint32_t)(BOOT_FLAGS_SECTOR_ADDR + 5)

#define APPLICATION_ADDRESS_BASE	(0x08022800)
#define APPLICATION_ADDRESS			((__IO uint64_t*)APPLICATION_ADDRESS_BASE)

#define UPGRADE_DEVICE_EMPTY	0xFFFFFFFF
#define UPGRADE_INITIATED		0x55555555
#define UPGRADE_COMPLETED		0x5A5A5A5A

#define	MODE_DEVICE_MISCONFIGURED -1
#define MODE_DEVICE_UPDATE	0

#define MAX_APPLICATION_FILE_SIZE	((0x08100000) - (APPLICATION_ADDRESS_BASE))
#define MAX_PUBLIC_KEYS	4

#define SEC_TRUE	0xAAAAAAAAU
#define	SEC_FALSE	0x00000000U

#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE 0x800
#endif

typedef struct {
	int32_t state;
	uint32_t fwVer;
//	uint32_t hwVer;
	uint32_t fwSize;
	uint32_t bootCount;
    uint32_t auth_state;
    uint32_t first_boot_on_update;
	uint8_t flashHash[32];
}upgradeInfo_t;


//typedef struct {
//	int32_t state;
//	int32_t mode;
//	uint32_t fwVer;
//	uint32_t hwVer;
//	uint32_t fwStart;
//	uint32_t fwSize;
//}upgradeInfo_t;
//
//
//#define	UPGRADE_INITIATED -1
//#define	UPGRADE_COMPLETED 0
//#define	UPGRADE_VALIDATED 1
//
//#define	UPGRADE_MODE_CONSOLE -1
//#define	UPGRADE_MODE_RF 0

#define UPGRADE_INFO_SIZE sizeof(upgradeInfo_t)
#define UPGRADE_INFO_WORD_SIZE ((UPGRADE_INFO_SIZE/sizeof(int32_t))+1)
#define __IO volatile /*!< Defines 'read / write' permissions */

#ifndef NULL
#define NULL                                            0U
#endif /* NULL */

void BSP_NonVolatileRead(uint32_t addr, uint32_t* srcAddr, uint32_t length);
BSP_Status_t BSP_NonVolatileWrite(uint32_t addr, uint32_t* srcAddr, uint32_t noOfWords);
BSP_Status_t BSP_NonVolatileErase(void);
BSP_Status_t BSP_BootAreaWrite(uint32_t addr, uint32_t* srcAddr, uint32_t noOfWords);
BSP_Status_t BSP_NVSysWrite(uint32_t addr, uint32_t* srcAddr, uint32_t noOfWords);
void BSP_NVSysRead(uint32_t addr, uint32_t* dstAddr, uint32_t length);
BSP_Status_t BSP_FlashSectorWrite(__IO uint32_t *dstAddr, const uint32_t* srcAddr, uint32_t noOfWords);
BSP_Status_t BSP_FlashSectorErase(uint32_t page_address, uint32_t noOfpages);

/* #### Random Number Generator ###  */
void BSP_RNG_Init(void);
BSP_Status_t BSP_RNG_Generate(uint32_t *random32bit);
void BSP_RNG_End(void);

uint32_t BSP_App_Timer_Create(uint8_t TimerId, void appTimerHandler(void));
void BSP_App_Timer_Start(uint8_t TimerId, uint32_t timeOutValue);
void BSP_App_Timer_Init();
void BSP_DebugPort_Write(uint8_t * data, uint8_t size);
void BSP_App_Timer_Run(void);
void BSP_App_Timer_Stop(uint8_t TimerId);

void BSP_reset(void);
#define IVT_OFFSET 0x10000
#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H */

