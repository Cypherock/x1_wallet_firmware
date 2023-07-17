/**
 * @file    btc_txn_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Bitcoin family transaction helper implementation
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

#include "btc_txn_helpers.h"

#include <stdio.h>

#include "btc_helpers.h"
#include "utils.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

#define STATIC
#ifndef STATIC
#define STATIC static
#endif

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Returns transaction weight for a given transaction
 * @details Weight is required for fee estimation and signifies how much time
 *          would it take to execute a txn in block. Refer
 * https://github.com/bitcoin/bips/blob/master/bip-0141.mediawiki#transaction-size-calculations
 *
 * @param [in] txn_ctx Instance of unsigned_txn
 *
 * @return weight of the transaction
 * @retval
 */
STATIC uint32_t get_transaction_weight(const btc_txn_context_t *txn_ctx);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

// TODO: fix weight for segwit transactions - effects upper-limit fee check
STATIC uint32_t get_transaction_weight(const btc_txn_context_t *txn_ctx) {
  uint8_t segwit_count = 0;
  uint32_t weight = 0;

  // TODO:Replace multiple instructions with single macro for weight
  weight += 4;    // network version size
  weight += 1;    // input count size

  for (uint8_t input_index = 0; input_index < txn_ctx->metadata.input_count;
       input_index++) {
    weight += 32;    // previous transaction hash
    weight += 4;     // previous output index
    weight += 1;     // script length size
    weight += txn_ctx->inputs[input_index].script_pub_key.size;
    weight += 4;    // sequence
                    // Check if current input is segwit or not
    if (0 == txn_ctx->inputs[input_index].script_pub_key.bytes[0]) {
      segwit_count++;
    }
  }

  weight += 1;    // output count size

  for (uint8_t output_index = 0; output_index < txn_ctx->metadata.output_count;
       output_index++) {
    weight += 8;    // value size
    weight += 1;    //  script length size
    weight += txn_ctx->outputs[output_index].script_pub_key.size;
  }

  weight += 4;            // locktime
  weight = 4 * weight;    // As per standard non segwit transaction size is
                          // multiplied by 4

  if (segwit_count > 0) {
    weight += 2;    // Segwit headers
    weight +=
        (106 * segwit_count);    // Adding sizes of all witnesses for all inputs
  }

  return weight;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

int btc_verify_input_utxo(const uint8_t *raw_txn,
                          const uint32_t size,
                          const btc_txn_input_t *input) {
  if (NULL == input || NULL == raw_txn || 0 == size) {
    return -1;
  }

  uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
  int32_t offset = 0;
  int32_t count = 0;
  int32_t value_offset = 0;
  int32_t start_offset = 0;
  uint8_t txn_data[size];

  // ignore network version (4-bytes), skip marker & flag (in segwit)
  offset += (raw_txn[4] == 0 ? 6 : 4);
  start_offset = offset;
  // remember the number of input UTXOs in the raw_txn
  count = raw_txn[offset++];

  for (int32_t input_index = 0; input_index < count; input_index++) {
    offset += 36;
    offset += (raw_txn[offset] + 1 + 4);
  }
  // remember the number of output UTXOs in the raw_txn
  count = raw_txn[offset++];
  for (int32_t output_index = 0; output_index < count; output_index++) {
    if (output_index == input->prev_output_index) {
      // only check the specified output index as we are looking for an exact
      // match; remember the location of value in the raw_txn & compare later
      value_offset = offset;

      // NOTE: do not break here, the 'offset' should traverse till the end
    }
    offset += 8;
    offset += (raw_txn[offset] + 1);
  }

  if (value_offset == 0) {
    return 1;
  }

  // network version (first 4 bytes)
  memcpy(txn_data, raw_txn, 4);
  // txin and txout (skip marker & flag)
  memcpy(txn_data + 4, raw_txn + start_offset, offset - start_offset);
  // locktime (last 4 bytes)
  memcpy(txn_data + offset - start_offset + 4, raw_txn + size - 4, 4);
  sha256_Raw(txn_data, offset - start_offset + 4 + 4, hash);
  sha256_Raw(hash, sizeof(hash), hash);
  // verify UTXO txn hash
  if (memcmp(hash, input->prev_txn_hash, sizeof(input->prev_txn_hash)) != 0) {
    return 2;
  }
  if (U64_READ_LE_ARRAY(raw_txn + value_offset) != input->value) {
    return 3;
  }
  return 0;
}

uint64_t get_transaction_fee_threshold(const btc_txn_context_t *txn_ctx) {
  return (g_app->max_fee / 1000) * (get_transaction_weight(txn_ctx) / 4);
}

uint64_t btc_get_txn_fee(const btc_txn_context_t *txn_ctx) {
  uint64_t result = 0;

  for (int idx = 0; idx < txn_ctx->metadata.input_count; idx++) {
    result += txn_ctx->inputs[idx].value;
  }

  for (int idx = 0; idx < txn_ctx->metadata.output_count; idx++) {
    result -= txn_ctx->outputs[idx].value;
  }

  return result;
}

void format_value(const uint64_t value_in_sat,
                  char *msg,
                  const size_t msg_len) {
  uint8_t precision = get_floating_precision(value_in_sat, SATOSHI_PER_BTC);
  double fee_in_btc = 1.0 * value_in_sat / (SATOSHI_PER_BTC);
  snprintf(msg, msg_len, "%0.*f %s", precision, fee_in_btc, g_app->lunit_name);
}
