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

#include "bignum.h"
#include "btc_helpers.h"
#include "btc_script.h"
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
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Returns transaction weight for a given transaction
 * @details Weight is required for fee estimation and signifies how much time
 *          would it take to execute a txn in block.
 *          This function will give precise results for P2PKH and P2WPKH when we
 *          use a fixed size for script_sig in addition to the weight returned
 *          by this function. For P2SH & P2WSH, the function will give wrong
 *          results so we will re-evaluate this function when the support for
 *          those types is added. Refer:
 *          https://github.com/bitcoin/bips/blob/master/bip-0141.mediawiki#transaction-size-calculations
 *          https://github.com/trezor/trezor-firmware/blob/f5983e7843f381423f30b8bc2ffc46e496775e5a/core/src/apps/bitcoin/sign_tx/tx_weight.py#L95
 *          https://github.com/trezor/trezor-firmware/blob/f5983e7843f381423f30b8bc2ffc46e496775e5a/common/protob/messages-bitcoin.proto#L357
 *
 * @param [in] txn_ctx Instance of btc_txn_context_t
 *
 * @return weight of the transaction
 */
STATIC uint32_t get_transaction_weight(const btc_txn_context_t *txn_ctx);

/**
 * @brief Digests all outputs of the current transaction into the provided
 * sha256 hasher context.
 *
 * @param context Reference to an instance of btc_txn_context_t
 * @param sha_256_ctx Reference to the SHA256_CTX
 *
 * @return bool Indicating if all the outputs are digested
 * @retval true If all the outputs are digested into the provided sha256 context
 */
STATIC bool digest_outputs(const btc_txn_context_t *context,
                           SHA256_CTX *sha_256_ctx);

/**
 * @brief Calculates digest for p2pkh according to the BIP definition
 *
 * @param context Reference to the bitcoin transaction context
 * @param index The index of the input to digest
 * @param digest Reference to a buffer to hold the calculated digest
 */
STATIC void calculate_p2pkh_digest(const btc_txn_context_t *context,
                                   uint8_t input_index,
                                   uint8_t *digest);

/**
 * @brief Calculates digest according to the serialization format defined in
 * BIP-0143.
 *
 * @param context Reference to the bitcoin transaction context
 * @param index The index of the input to digest
 * @param digest Reference to a buffer to hold the calculated digest
 *
 * @return bool Indicating if the specified input was digested or not
 * @retval true If the digest was calculated successfully
 * @retval false If the digest was not calculated due to missing segwit cache
 */
STATIC bool calculate_p2wpkh_digest(const btc_txn_context_t *context,
                                    uint8_t input_index,
                                    uint8_t *digest);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

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
    weight += 4;     // sequence
                     // Check if current input is segwit or not
    if (0 == txn_ctx->inputs[input_index].script_pub_key.bytes[0]) {
      segwit_count++;
    } else {
      weight += EXPECTED_SCRIPT_SIG_SIZE;
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
    weight += (EXPECTED_SCRIPT_SIG_SIZE *
               segwit_count);    // Adding sizes of all witnesses for all inputs
  }

  return weight;
}

STATIC bool digest_outputs(const btc_txn_context_t *context,
                           SHA256_CTX *sha_256_ctx) {
  uint8_t buffer[100] = {0};
  for (uint8_t idx = 0; idx < context->metadata.output_count; idx++) {
    btc_sign_txn_output_t *output = &context->outputs[idx];

    // digest the 64-bit value
    uint64_t val = context->outputs[idx].value;
    REVERSE64(val, val);
    dec_to_hex(val, buffer, 8);
    sha256_Update(sha_256_ctx, buffer, 8);

    // correct the encoding for size to CompactSize. Refer:
    buffer[0] = output->script_pub_key.size;
    sha256_Update(sha_256_ctx, buffer, 1);
    sha256_Update(sha_256_ctx, output->script_pub_key.bytes, buffer[0]);
  }
}

