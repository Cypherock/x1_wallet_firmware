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

#include "base58.h"
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

/// account that implements create_account (the registrar) on each network
static const char *const near_registrar_accounts[] = {"near", "testnet"};

static bool is_registrar_account(const uint8_t *account, uint32_t length) {
  for (size_t i = 0;
       i < sizeof(near_registrar_accounts) / sizeof(near_registrar_accounts[0]);
       i++) {
    if (length == strlen(near_registrar_accounts[i]) &&
        0 == memcmp(account, near_registrar_accounts[i], length)) {
      return true;
    }
  }
  return false;
}

static bool is_account_id_char(uint8_t c) {
  return ('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || '.' == c ||
         '_' == c || '-' == c;
}

/**
 * Parses create_account args, accepting exactly
 * {"new_account_id":"<id>","new_public_key":"ed25519:<public_key>"}
 * where public_key is the given key, so the new account is controlled by the
 * same key that signs this transaction.
 */
static bool parse_create_account_args(const uint8_t *args,
                                      uint32_t args_length,
                                      const uint8_t *public_key,
                                      const uint8_t **account_id,
                                      uint32_t *account_id_length) {
  static const char prefix[] = "{\"new_account_id\":\"";
  static const char separator[] = "\",\"new_public_key\":\"ed25519:";
  static const char suffix[] = "\"}";

  char key[60] = {0};
  size_t key_length = sizeof(key);
  if (!b58enc(key, &key_length, public_key, 32)) {
    return false;
  }
  // b58enc counts the terminating NUL
  key_length = strnlen(key, sizeof(key));

  uint32_t offset = 0;
  if (args_length < sizeof(prefix) - 1 ||
      0 != memcmp(args, prefix, sizeof(prefix) - 1)) {
    return false;
  }
  offset = sizeof(prefix) - 1;

  uint32_t id_start = offset;
  while (offset < args_length && is_account_id_char(args[offset])) {
    offset++;
  }
  uint32_t id_length = offset - id_start;
  // NEAR account ids are 2 to 64 characters
  if (id_length < 2 || id_length > 64) {
    return false;
  }

  if (args_length - offset !=
          (sizeof(separator) - 1) + key_length + (sizeof(suffix) - 1) ||
      0 != memcmp(args + offset, separator, sizeof(separator) - 1)) {
    return false;
  }
  offset += sizeof(separator) - 1;
  if (0 != memcmp(args + offset, key, key_length)) {
    return false;
  }
  offset += key_length;
  if (0 != memcmp(args + offset, suffix, sizeof(suffix) - 1)) {
    return false;
  }

  *account_id = args + id_start;
  *account_id_length = id_length;
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
bool near_parse_transaction(const uint8_t *byte_array,
                            uint16_t byte_array_size,
                            near_unsigned_txn *utxn) {
  if (byte_array == NULL || utxn == NULL)
    return false;
  memzero(utxn, sizeof(near_unsigned_txn));

  uint16_t offset = 0;

  // every length below is host-controlled; ensure the read
  // (or advance) stays within byte_array_size before it happens.
#define NEAR_NEED(n)                                                           \
  do {                                                                         \
    if ((uint64_t)offset + (uint32_t)(n) > (uint64_t)byte_array_size)          \
      return false;                                                            \
  } while (0)

  NEAR_NEED(4);
  utxn->signer_id_length = U32_READ_LE_ARRAY(byte_array);
  offset += 4;
  NEAR_NEED(utxn->signer_id_length);
  utxn->signer = (byte_array + offset);
  offset += utxn->signer_id_length;
  NEAR_NEED(1 + 32);
  utxn->signer_key.key_type = byte_array[offset++];
  utxn->signer_key.key = (byte_array + offset);
  offset += 32;
  NEAR_NEED(8);
  memcpy(utxn->nonce, byte_array + offset, sizeof(utxn->nonce));
  offset += 8;
  NEAR_NEED(4);
  utxn->receiver_id_length = U32_READ_LE_ARRAY(byte_array + offset);
  offset += 4;
  NEAR_NEED(utxn->receiver_id_length);
  utxn->receiver = (byte_array + offset);
  offset += utxn->receiver_id_length;
  NEAR_NEED(32 + 4 + 1);
  utxn->blockhash = (byte_array + offset);
  offset += 32;
  utxn->action_count = U32_READ_LE_ARRAY(byte_array + offset);
  offset += 4;
  utxn->actions_type = byte_array[offset++];

  // exactly 1 action: with 0 the decoded "action" would be shown but not be
  // part of the signed transaction
  if (1 != utxn->action_count) {
    return false;
  }

  switch (utxn->actions_type) {
    case NEAR_ACTION_TRANSFER: {
      NEAR_NEED(sizeof(utxn->action.transfer.amount));
      memcpy(utxn->action.transfer.amount,
             byte_array + offset,
             sizeof(utxn->action.transfer.amount));
      cy_reverse_byte_array(utxn->action.transfer.amount,
                            sizeof(utxn->action.transfer.amount));
      offset += sizeof(utxn->action.transfer.amount);
      break;
    }

    case NEAR_ACTION_FUNCTION_CALL: {
      NEAR_NEED(4);
      utxn->action.fn_call.method_name_length =
          U32_READ_LE_ARRAY(byte_array + offset);
      offset += 4;
      NEAR_NEED(utxn->action.fn_call.method_name_length);
      utxn->action.fn_call.method_name = (char *)(byte_array + offset);
      offset += utxn->action.fn_call.method_name_length;

      // As of now, we only support signing of create_account method.
      // require an exact-length match
      if (utxn->action.fn_call.method_name_length !=
              strlen(ui_text_near_create_account_method) ||
          0 != strncmp(utxn->action.fn_call.method_name,
                       ui_text_near_create_account_method,
                       utxn->action.fn_call.method_name_length)) {
        return false;
      }

      NEAR_NEED(4);
      utxn->action.fn_call.args_length = U32_READ_LE_ARRAY(byte_array + offset);
      offset += 4;
      NEAR_NEED(utxn->action.fn_call.args_length);
      utxn->action.fn_call.args = (byte_array + offset);
      offset += utxn->action.fn_call.args_length;

      // create_account is only trusted on the registrar, and only for an
      // account controlled by the signing key
      if (!is_registrar_account(utxn->receiver, utxn->receiver_id_length) ||
          NEAR_CURVE_ED25519 != utxn->signer_key.key_type ||
          !parse_create_account_args(
              utxn->action.fn_call.args,
              utxn->action.fn_call.args_length,
              utxn->signer_key.key,
              &utxn->action.fn_call.new_account_id,
              &utxn->action.fn_call.new_account_id_length)) {
        return false;
      }
      NEAR_NEED(sizeof(utxn->action.fn_call.gas) +
                sizeof(utxn->action.fn_call.deposit));
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
      offset += sizeof(utxn->action.fn_call.deposit);
      break;
    }

    default: {
      return false;
      break;
    }
  }
#undef NEAR_NEED

  // Reverse byte order
  cy_reverse_byte_array(utxn->nonce, sizeof(utxn->nonce));

  // every byte must have been parsed: trailing bytes are signed but not shown
  return offset == byte_array_size;
}
