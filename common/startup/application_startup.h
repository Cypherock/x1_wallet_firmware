/**
 * @file    application_startup.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
/**
 * @file application_startup.h
 * @author
 * @brief This file contains declarations of functions which handle application
 * level initialisation of system.
 * @version 0.1
 * @date 2020-05-23
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef APPLICATION_STARTUP_H
#define APPLICATION_STARTUP_H

#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

// #include "app_error.h"
// #include "app_timer.h"

#if USE_SIMULATOR == 0
#include "SSD1306.h"
#endif    // USE_SIMULATOR

#include "adafruit_pn532.h"
#include "buzzer.h"
#include "controller_main.h"
#include "flash_if.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lvgl.h"
#include "sec_flash.h"
#include "ui_common.h"

/**
 * @brief Interval defined to repeatedly call callback repeated_timer_handler
 *
 */
#define POLLING_TIME 10
/// Interval defined for user inactivity in a flow in milli seconds
#define INACTIVITY_TIME (300 * 1000)

#define ATECC_CFG_88_MASK 0x4C
#define ATECC_CFG_89_MASK 0x01

#define ATECC_CFG_88_MASK_OLD_PROV 44

typedef enum {
  provision_empty = 0,
  provision_incomplete = 1,
  provision_complete = 2,
  provision_v1_complete = 3
} provision_status_t;

extern uint8_t device_auth_flag;
extern bool main_app_ready;

/**
 * @brief This function should be called from main to setup application level
 * functionalities
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void application_init();

/**
 * @brief This function should be called to check for unverified / partial /
 * locked state of any wallet.
 *
 * It shows a delay screen before the main menu for each state of each wallet.
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void check_invalid_wallets();

/**
 * @brief This function should be called to check the boot count to display
 * message to users to check their cards.
 *
 * @details It checks if the number of time the device has been booted is 20 or
 * not. On 20 boots, this functions shows a delay screen with a message to check
 * the proper working of their cards
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void check_boot_count();

/**
 * @brief Resets inactivity timer. It is expected that this function is called
 * only if any activity is received. Currently activity is defined as any user
 * input on the keypad.
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void reset_inactivity_timer();

/**
 * @brief This function logs data from backup register when if a fault had
 * occured before device reset RTC backup register 1 denotes if Reset had
 * occurred due to assert or error hander Error handlers save the registers from
 * stack to RTC backup register 2 to 9
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void log_error_handler_faults();

/**
 * @brief Checks if there is any fault in prev boot cycle
 * @details
 *
 * @param
 *
 * @return bool
 * @retval true
 * @retval false
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
bool fault_in_prev_boot();

/**
 * @brief Resets registers that denote fault in prev boot cycle
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void handle_fault_in_prev_boot();

/**
 * @brief
 * @details
 *
 * @return uint8_t Provision status of the device
 * @retval 0    Not provisioned
 * @retval 1    Semi-provisioned (serial and IO-Protection Key present)
 * @retval 2    Fully provisioned
 * @retval 3    External auth configuration
 */
provision_status_t check_provision_status();

/**
 * @brief   fetch device serial and check if UID in the serial matches MCU UID
 * or not
 * @details
 *
 * @return uint32_t device serial fetch status or failure status
 * @retval 0    fetched successfully
 * @retval 1    device UID doesn't match with serial UID
 */
uint32_t get_device_serial();

/**
 * @brief Detect hardware related faults
 *
 * @since v1.0.0
 */
void device_hardware_check();

/**
 * @brief Handle cases if device is not provisioned
 *
 * @since v1.0.0
 */
void device_provision_check();

/**
 * @brief returns auth state
 * @return DEVICE_AUTHENTICATED if auth state is Authenticated and not first
 * boot after update else DEVICE_NOT_AUTHENTICATED
 */
device_auth_state device_auth_check();

/**
 * @brief Sets device state to restricted
 * @details device state is changed to restricted mode where desktop app
 * listener is replaced with __authentication_listener and device waits for
 * authentication to start
 *
 * @see __authentication_listener
 *  */
void restrict_app();

#endif