STATIC void calculate_p2pkh_digest(const btc_txn_context_t *context,
                                   const uint8_t input_index,
                                   uint8_t *digest) {
  uint8_t buffer[100] = {0};
  SHA256_CTX sha_256_ctx = {0};

  memzero(&sha_256_ctx, sizeof(sha_256_ctx));
  sha256_Init(&sha_256_ctx);

  // digest version and input count
  write_le(buffer, context->metadata.version);
  buffer[4] = context->metadata.input_count;
  sha256_Update(&sha_256_ctx, buffer, 5);

  for (uint8_t idx = 0; idx < context->metadata.input_count; idx++) {
    btc_txn_input_t *input = &context->inputs[idx];

    // digest Outpoint (input transaction hash, index)
    sha256_Update(&sha_256_ctx, input->prev_txn_hash, 32);
    write_le(buffer, input->prev_output_index);
    sha256_Update(&sha_256_ctx, buffer, 4);

    if (input_index == idx) {
      // TODO: use Compact size encoding here. Ref -
      // https://developer.bitcoin.org/reference/transactions.html#compactsize-unsigned-integers
      // digest the locking script to sign
      buffer[0] = input->script_pub_key.size;
      sha256_Update(&sha_256_ctx, buffer, 1);
      sha256_Update(&sha_256_ctx, input->script_pub_key.bytes, buffer[0]);
    } else {
      // skip all the other Outpoints
      buffer[0] = 0;
      sha256_Update(&sha_256_ctx, buffer, 1);
    }

    write_le(buffer, input->sequence);
    sha256_Update(&sha_256_ctx, buffer, 4);
  }

  buffer[0] = context->metadata.output_count;
  sha256_Update(&sha_256_ctx, buffer, 1);
  digest_outputs(context, &sha_256_ctx);

  // digest locktime and sighash
  write_le(buffer, context->metadata.locktime);
  write_le(buffer + 4, context->metadata.sighash);
  sha256_Update(&sha_256_ctx, buffer, 8);

  // double hash
  sha256_Final(&sha_256_ctx, digest);
  sha256_Raw(digest, 32, digest);
  memzero(&sha_256_ctx, sizeof(sha_256_ctx));
}

