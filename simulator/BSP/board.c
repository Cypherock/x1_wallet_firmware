
#include "board.h"

#include <SDL.h>
#include <stdio.h>

#include "flash.h"
#include "lvgl.h"
#include "sim_usb.h"
#include "stdlib.h"

uint32_t buzzer_counter = 0;
uint8_t STM32_UID[12] = {0};

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
void BSP_I2C1_Init(void) {
}

void BSP_LPUART1_UART_Init(uint32_t baudrate) {
}
void BSP_LPUART1_UART_DeInit() {
}
uint8_t BSP_LPUART1_UART_Write(uint8_t *data, uint16_t size) {
  return 1;
}
uint8_t BSP_LPUART1_UART_Read(uint8_t *data, uint16_t size) {
  return 1;
}
uint8_t BSP_LPUART1_UART_ClearReadBuff() {
  return 1;
}

uint8_t BSP_I2C1_IO_Write(uint16_t devAddress, uint8_t *pData, uint8_t size) {
  uint8_t status = 0;
  return status;
}

uint8_t BSP_I2C1_IO_Read(uint16_t devAddress, uint8_t *pData, uint8_t size) {
  uint8_t status = 0;
  return status;
}

void BSP_I2C1_IO_Read_MEM(uint16_t devAddress,
                          uint16_t memAddress,
                          uint8_t *pData,
                          uint8_t size) {
}

void BSP_I2C1_AddressScan(void) {
}

/**
 * @brief I2C2 Initialization Function
 * @param None
 * @retval None
 */
void BSP_I2C2_Init(uint32_t timing) {
}

uint8_t BSP_I2C2_IO_Write(uint16_t devAddress, uint8_t *pData, uint8_t size) {
  uint8_t status = 0;
  return status;
}

uint8_t BSP_I2C2_IO_Read(uint16_t devAddress, uint8_t *pData, uint8_t size) {
  uint8_t status = 0;
  return status;
}

void BSP_I2C2_AddressScan(void) {
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
void BSP_TIM2_Init(void) {
}

void BSP_TIM3_Base_Start_IT() {
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
void BSP_TIM3_Init(void) {
}

void BSP_TIM6_Base_Start_IT(void) {
}

void BSP_TIM6_Init(void) {
}

uint32_t delay_us_counter = 0;
void BSP_DelayCounterDec() {
}

void BSP_DelayUs(uint32_t delay) {
}

void BSP_TIM2_PWM_Start(uint32_t buzzer_on_time) {
}

void BSP_TIM2_PWM_Stop(void) {
}
/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
void BSP_GPIO_Init(uint32_t hardware_version) {
}

uint32_t Key_Pressed = 0;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
}

uint32_t BSP_GetKeyPressed(void) {
  return 0;
}

void BSP_ClearKeyPressed(void) {
}
static uint32_t GetBank(uint32_t Addr);
static uint32_t GetPage(uint32_t Addr);
/**
 * Read word from Non-Volatile memory
 */
void BSP_NonVolatileRead(uint32_t addr, uint32_t *dstAddr, uint32_t length) {
  read_file(addr, dstAddr, length);
}

BSP_Status_t BSP_FlashSectorErase(uint32_t page_address, uint32_t noOfpages) {
  BSP_Status_t status = BSP_OK;
  if (!erase_file(page_address, noOfpages))
    status = BSP_FLASH_ERR;
  return status;
}

BSP_Status_t BSP_FlashSectorWrite(uint32_t *dstAddr,
                                  const uint32_t *srcAddr,
                                  uint32_t noOfWords) {
  BSP_Status_t status = BSP_OK;
  if (!write_file(dstAddr, srcAddr, noOfWords))
    status = BSP_FLASH_ERR;
  return status;
}

/**
 * Program non-volatile memory
 */
BSP_Status_t BSP_NonVolatileWrite(uint32_t addr,
                                  uint32_t *srcAddr,
                                  uint32_t noOfWords) {
  BSP_Status_t status = BSP_FLASH_ERR;

  status = BSP_FlashSectorErase(addr, 1);
  if (status != BSP_OK)
    return status;
  status = BSP_FLASH_ERR;
  status = BSP_FlashSectorWrite((uint32_t *)addr, srcAddr, noOfWords);
  if (status != BSP_OK)
    return status;
  return status;
}

void BSP_reset(void) {
}

void BSP_DelayMs(uint32_t delayValue) {
  SDL_Delay(delayValue);
}

void BSP_Buzzer_Timer() {
}

void BSP_RNG_Init(void) {
}

BSP_Status_t BSP_RNG_Generate(uint32_t *random32bit) {
#ifdef USE_SIMULATOR
  *random32bit = rand();
#endif    // USE_SIMULATOR

  return BSP_OK;
}

void BSP_RNG_End(void) {
}

/* user callback function for systick Handler */
void HAL_SYSTICK_Callback(void) {
}

BSP_App_Timer_Callback_t appTimerList[BSP_APP_TIMER_MAX];
static uint8_t appTimerRegisterCount = 0;
static uint8_t appTimerRunCount = 0;
static uint32_t systickCounter = 0;

void BSP_App_Timer_Init() {
}

uint32_t BSP_App_Timer_Create(uint8_t TimerId, void appTimerHandler(void)) {
  return 1;
}

void BSP_App_Timer_Start(uint8_t TimerId, uint32_t timeOutValue) {
}

void BSP_App_Timer_Stop(uint8_t TimerId) {
}

void BSP_App_Timer_Run(void) {
}

/**
 * @brief  Gets the page of a given address
 * @param  Addr: Address of the FLASH Memory
 * @retval The page of a given address
 */
static uint32_t GetPage(uint32_t Addr) {
  uint32_t page = 0;
  return page;
}

/**
 * @brief  Gets the bank of a given address
 * @param  Addr: Address of the FLASH Memory
 * @retval The bank of a given address
 */
static uint32_t GetBank(uint32_t Addr) {
  uint32_t bank = 0;
  return bank;
}

void BSP_sysClkDisable(void) {
}

uint32_t read_hw_gpio_config() {
  return DEVICE_HARDWARE_STM32_2;
  // return DEVICE_HARDWARE_STM32_3;
}
