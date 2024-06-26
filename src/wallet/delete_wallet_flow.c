/**
 * @file    delete_wallet_flow.c
 * @author  Cypherock X1 Team
 * @brief   Flow for delete wallet operation on an existing wallet
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
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
#include "card_flow_delete_wallet.h"
#include "constant_texts.h"
#include "core_error.h"
#include "flash_api.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "tasks.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "ui_state_machine.h"
#include "wallet.h"
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

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/
extern char *ALPHA_NUMERIC;

extern Wallet wallet;
extern Wallet_credential_data wallet_credential_data;
/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool get_pin_input() {
  if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
    return true;
  }

  memzero(&wallet_credential_data, sizeof(wallet_credential_data));
  memzero(wallet.password_double_hash, sizeof(wallet.password_double_hash));

  input_text_init(ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
  if (0 == get_state_on_input_scr(0, 1, 2)) {
    sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
               strnlen(flow_level.screen_input.input_text,
                       sizeof(flow_level.screen_input.input_text)),
               wallet_credential_data.password_single_hash);
    sha256_Raw(wallet_credential_data.password_single_hash,
               SHA256_DIGEST_LENGTH,
               wallet.password_double_hash);

    memzero(flow_level.screen_input.input_text,
            sizeof(flow_level.screen_input.input_text));
    return true;
  }

  return false;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void delete_wallet_flow(const Flash_Wallet *flash_wallet) {
  ASSERT(NULL != flash_wallet);

  char confimation_display[100];
  snprintf(confimation_display,
           sizeof(confimation_display),
           UI_TEXT_PERMANENTLY_DELETE,
           flash_wallet->wallet_name);

  if (!core_scroll_page(NULL, confimation_display, NULL)) {
    return;
  }

  if (!core_scroll_page(
          NULL, ui_text_need_all_x1cards_to_delete_wallet_entirely, NULL)) {
    return;
  }

  clear_wallet_data();

  // Populate global wallet object with wallet data
  memcpy(wallet.wallet_id, flash_wallet->wallet_id, WALLET_ID_SIZE);
  memcpy(wallet.wallet_name, flash_wallet->wallet_name, NAME_SIZE);
  wallet.wallet_info = flash_wallet->wallet_info;

  while (1) {
    if (!get_pin_input()) {
      break;
    }

    if (CARD_OPERATION_INCORRECT_PIN_ENTERED ==
        card_flow_delete_wallet(&wallet)) {
      continue;
    } else {
      break;
    }
  }

  clear_wallet_data();
}