STATIC bool calculate_p2wpkh_digest(const btc_txn_context_t *context,
                                    const uint8_t input_index,
                                    uint8_t *digest) {
  if (!context->segwit_cache.filled) {
    // cache is not filled, no benefit to proceed as we depend on it
    return false;
  }

  uint8_t buffer[100] = {0};
  SHA256_CTX sha_256_ctx = {0};
  btc_txn_input_t *input = &context->inputs[input_index];

  memzero(&sha_256_ctx, sizeof(sha_256_ctx));
  sha256_Init(&sha_256_ctx);

  // digest version
  write_le(buffer, context->metadata.version);
  sha256_Update(&sha_256_ctx, buffer, 4);

  sha256_Update(&sha_256_ctx, context->segwit_cache.hash_prevouts, 32);
  sha256_Update(&sha_256_ctx, context->segwit_cache.hash_sequence, 32);
  sha256_Update(&sha_256_ctx, input->prev_txn_hash, 32);

  write_le(buffer, input->prev_output_index);
  sha256_Update(&sha_256_ctx, buffer, 4);

  buffer[0] = input->script_pub_key.size + 2 + 2 - 1;
  buffer[1] = 0x76;
  buffer[2] = 0xa9;
  sha256_Update(&sha_256_ctx, buffer, 3);
  sha256_Update(&sha_256_ctx,
                &input->script_pub_key.bytes[1],
                input->script_pub_key.size - 1);
  buffer[0] = 0x88;
  buffer[1] = 0xac;
  sha256_Update(&sha_256_ctx, buffer, 2);

  // digest the 64-bit value (little-endian)
  sha256_Update(&sha_256_ctx, (uint8_t *)&input->value, 8);
  write_le(buffer, input->sequence);
  sha256_Update(&sha_256_ctx, buffer, 4);
  sha256_Update(&sha_256_ctx, context->segwit_cache.hash_outputs, 32);

  // digest locktime and sighash
  write_le(buffer, context->metadata.locktime);
  write_le(buffer + 4, context->metadata.sighash);
  sha256_Update(&sha_256_ctx, buffer, 8);

  // double hash
  sha256_Final(&sha_256_ctx, digest);
  sha256_Raw(digest, 32, digest);
  memzero(&sha_256_ctx, sizeof(sha_256_ctx));
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

int btc_verify_input(const uint8_t *raw_txn,
                     const uint32_t size,
                     const btc_sign_txn_input_t *input) {
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
  // remember the number of inputs in the raw_txn
  count = raw_txn[offset++];

  for (int32_t input_index = 0; input_index < count; input_index++) {
    offset += 36;
    offset += (raw_txn[offset] + 1 + 4);
  }
  // remember the number of outputs in the raw_txn
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
  // verify input txn hash
  if (memcmp(hash, input->prev_txn_hash, sizeof(input->prev_txn_hash)) != 0) {
    return 2;
  }
  if (U64_READ_LE_ARRAY(raw_txn + value_offset) != input->value) {
    return 3;
  }
  return 0;
}

uint64_t get_transaction_fee_threshold(const btc_txn_context_t *txn_ctx) {
  return (g_btc_app->max_fee / 1000) * (get_transaction_weight(txn_ctx) / 4);
}

bool btc_get_txn_fee(const btc_txn_context_t *txn_ctx, uint64_t *fee) {
  if (NULL == fee) {
    return false;
  }

  uint64_t input = 0;
  uint64_t output = 0;
  *fee = UINT64_MAX;

  for (int idx = 0; idx < txn_ctx->metadata.input_count; idx++) {
    input += txn_ctx->inputs[idx].value;
  }

  for (int idx = 0; idx < txn_ctx->metadata.output_count; idx++) {
    output += txn_ctx->outputs[idx].value;
  }

  if (input < output) {
    // case of an overspending transaction
    return false;
  }

  *fee = (input - output);
  return true;
}

void btc_segwit_init_cache(btc_txn_context_t *context) {
  uint8_t bytes[32] = {0};
  SHA256_CTX sha_256_ctx = {0};
  btc_segwit_cache_t *segwit_cache = &context->segwit_cache;

  // calculate double SHA256 of the input UTXOs
  sha256_Init(&sha_256_ctx);
  for (int idx = 0; idx < context->metadata.input_count; idx++) {
    sha256_Update(&sha_256_ctx, context->inputs[idx].prev_txn_hash, 32);
    write_le(bytes, context->inputs[idx].prev_output_index);
    sha256_Update(&sha_256_ctx, bytes, sizeof(uint32_t));
  }
  // double hash
  sha256_Final(&sha_256_ctx, segwit_cache->hash_prevouts);
  sha256_Raw(segwit_cache->hash_prevouts, 32, segwit_cache->hash_prevouts);
  sha256_Init(&sha_256_ctx);

  // calculate double SHA256 of the input sequences
  for (int idx = 0; idx < context->metadata.input_count; idx++) {
    write_le(bytes, context->inputs[idx].sequence);
    sha256_Update(&sha_256_ctx, bytes, sizeof(uint32_t));
  }
  // double hash
  sha256_Final(&sha_256_ctx, segwit_cache->hash_sequence);
  sha256_Raw(segwit_cache->hash_sequence, 32, segwit_cache->hash_sequence);
  sha256_Init(&sha_256_ctx);

  // calculate double SHA256 of the output UTXOs
  digest_outputs(context, &sha_256_ctx);
  // double hash
  sha256_Final(&sha_256_ctx, segwit_cache->hash_outputs);
  sha256_Raw(segwit_cache->hash_outputs, 32, segwit_cache->hash_outputs);

  segwit_cache->filled = true;
  memzero(&sha_256_ctx, sizeof(sha_256_ctx));
}

bool btc_digest_input(const btc_txn_context_t *context,
                      const uint32_t index,
                      uint8_t *digest) {
  bool status = true;
  // detect input type and calculate appropriate digest
  btc_sign_txn_input_script_pub_key_t *script =
      &context->inputs[index].script_pub_key;
  btc_script_type_e type = btc_get_script_type(script->bytes, script->size);

  if (SCRIPT_TYPE_P2WPKH == type) {
    // segwit digest calculation; could fail if segwit_cache not filled
    status = calculate_p2wpkh_digest(context, index, digest);
  } else if (SCRIPT_TYPE_P2PKH == type) {
    // p2pkh digest calculation; has not failure case
    calculate_p2pkh_digest(context, index, digest);
  } else {
    status = false;
  }
  return status;
}
