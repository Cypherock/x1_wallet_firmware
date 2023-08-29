/**
 * @file    near_txn_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the NEAR app for txn signing flow
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

#include "near_txn_helpers.h"

#include "coin_utils.h"
#include "constant_texts.h"
#include "near_context.h"
#include "utils.h"

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
bool near_parse_transaction(const uint8_t *byte_array,
                            uint16_t byte_array_size,
                            near_unsigned_txn *utxn) {
  if (byte_array == NULL || utxn == NULL)
    return;
  memzero(utxn, sizeof(near_unsigned_txn));

  uint16_t offset = 0;

  utxn->signer_id_length = U32_READ_LE_ARRAY(byte_array);
  offset += 4;
  utxn->signer = (byte_array + offset);
  offset += utxn->signer_id_length;
  utxn->signer_key.key_type = byte_array[offset++];
  utxn->signer_key.key = (byte_array + offset);
  offset += 32;
  memcpy(utxn->nonce, byte_array + offset, sizeof(utxn->nonce));
  offset += 8;
  utxn->receiver_id_length = U32_READ_LE_ARRAY(byte_array + offset);
  offset += 4;
  utxn->receiver = (byte_array + offset);
  offset += utxn->receiver_id_length;
  utxn->blockhash = (byte_array + offset);
  offset += 32;
  utxn->action_count = U32_READ_LE_ARRAY(byte_array + offset);
  offset += 4;
  utxn->actions_type = byte_array[offset++];

  // Currently, our decoder only supports 1 action
  if (1 < utxn->action_count) {
    return false;
  }

  switch (utxn->actions_type) {
    case NEAR_ACTION_TRANSFER: {
      memcpy(utxn->action.transfer.amount,
             byte_array + offset,
             sizeof(utxn->action.transfer.amount));
      cy_reverse_byte_array(utxn->action.transfer.amount,
                            sizeof(utxn->action.transfer.amount));
      break;
    }

    case NEAR_ACTION_FUNCTION_CALL: {
      utxn->action.fn_call.method_name_length =
          U32_READ_LE_ARRAY(byte_array + offset);
      offset += 4;
      utxn->action.fn_call.method_name = (char *)(byte_array + offset);
      offset += utxn->action.fn_call.method_name_length;

      // As of now, we only support signing of create_account method
      if (0 != strncmp(utxn->action.fn_call.method_name,
                       ui_text_near_create_account_method,
                       utxn->action.fn_call.method_name_length)) {
        return false;
      }

      utxn->action.fn_call.args_length = U32_READ_LE_ARRAY(byte_array + offset);
      offset += 4;
      utxn->action.fn_call.args = (byte_array + offset);
      offset += utxn->action.fn_call.args_length;
      memcpy(utxn->action.fn_call.gas,
             byte_array + offset,
             sizeof(utxn->action.fn_call.gas));
      cy_reverse_byte_array(utxn->action.fn_call.gas,
                            sizeof(utxn->action.fn_call.gas));
      offset += 8;
      memcpy(utxn->action.fn_call.deposit,
             byte_array + offset,
             sizeof(utxn->action.fn_call.deposit));
      cy_reverse_byte_array(utxn->action.fn_call.deposit,
                            sizeof(utxn->action.fn_call.deposit));
      break;
    }

    default: {
      return false;
      break;
    }
  }

  // Reverse byte order
  cy_reverse_byte_array(utxn->nonce, sizeof(utxn->nonce));

  // TODO: Offset validation should occur at every read
  if (offset <= byte_array_size) {
    return true;
  }

  return false;
}
