/**
 * @file    wallet_list.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
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
#include "wallet_list.h"

#include <string.h>

#include "flash_api.h"

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
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
uint8_t get_wallet_list(const char *wallet_list[]) {
  uint8_t num_wallets = 0;
  if (NULL == wallet_list) {
    return num_wallets;
  }

  for (uint8_t wallet_idx = 0; wallet_idx < MAX_WALLETS_ALLOWED; wallet_idx++) {
    /* Only append to the wallet list if the wallet is filled - regardless of
     * it's state */
    if (true != wallet_is_filled(wallet_idx, NULL)) {
      continue;
    }

    /* Point to the wallet name on wallet_list at index num_wallets */
    wallet_list[num_wallets] = (char *)get_wallet_name(wallet_idx);
    num_wallets++;
  }

  return num_wallets;
}

uint8_t get_filled_wallet_meta_data_list(wallet_list_t *list) {
  uint8_t count = 0;
  if (NULL == list) {
    return count;
  }

  for (uint8_t wallet_idx = 0; wallet_idx < MAX_WALLETS_ALLOWED; wallet_idx++) {
    if (!wallet_is_filled_with_share(wallet_idx)) {
      continue;
    }

    memcpy(list->wallet[count].name, get_wallet_name(wallet_idx), NAME_SIZE);
    memcpy(list->wallet[count].id, get_wallet_id(wallet_idx), WALLET_ID_SIZE);
    count++;
  }

  list->count = count;
  return count;
}

bool get_wallet_data_by_id(const uint8_t *wallet_id,
                           Wallet *wallet,
                           rejection_cb *reject_cb) {
  if ((NULL == wallet_id) || (NULL == wallet)) {
    return false;
  }

  for (uint8_t wallet_idx = 0; wallet_idx < MAX_WALLETS_ALLOWED; wallet_idx++) {
    wallet_state state = INVALID_WALLET;
    if (!wallet_is_filled(wallet_idx, &state)) {
      continue;
    }

    if (0 == memcmp(wallet_id, get_wallet_id(wallet_idx), WALLET_ID_SIZE)) {
      // Check if wallet is in usable state or not
      if ((VALID_WALLET != state) || is_wallet_locked(wallet_idx) ||
          is_wallet_partial(wallet_idx)) {
        if (reject_cb) {
          reject_cb(ERROR_COMMON_ERROR_WALLET_PARTIAL_STATE_TAG,
                    ERROR_WALLET_PARTIAL_STATE_UNKNOWN);
        }
        return false;
      } else {
        memcpy(wallet->wallet_id, wallet_id, WALLET_ID_SIZE);
        memcpy(wallet->wallet_name, get_wallet_name(wallet_idx), NAME_SIZE);
        wallet->wallet_info = get_wallet_info(wallet_idx);
        return true;
      }
    }
  }

  // If control reaches here, that means `wallet_id` search failed
  if (reject_cb) {
    reject_cb(ERROR_COMMON_ERROR_WALLET_NOT_FOUND_TAG,
              ERROR_WALLET_NOT_FOUND_UNKNOWN);
  }

  return false;
}

bool get_wallet_name_by_id(const uint8_t *wallet_id,
                           uint8_t *wallet_name,
                           rejection_cb *reject_cb) {
  Wallet wallet = {0};
  if (!get_wallet_data_by_id(wallet_id, &wallet, reject_cb)) {
    return false;
  }

  if (wallet_name) {
    memcpy(wallet_name, wallet.wallet_name, NAME_SIZE);
  }

  return true;
}