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

#include <stdint.h>
#include <stdio.h>

#include "bignum.h"
#include "btc_helpers.h"
#include "btc_script.h"
#include "secp256k1.h"
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
static const uint8_t TAP_SIG_HASH[] = {
    244, 10,  72,  223, 75,  42,  112, 200, 180, 146, 75,  242, 101,
    70,  97,  237, 61,  149, 253, 102, 163, 19,  235, 135, 35,  117,
    151, 198, 40,  228, 160, 49,  244, 10,  72,  223, 75,  42,  112,
    200, 180, 146, 75,  242, 101, 70,  97,  237, 61,  149, 253, 102,
    163, 19,  235, 135, 35,  117, 151, 198, 40,  228, 160, 49,
};
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

STATIC bool calculate_p2tr_digest(const btc_txn_context_t *context,
                                  const uint8_t input_index,
                                  uint8_t *digest) {
  if (!context->taproot_cache.filled) {
    // cache is not filled, no benefit to proceed as we depend on it
    return false;
  }

  uint8_t buffer[300] = {0};
  uint32_t len = 0;

  bool annex = false;    //!(!context->metadata.annex);

  uint8_t output_type = (context->metadata.sighash == SIGHASH_DEFAULT)
                            ? SIGHASH_ALL
                            : context->metadata.sighash & SIGHASH_OUTPUT_MASK;
  uint8_t input_type = context->metadata.sighash & SIGHASH_INPUT_MASK;
  bool is_anyone_can_pay = (input_type == SIGHASH_ANYONECANPAY);
  bool is_none = (output_type == SIGHASH_NONE);
  bool is_single = (output_type == SIGHASH_SINGLE);

  uint8_t prefix = 0x00;
  memcpy(buffer, (uint8_t *)&prefix, 1);
  len += 1;

  memcpy(buffer + len, (uint8_t *)&context->metadata.sighash, 1);
  len += 1;

  write_le(buffer + len, context->metadata.version);
  len += 4;

  write_le(buffer + len, context->metadata.locktime);
  len += 4;

  if (!is_anyone_can_pay) {
    memcpy(buffer + len, context->taproot_cache.sha_prevouts, 32);
    len += 32;

    memcpy(buffer + len, context->taproot_cache.sha_amounts, 32);
    len += 32;

    memcpy(buffer + len, context->taproot_cache.sha_scriptpubkeys, 32);
    len += 32;

    memcpy(buffer + len, context->taproot_cache.sha_sequences, 32);
    len += 32;
  }

  if (!(is_none || is_single)) {
    // sha_outputs
    memcpy(buffer + len, context->taproot_cache.sha_outputs, 32);
    len += 32;
  }

  // spend_flag = (ext_flag*2)+annex_present
  // ext_flag = 0, should compute annex_present
  uint8_t spendbit = 0;
  memcpy(buffer + len, &spendbit, 1);
  len += 1;

  if (is_anyone_can_pay) {
    memcpy(buffer + len, context->inputs[input_index].prev_txn_hash, 32);
    len += 32;
    memcpy(buffer + len,
           (uint8_t *)&context->inputs[input_index].prev_output_index,
           4);
    len += 4;

    memcpy(buffer + len, (uint8_t *)&context->inputs[input_index].value, 8);
    len += 8;

    memcpy(buffer + len,
           (uint8_t *)&context->inputs[input_index].script_pub_key.size,
           1);
    len += 1;

    memcpy(buffer + len,
           context->inputs[input_index].script_pub_key.bytes,
           context->inputs[input_index].script_pub_key.size);
    len += context->inputs[input_index].script_pub_key.size;

    memcpy(buffer + len, (uint8_t *)&context->inputs[input_index].sequence, 4);
    len += 4;
  } else {
    uint32_t index = input_index;
    memcpy(buffer + len, &index, 4);
    len += 4;
  }

  if (annex) {
    // TODO: implement support for annex
    return false;
  }

  if (is_single) {
    memcpy(buffer + len, context->taproot_cache.sha_outputs, 32);
    len += 32;
  }

  // TODO: BIP342 extension
  /*
  Length calculation from:
  https://github.com/bitcoin/bips/blob/master/bip-0341.mediawiki#cite_note-14
  With extension from:
  https://github.com/bitcoin/bips/blob/master/bip-0342.mediawiki#signature-validation
  uint8_t sig_msg_size =
  174 -
  (isAnyoneCanPay ? 49 : 0) -
  (isNone ? 32 : 0) +
  (annex ? 32 : 0) +
  (leafHash ? 37 : 0);
  */

  // compute message hash
  uint8_t temp_buffer[400] = {0};
  memcpy(temp_buffer,
         TAP_SIG_HASH,
         sizeof(TAP_SIG_HASH) / sizeof(TAP_SIG_HASH[0]));
  memcpy(temp_buffer + (sizeof(TAP_SIG_HASH) / sizeof(TAP_SIG_HASH[0])),
         buffer,
         len);

  sha256_Raw(temp_buffer,
             len + (sizeof(TAP_SIG_HASH) / sizeof(TAP_SIG_HASH[0])),
             digest);
  return true;
}

