/**
 * @file    pow.c
 * @author  Cypherock X1 Team
 * @brief   Proof of work handler for wallet unlock process
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * "Commons Clause" License Condition v1.0
 *
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "pow.h"

#include "application_startup.h"
#include "board.h"
#include "lvgl.h"
#include "pow_utilities.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/
extern Wallet wallet;

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define SECS_TO_HASHES(sec) ((sec)*pow_hash_rate)

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief saves data to flash memory and updates the text slideshow.

 * Purpose :
 * 1. Saves the current nonce in flash
 * This ensures that the progress in cracking the challenge is saved in flash.
 * If a user re-plugs the device his progress won't be lossed.
 *
 * 2. Update the approx time left to unlock in flash
 * The approx time to unlock is calculated when the challenge is fetched from
 * card. This timer decreases its value.
 *
 * @param p_context Used by app_timer module
 *
 * @return If the variable `pow_started` is false, then the function will return
 without performing any
 * further actions.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void pow_save_data_to_flash();

/**
 * @brief The function `pow_timer_handler` saves data to flash memory.
 * @details This is called periodically after POW_TIMER_MS milliseconds.
 *
 * @param task The parameter "task" is a pointer to the task structure that
 * represents the timer. It contains information about the timer, such as its
 * period, callback function, and other properties.
 */
static void pow_timer_handler(lv_task_t *task);

/**
 * The function checks if a hash value is smaller than a target value.
 *
 * @return a boolean value, which indicates whether the hash stored in the flash
 * wallet is smaller than the target hash.
 */
static bool hash_smaller_than_target();

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
/**
 * @note Every byte array is assumed to be in big endian form
 */
static uint8_t nonce[POW_NONCE_SIZE], hash[SHA256_SIZE];
static bool pow_started;
static SHA256_CTX sha2;
static Flash_Wallet *flash_wallet;    // Pointer to wallet which the device is
                                      // currently trying to unlock
static lv_task_t *pow_update_flash_task = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/
size_t pow_hash_rate = 1;

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void pow_save_data_to_flash() {
  if (!pow_started) {
    return;
  }

  // Defining them static so that memory does not get allocated on stack after
  // every call
  static char new_text[MAX_NUM_OF_CHARS_IN_A_SLIDE];
  static uint32_t new_time_to_unlock_in_secs;

  new_time_to_unlock_in_secs = flash_wallet->challenge.time_to_unlock_in_secs -
                               (uint32_t)(POW_TIMER_MS / 1000);

  if (new_time_to_unlock_in_secs >
      flash_wallet->challenge.time_to_unlock_in_secs) {
    // This means time has become negative
    new_time_to_unlock_in_secs = 0;
  }

  save_nonce_flash(
      (char *)flash_wallet->wallet_name, nonce, new_time_to_unlock_in_secs);
  convert_secs_to_time(
      new_time_to_unlock_in_secs, (char *)wallet.wallet_name, new_text);
  ui_text_slideshow_change_text(
      new_text, strnlen(new_text, MAX_NUM_OF_CHARS_IN_A_SLIDE), 0);
}

static void pow_timer_handler(lv_task_t *task) {
  pow_save_data_to_flash();
}

static bool hash_smaller_than_target() {
  return memcmp(flash_wallet->challenge.target, hash, SHA256_SIZE) > 0;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void pow_init_hash_rate() {
  uint8_t bytes_1[64] = {0};
  size_t start_time = uwTick, hashes = 8192;
  for (size_t i = 0; i < hashes; i++) {
    sha256_Raw(bytes_1, sizeof(bytes_1), bytes_1);
  }
  size_t duration = uwTick - start_time;
  pow_hash_rate = (hashes * 1000 / duration);

  // Adjust for 5% margin of error due to 50ms hard delay in the main event loop
  pow_hash_rate = (pow_hash_rate * 95 / 100);
}

void start_proof_of_work_task(const char *name) {
  if (get_flash_wallet_by_name(name, &flash_wallet) != SUCCESS_)
    return;

  // Set nonce = nonce in flash
  memcpy(nonce, flash_wallet->challenge.nonce, POW_NONCE_SIZE);
  pow_started = true;

  sha256_Init(&sha2);
  pow_update_flash_task =
      lv_task_create(pow_timer_handler, POW_TIMER_MS, LV_TASK_PRIO_MID, NULL);

  BSP_App_Timer_Start(BSP_POW_TIMER, POW_TIMER_MS);
  log_hex_array("hash", hash, sizeof(hash));
  log_hex_array("nonce", nonce, sizeof(nonce));
  log_hex_array("target",
                flash_wallet->challenge.target,
                sizeof(flash_wallet->challenge.target));
}

void stop_proof_of_work_task() {
  log_hex_array("hash", hash, sizeof(hash));
  log_hex_array("nonce", nonce, sizeof(nonce));
  log_hex_array("target",
                flash_wallet->challenge.target,
                sizeof(flash_wallet->challenge.target));
  lv_task_set_prio(pow_update_flash_task, LV_TASK_PRIO_OFF);
  lv_task_del(pow_update_flash_task);
  pow_update_flash_task = NULL;
  pow_save_data_to_flash();
  pow_started = false;
}

bool proof_of_work_task() {
  if (!pow_started) {
    return false;
  }

  bool result = false;

  /**
   * @brief LV task handler is required to update the display and run the task
   * which calls pow_timer_handler.
   */
  lv_task_handler();

  // The counter will run for x secs if limit = SECS_TO_HASHES(x)
  uint16_t limit = SECS_TO_HASHES(1);

  for (uint16_t counter = 0; counter < limit; counter++) {
    sha256_Init(&sha2);
    sha256_Update(
        &sha2, flash_wallet->challenge.random_number, POW_RAND_NUMBER_SIZE);
    sha256_Update(&sha2, nonce, POW_NONCE_SIZE);
    sha256_Final(&sha2, hash);

    // If target value found, update result and exit the flow
    if (hash_smaller_than_target()) {
      result = true;
      stop_proof_of_work_task();
      break;
    }

    increment_byte_array(nonce, POW_NONCE_SIZE);
  }

  return result;
}

uint8_t *get_proof_of_work_nonce() {
  return &nonce[0];
}
