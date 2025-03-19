/**
 * @file    constellation_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Constellation chains
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2024 by HODL TECH PTE LTD
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
#include "constellation_helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coin_utils.h"
#include "constellation_context.h"

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

bool constellation_derivation_path_guard(const uint32_t *path, uint8_t levels) {
  bool status = false;
  if (levels != CONSTELLATION_IMPLICIT_ACCOUNT_DEPTH) {
    return status;
  }

  uint32_t purpose = path[0], coin = path[1], account = path[2],
           change = path[3], address = path[4];

  // m/44'/144'/0'/0/i
  status = (CONSTELLATION_PURPOSE_INDEX == purpose &&
            CONSTELLATION_COIN_INDEX == coin &&
            CONSTELLATION_ACCOUNT_INDEX == account &&
            CONSTELLATION_CHANGE_INDEX == change && is_non_hardened(address));

  return status;
}

/// Ref:
// https://github.com/StardustCollective/dag4.js/blob/main/packages/dag4-keystore/src/transaction-v2.ts#L113
size_t encode_txn(const constellation_transaction_t *txn, char *output) {
  char temp[1024] = "";
  char buffer[32] = "";
  size_t output_len = 0;

  // Parent count (always "2")
  strcat(temp, "2");
  output_len++;

  // Source address length and value
  size_t src_length = strnlen(txn->source, CONSTELLATION_ACCOUNT_ADDRESS_SIZE);
  size_t src_length_string_len =
      snprintf(buffer, sizeof(buffer), "%u", src_length);
  strncat(temp, buffer, src_length_string_len);
  strncat(temp, txn->source, src_length);
  output_len += src_length_string_len + src_length;

  // Destination address length and value
  size_t dest_length =
      strnlen(txn->destination, CONSTELLATION_ACCOUNT_ADDRESS_SIZE);
  size_t dest_length_string_len =
      snprintf(buffer, sizeof(buffer), "%u", dest_length);
  strncat(temp, buffer, dest_length_string_len);
  strncat(temp, txn->destination, dest_length);
  output_len += dest_length_string_len + dest_length;

  // Amount length and value (hex string)
  char hex_str_amount[16] = "";
  size_t hex_str_amount_length =
      snprintf(hex_str_amount, sizeof(hex_str_amount), "%llx", txn->amount);
  size_t hex_str_amount_length_string_len =
      snprintf(buffer, sizeof(buffer), "%u", hex_str_amount_length);
  strncat(temp, buffer, hex_str_amount_length_string_len);
  strncat(temp, hex_str_amount, hex_str_amount_length);
  output_len += hex_str_amount_length_string_len + hex_str_amount_length;

  // Parent hash length and value
  size_t parent_hash_length =
      strnlen(txn->parent.hash, CONSTELLATION_TXN_HASH_LENGTH);
  size_t parent_hash_length_string_len =
      snprintf(buffer, sizeof(buffer), "%u", parent_hash_length);
  strncat(temp, buffer, parent_hash_length_string_len);
  strncat(temp, txn->parent.hash, parent_hash_length);
  output_len += parent_hash_length_string_len + parent_hash_length;

  // Ordinal length and value
  char str_ordinal[32] = "";
  size_t str_ordinal_length =
      snprintf(str_ordinal, sizeof(str_ordinal), "%lu", txn->parent.ordinal);
  size_t str_ordinal_length_string_len =
      snprintf(buffer, sizeof(buffer), "%u", str_ordinal_length);
  strncat(temp, buffer, str_ordinal_length_string_len);
  strncat(temp, str_ordinal, str_ordinal_length);
  output_len += str_ordinal_length_string_len + str_ordinal_length;

  // Fee length and value
  char str_fee[32] = "";
  size_t str_fee_length = snprintf(str_fee, sizeof(str_fee), "%lu", txn->fee);
  size_t str_fee_length_string_len =
      snprintf(buffer, sizeof(buffer), "%u", str_fee_length);
  strncat(temp, buffer, str_fee_length_string_len);
  strncat(temp, str_fee, str_fee_length);
  output_len += str_fee_length_string_len + str_fee_length;

  // Salt length and value
  size_t salt_length = strnlen(txn->salt, 16);
  size_t salt_length_string_len =
      snprintf(buffer, sizeof(buffer), "%u", salt_length);
  strncat(temp, buffer, salt_length_string_len);
  strncat(temp, txn->salt, salt_length);
  output_len += salt_length_string_len + salt_length;

  strncpy(output, temp, output_len);

  return output_len;
}

// Encode a variable-length integer (similar to utf8Length in JS)
/// Ref:
// https://github.com/StardustCollective/dag4.js/blob/main/packages/dag4-keystore/src/tx-encode.ts#L81
void encode_var_length(uint32_t value, uint8_t *output, size_t *out_len) {
  size_t position = 0;

  if (value < (1 << 6)) {
    output[position++] = (value | 0x80);
  } else if (value < (1 << 13)) {
    output[position++] = ((value & 0x3F) | 0xC0);
    output[position++] = (value >> 6);
  } else if (value < (1 << 20)) {
    output[position++] = ((value & 0x3F) | 0xC0);
    output[position++] = ((value >> 6) | 0x80);
    output[position++] = (value >> 13);
  } else if (value < (1 << 27)) {
    output[position++] = ((value & 0x3F) | 0xC0);
    output[position++] = ((value >> 6) | 0x80);
    output[position++] = ((value >> 13) | 0x80);
    output[position++] = (value >> 20);
  } else {
    output[position++] = ((value & 0x3F) | 0xC0);
    output[position++] = ((value >> 6) | 0x80);
    output[position++] = ((value >> 13) | 0x80);
    output[position++] = ((value >> 20) | 0x80);
    output[position++] = (value >> 27);
  }
  *out_len = position;
}

/// Ref:
// https://github.com/StardustCollective/dag4.js/blob/main/packages/dag4-keystore/src/tx-encode.ts#L71
void kryo_serialize(const char *msg,
                    size_t msg_len,
                    uint8_t *output,
                    size_t *out_len) {
  uint8_t length_encoded[5] = {0};
  size_t len = 0;

  // Prefix is "03" + utf8Length(msg length + 1)
  encode_var_length(msg_len + 1, length_encoded, &len);

  size_t index = 0;
  output[index++] = 0x03;    // Prefix "03"

  // Append length encoding
  memcpy(&output[index], length_encoded, len);
  index += len;

  // Append msg as raw bytes
  memcpy(&output[index], msg, msg_len);
  index += msg_len;

  *out_len = index;
}

/// Ref:
// https://github.com/StardustCollective/dag4.js/blob/main/packages/dag4-keystore/src/key-store.ts#L337
void serialize_txn(const constellation_transaction_t *txn,
                   uint8_t *output,
                   size_t *output_len) {
  char encoded_txn[1024] = "";
  size_t encoded_txn_len = encode_txn(txn, encoded_txn);

  kryo_serialize(encoded_txn, encoded_txn_len, output, output_len);
}
