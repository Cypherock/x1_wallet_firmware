/**
 * @file    export_wallets.c
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
#include "flash_api.h"
#include "manager_api.h"
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
 * @brief Fills wallet list in structure type
 * manager_get_wallets_result_response_t
 *
 * @param resp The reference to the structure which needs to be populated
 */
static void fill_wallet_list(manager_get_wallets_result_response_t *resp);

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

static void fill_wallet_list(manager_get_wallets_result_response_t *resp) {
  const char *wallet_list[MAX_WALLETS_ALLOWED] = {0};

  resp->wallet_list_count = get_wallet_list(&wallet_list[0]);

  manager_wallet_item_t *list = &resp->wallet_list[0];

  uint8_t index = 0;
  for (index = 0; index < resp->wallet_list_count; index++) {
    uint8_t flash_index;
    get_index_by_name(wallet_list[index], &flash_index);

    snprintf(
        list[index].name, sizeof(list[index].name), "%s", wallet_list[index]);

    memcpy(list[index].id, get_wallet_id(flash_index), WALLET_ID_SIZE);

    uint8_t wallet_info = get_wallet_info(flash_index);
    list[index].has_passphrase = WALLET_IS_PASSPHRASE_SET(wallet_info);
    list[index].has_pin = WALLET_IS_PIN_SET(wallet_info);

    Flash_Wallet *flash_wallet = NULL;
    get_flash_wallet_by_name(wallet_list[index], &flash_wallet);

    ASSERT(NULL != flash_wallet);

    // Wallet is NOT in usable state if
    // 1. It is in locked state in any of the X1 cards
    // 2. Wallet state in X1 Vault flash is not VALID_WALLET
    // 3. Card state in X1 Vault flash is not 0xF
    if (flash_wallet->is_wallet_locked || VALID_WALLET != flash_wallet->state ||
        0x0F != flash_wallet->cards_states) {
      list[index].is_valid = false;
    } else {
      list[index].is_valid = true;
    }
  }

  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void manager_export_wallets(manager_query_t *query) {
  if (!check_which_request(query, MANAGER_GET_WALLETS_REQUEST_INITIATE_TAG)) {
    return;
  }

  manager_result_t result = init_manager_result(MANAGER_RESULT_GET_WALLETS_TAG);
  result.get_wallets.which_response = MANAGER_GET_WALLETS_RESPONSE_RESULT_TAG;
  fill_wallet_list(&result.get_wallets.result);

  manager_send_result(&result);
  return;
}