/// Ref:
/// https://github.com/bitcoin/bips/blob/master/bip-0340.mediawiki#default-signing
STATIC int schnorrsig_sign(const uint8_t *tweaked_private_key,
                           const uint8_t *aux,
                           const uint8_t *digest,
                           uint8_t *signature_bytes) {
  uint8_t tweaked_public_key[33] = {0};
  bignum256 sk = {0};
  bignum256 e = {0};
  bignum256 k = {0};
  bignum256 s = {0};
  curve_point R = {0};
  curve_point P = {0};
  uint8_t aux_hash[32] = {0};                // Use zero auxiliary data
  uint8_t nonce_data[32 + 32 + 32] = {0};    // sk || P.x || msg
  uint8_t nonce_hash[32] = {0};
  uint8_t challenge_data[32 + 32 + 32] = {0};    // R.x || P.x || msg
  uint8_t challenge_hash[32] = {0};

  const ecdsa_curve *curve = &secp256k1;

  // Load and validate private key
  bn_read_be(tweaked_private_key, &sk);
  if (bn_is_zero(&sk) || !bn_is_less(&sk, &curve->order)) {
    return -1;
  }

  // Get public key
  ecdsa_get_public_key33(&secp256k1, tweaked_private_key, tweaked_public_key);

  // Negate private key if public key y is odd
  if (tweaked_public_key[0] == 0x03) {
    bn_subtract(&curve->order, &sk, &sk);
    bn_mod(&sk, &curve->order);
  }

  // Generate deterministic nonce k (BIP340)
  // First, XOR auxiliary data hash with private key bytes for additional
  // randomness
  bip340_tagged_hash("BIP0340/aux", aux_hash, aux, 32);

  uint8_t sk_bytes[32];
  bn_write_be(&sk, sk_bytes);
  for (int i = 0; i < 32; i++) {
    sk_bytes[i] ^= aux_hash[i];
  }

  // Build nonce input: sk || P.x || msg || aux
  memcpy(nonce_data, sk_bytes, 32);
  memcpy(nonce_data + 32, tweaked_public_key + 1, 32);
  memcpy(nonce_data + 64, digest, 32);

  // Generate nonce
  bip340_tagged_hash("BIP0340/nonce", nonce_hash, nonce_data, 96);
  bn_read_be(nonce_hash, &k);
  bn_mod(&k, &curve->order);

  // Ensure k != 0
  if (bn_is_zero(&k)) {
    return -1;
  }

  // Calculate R = k * G
  scalar_multiply(curve, &k, &R);

  // Negate k if R.y is odd
  if (bn_is_odd(&R.y)) {
    bn_subtract(&curve->order, &k, &k);
    bn_mod(&k, &curve->order);
  }

  // Store R.x in signature (first 32 bytes)
  bn_write_be(&R.x, signature_bytes);

  // Calculate e = tagged_hash("BIP0340/challenge", R.x || P.x || m)
  memcpy(challenge_data, signature_bytes, 32);                // R.x
  memcpy(challenge_data + 32, tweaked_public_key + 1, 32);    // P.x
  memcpy(challenge_data + 64, digest, 32);                    // message

  // Generate challenge e
  bip340_tagged_hash("BIP0340/challenge", challenge_hash, challenge_data, 96);
  bn_read_be(challenge_hash, &e);

  // Calculate s = k + e * sk mod n
  bignum256 e_sk = {0};
  bignum256 result = {0};
  bn_copy(&sk, &e_sk);
  bn_multiply(&e, &e_sk, &curve->order);    // e_sk = e*sk mod n
  bn_copy(&k, &result);
  bn_add(&result, &e_sk);    // result = k + e_sk mod n
  bn_mod(&result, &curve->order);
  bn_copy(&result, &s);

  // Store s in signature (second 32 bytes)
  bn_write_be(&s, signature_bytes + 32);

  // Clear all sensitive data
  memzero(&tweaked_public_key, sizeof(tweaked_public_key));
  memzero(&sk, sizeof(sk));
  memzero(&k, sizeof(k));
  memzero(&e, sizeof(e));
  memzero(&e_sk, sizeof(e_sk));
  memzero(&result, sizeof(result));
  memzero(&s, sizeof(s));
  memzero(&R, sizeof(R));
  memzero(&P, sizeof(P));
  memzero(sk_bytes, sizeof(sk_bytes));
  memzero(aux_hash, sizeof(aux_hash));
  memzero(nonce_data, sizeof(nonce_data));
  memzero(nonce_hash, sizeof(nonce_hash));
  memzero(challenge_data, sizeof(challenge_data));
  memzero(challenge_hash, sizeof(challenge_hash));

  return 0;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

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

void btc_taproot_init_cache(btc_txn_context_t *context) {
  btc_taproot_cache_t *taproot_cache = &context->taproot_cache;

  // sha_prevouts
  SHA256_CTX sha_256_ctx = {0};
  memzero(&sha_256_ctx, sizeof(sha_256_ctx));
  sha256_Init(&sha_256_ctx);
  for (uint32_t idx = 0; idx < context->metadata.input_count; idx++) {
    sha256_Update(&sha_256_ctx, context->inputs[idx].prev_txn_hash, 32);
    sha256_Update(
        &sha_256_ctx, (uint8_t *)&context->inputs[idx].prev_output_index, 4);
  }
  sha256_Final(&sha_256_ctx, taproot_cache->sha_prevouts);

  // sha_amounts
  memzero(&sha_256_ctx, sizeof(sha_256_ctx));
  sha256_Init(&sha_256_ctx);
  for (uint32_t idx = 0; idx < context->metadata.input_count; idx++) {
    sha256_Update(&sha_256_ctx, (uint8_t *)&context->inputs[idx].value, 8);
  }
  sha256_Final(&sha_256_ctx, taproot_cache->sha_amounts);

  // sha_scriptpubkeys
  memzero(&sha_256_ctx, sizeof(sha_256_ctx));
  sha256_Init(&sha_256_ctx);
  for (uint32_t idx = 0; idx < context->metadata.input_count; idx++) {
    sha256_Update(
        &sha_256_ctx, (uint8_t *)&context->inputs[idx].script_pub_key.size, 1);
    sha256_Update(&sha_256_ctx,
                  context->inputs[idx].script_pub_key.bytes,
                  context->inputs[idx].script_pub_key.size);
  }
  sha256_Final(&sha_256_ctx, taproot_cache->sha_scriptpubkeys);

  // sha_sequences
  sha256_Init(&sha_256_ctx);
  for (uint32_t idx = 0; idx < context->metadata.input_count; idx++) {
    sha256_Update(&sha_256_ctx, (uint8_t *)&context->inputs[idx].sequence, 4);
  }
  sha256_Final(&sha_256_ctx, taproot_cache->sha_sequences);

  // sha_outputs
  memzero(&sha_256_ctx, sizeof(sha_256_ctx));
  sha256_Init(&sha_256_ctx);
  for (uint32_t idx = 0; idx < context->metadata.output_count; idx++) {
    sha256_Update(&sha_256_ctx, (uint8_t *)&context->outputs[idx].value, 8);
    sha256_Update(
        &sha_256_ctx, (uint8_t *)&context->outputs[idx].script_pub_key.size, 1);
    sha256_Update(&sha_256_ctx,
                  context->outputs[idx].script_pub_key.bytes,
                  context->outputs[idx].script_pub_key.size);
  }
  sha256_Final(&sha_256_ctx, taproot_cache->sha_outputs);

  taproot_cache->filled = true;
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
  } else if (SCRIPT_TYPE_P2TR == type) {
    status = calculate_p2tr_digest(context, index, digest);
  } else {
    status = false;
  }
  return status;
}

// BIP340 Schnorr signature for Taproot with tweaked private key
int schnorrsig_sign32_taproot(const uint8_t *private_key,
                              const uint8_t *public_key,
                              const uint8_t *digest,
                              uint8_t *signature_bytes) {
  // For Taproot, we need to compute the tweaked private key
  uint8_t tweaked_private_key[32] = {0};
  uint8_t aux[32] = {0};    // Use zero auxiliary data

  // Compute tweaked private key
  if (!bip340_tweak_private_key(
          private_key, public_key, NULL, tweaked_private_key)) {
    return -1;
  }

  // generate random aux data
  random_generate(aux, 32);

  int result =
      schnorrsig_sign(tweaked_private_key, aux, digest, signature_bytes);

  memzero(tweaked_private_key, sizeof(tweaked_private_key));
  memzero(aux, sizeof(aux));

  return result;
}
