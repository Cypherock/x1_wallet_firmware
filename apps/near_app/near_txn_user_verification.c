/**
 * @file    near_txn_user_verification.c
 * @author  Cypherock X1 Team
 * @brief   Source file to handle user confirmation flow during txn signing for
 *          NEAR protocol
 *
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

#include <stdint.h>

#include "constant_texts.h"
#include "near_api.h"
#include "near_context.h"
#include "near_helpers.h"
#include "near_priv.h"
#include "ui_core_confirm.h"

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

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
bool user_verification_transfer(const near_unsigned_txn *decoded_utxn) {
  char transaction[100] = "";
  char address[200] = "";
  char value[100] = "";

  snprintf(transaction,
           sizeof(transaction),
           UI_TEXT_REVIEW_TXN_PROMPT,
           ui_text_near_transfer_action_type);

  snprintf(address,
           CY_MIN(decoded_utxn->receiver_id_length + 1, sizeof(address)),
           "%s",
           decoded_utxn->receiver);

  get_amount_string(decoded_utxn->action.transfer.amount, value, sizeof(value));

  if (!core_scroll_page(NULL, transaction, near_send_error) ||
      !core_scroll_page(ui_text_verify_address, address, near_send_error) ||
      !core_confirmation(value, near_send_error)) {
    return false;
  }

  return true;
}

bool user_verification_function(const near_unsigned_txn *decoded_utxn) {
  char transaction[100] = "";
  char address[200] = "";
  char account[200] = "";
  char value[100] = "";

  snprintf(transaction,
           sizeof(transaction),
           UI_TEXT_REVIEW_TXN_PROMPT,
           ui_text_near_create_account_method);

  snprintf(address,
           CY_MIN(decoded_utxn->signer_id_length, sizeof(address)),
           "%s",
           decoded_utxn->signer);

  near_get_new_account_id_from_fn_args(
      (const char *)decoded_utxn->action.fn_call.args,
      decoded_utxn->action.fn_call.args_length,
      account);

  get_amount_string(decoded_utxn->action.fn_call.deposit, value, sizeof(value));

  if (!core_scroll_page(NULL, transaction, near_send_error) ||
      !core_scroll_page(ui_text_verify_create_from, address, near_send_error) ||
      !core_scroll_page(
          ui_text_verify_new_account_id, account, near_send_error) ||
      !core_confirmation(value, near_send_error)) {
    return false;
  }

  return true;
}