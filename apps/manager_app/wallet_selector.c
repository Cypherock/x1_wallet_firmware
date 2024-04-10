/**
 * @file    wallet_selector.c
 * @author  Cypherock X1 Team
 * @brief   Exports list of existing wallets to the host
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
#include "events.h"
#include "flash_api.h"
#include "manager_api.h"
#include "ui_core_confirm.h"
#include "wallet_list.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host manager app and return false.
 *
 * @param query Reference to an instance of manager_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const manager_query_t *query,
                                pb_size_t which_request);

/**
 * @brief The functions presents wallet list to the user & waits for input.
 * @details The function waits for user action and returns true if the
 * selections was completed by the user. In case if user chooses a wallet
 * that is unusable, the function will convey the situation to the user &
 * prompt the user to choose a wallet again.
 *
 * @param flash_wallet_idx Reference to store the selected wallet index as on
 * flash
 *
 * @return bool Indicating if the user chose a valid wallet to continue with
 * @retval true If user chose a valid wallet
 * @retval false If user rejected, timeout happened, or abort received.
 */
// static bool choose_wallet(uint8_t *flash_wallet_idx);

/**
 * @brief Fills wallet list in structure type manager_wallet_item_t with
 * details of the provided wallet_idx on device storage.
 *
 * @param wallet a Pointer to the structure to be filled
 * @param wallet_idx Index of wallet in storage whose details are to be copied
 */
static void fill_wallet(manager_wallet_item_t *wallet, uint8_t wallet_idx);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool check_which_request(const manager_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->get_wallets.which_request) {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

// static bool choose_wallet(uint8_t *flash_wallet_idx) {

// }

static void fill_wallet(manager_wallet_item_t *wallet, uint8_t wallet_idx) {
  const Flash_Wallet *flash_wallet = get_wallet_by_index(wallet_idx);
  ASSERT(flash_wallet != NULL);

  snprintf(wallet->name, sizeof(wallet->name), "%s", flash_wallet->wallet_name);
  memcpy(wallet->id, flash_wallet->wallet_id, WALLET_ID_SIZE);

  uint8_t wallet_info = flash_wallet->wallet_info;
  wallet->has_passphrase = WALLET_IS_PASSPHRASE_SET(wallet_info);
  wallet->has_pin = WALLET_IS_PIN_SET(wallet_info);

  ASSERT(NULL != flash_wallet);

  // Wallet is NOT in usable state if
  // 1. It is in locked state in any of the X1 cards
  // 2. Wallet state in X1 Vault flash is not VALID_WALLET
  // 3. Card state in X1 Vault flash is not 0xF
  if (flash_wallet->is_wallet_locked || VALID_WALLET != flash_wallet->state ||
      0x0F != flash_wallet->cards_states) {
    wallet->is_valid = false;
  } else {
    wallet->is_valid = true;
  }
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void manager_wallet_selector(manager_query_t *query) {
  if (!check_which_request(query, MANAGER_SELECT_WALLET_REQUEST_INITIATE_TAG)) {
    return;
  }

  /* Create an array of pointers holding the string to display on the menu. */
  const char *options[MAX_WALLETS_ALLOWED] = {0};
  static uint16_t choice_idx = 0;
  uint8_t option_count = get_wallet_list(&options[0]);

  while (true) {
    menu_init_with_choice(
        options, option_count, "Wallet Select", choice_idx, true);

    evt_status_t events = get_events(EVENT_CONFIG_UI, MAX_INACTIVITY_TIMEOUT);
    if (true == events.p0_event.flag) {
      // core will handle p0 events, exit now
      break;
    }
    if (UI_EVENT_REJECT == events.ui_event.event_type) {
      // user rejected, send error and exit
      manager_send_error(ERROR_COMMON_ERROR_USER_REJECTION_TAG,
                         ERROR_USER_REJECTION_CONFIRMATION);
      break;
    }

    uint8_t flash_wallet_idx = 0;
    choice_idx = events.ui_event.list_selection - 1;

    ASSERT(UI_EVENT_LIST_CHOICE == events.ui_event.event_type);
    ASSERT(SUCCESS ==
           get_index_by_name(options[choice_idx], &flash_wallet_idx));
    if (get_wallet_data_by_id(get_wallet_id(flash_wallet_idx), &wallet, NULL)) {
      manager_result_t result =
          init_manager_result(MANAGER_RESULT_SELECT_WALLET_TAG);
      result.select_wallet.which_response =
          MANAGER_SELECT_WALLET_RESPONSE_RESULT_TAG;
      result.select_wallet.result.has_wallet = true;
      fill_wallet(&result.select_wallet.result.wallet, flash_wallet_idx);

      manager_send_result(&result);
      break;
    } else {
      bool retry_choice =
          core_scroll_page(NULL, ui_text_wallet_selector_invalid, NULL);

      if (false == retry_choice) {
        // user selected to choose a different wallet
        manager_send_error(ERROR_COMMON_ERROR_USER_REJECTION_TAG,
                           ERROR_USER_REJECTION_CONFIRMATION);
        break;
      }
    }
  }

  return;
}