/**
 * @file    ${file_name}
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) ${YEAR} HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
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
#include "wallet_selector.h"

#include <string.h>

#include "flash_api.h"
#include "flash_struct.h"
#include "options.h"
#include "wallet.h"
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
static Flash_Wallet selected_wallet = {0};

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief
 *
 */
void deselect_wallet(void);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
void deselect_wallet(void) {
  memzero(&selected_wallet, sizeof(selected_wallet));
  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
bool select_wallet_with_index(uint8_t index) {
  /* Before doing any operation, clear the selected_wallet */
  deselect_wallet();

  /* Check if wallet at index exists or not and get it's state */
  if (false == wallet_is_filled(index, &selected_wallet.state)) {
    return false;
  }

  /* Get wallet details like wallet_name, wallet_id, wallet_info,
   * is_wallet_locked and card_states */
  memcpy(selected_wallet.wallet_name,
         (const char *)get_wallet_name(index),
         sizeof(selected_wallet.wallet_name));

  memcpy(selected_wallet.wallet_id,
         get_wallet_id(index),
         sizeof(selected_wallet.wallet_id));

  selected_wallet.wallet_info = get_wallet_info(index);
  selected_wallet.is_wallet_locked = is_wallet_locked(index);
  selected_wallet.cards_states = is_wallet_partial(index);

  return true;
}

const char *selected_wallet_get_name(void) {
  return (const char *)&(selected_wallet.wallet_name[0]);
}

const uint8_t *selected_wallet_get_wallet_id(void) {
  return &(selected_wallet.wallet_id[0]);
}

const uint8_t selected_wallet_get_state(void) {
  return selected_wallet.state;
}

const uint8_t selected_wallet_is_locked(void) {
  return selected_wallet.is_wallet_locked;
}

const uint8_t selected_wallet_is_partial(void) {
  return selected_wallet.cards_states == 15;
}

bool selected_wallet_has_pin(void) {
  if (!WALLET_IS_PIN_SET(selected_wallet.wallet_info)) {
    return false;
  }

  return true;
}

bool selected_wallet_has_passphrase(void) {
  if (!WALLET_IS_PASSPHRASE_SET(selected_wallet.wallet_info)) {
    return false;
  }

  return true;
}