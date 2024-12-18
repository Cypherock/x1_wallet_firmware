/**
 * @file    starknet_poseidon.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Starknet POSEIDON
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

#include "starknet_poseidon.h"

#include <error.pb.h>
#include <starknet/sign_txn.pb.h>
#include <stdint.h>
#include <string.h>

#include "coin_utils.h"
#include "f251.h"
#include "mini-gmp-helpers.h"
#include "mini-gmp.h"
#include "poseidon.h"
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
/**
 * @brief Converts BE hex array to felt
 *
 */
static void hex_to_felt_t(const uint8_t hex[],
                          const uint8_t hex_size,
                          felt_t felt);

/**
 * @brief Converts mpz num to felt
 *
 */
static void mpz_to_felt(felt_t felt, const mpz_t mpz);

/**
 * @brief Clears input felt state to 0
 *
 */
static void clear_state(felt_t *state, int size);

/**
 * @brief Computes Poseidon Hash on array of elements
 *
 * @param state array of felts to hash on
 * @param state_size size of array
 * @param res resultant hash
 *
 */
static void poseidon_hash_many(const felt_t state[],
                               const uint8_t state_size,
                               felt_t res);

/**
 * @brief Encode the L1 gas limits of a V3 transaction
 *
 * @param bounds object including the limits for L1
 * @param out encoded data
 */
static void encode_resource_bounds_l1(const starknet_resource_bounds_t bounds,
                                      felt_t out);

/**
 * @brief Encode the L2 gas limits of a V3 transaction
 *
 * @param bounds object including the limits for L2
 * @param out encoded data
 */
static void encode_resource_bounds_l2(const starknet_resource_bounds_t bounds,
                                      felt_t out);

/**
 * @brief Calculates posiedon_many([tip, L1bound, L2Bound]) required for
 * @ref calculate_transaction_hash_common
 *
 * @param bounds object including the limits for L1&L2
 * @param result result hash
 */
static void hash_fee_field(const pb_byte_t tip,
                           const starknet_resource_bounds_t bounds,
                           felt_t result);

/**
 * @brief Calculates hash of Data Availability Mode required for
 * @ref calculate_transaction_hash_common
 *
 * @param res result hash
 */
static void hash_DAMode(const pb_byte_t nonce_DAMode,
                        const pb_byte_t fee_DAMode,
                        felt_t res);

/**
 * @brief Common hash calculation for starknet txns
 *
 * @param hash result hash
 */
static void calculate_transaction_hash_common(
    const felt_t transaction_hash_prefix,
    const pb_byte_t tip[],
    const starknet_resource_bounds_t resource_bound,
    const pb_byte_t nonce_data_availability_mode[],
    const pb_byte_t fee_data_availability_mode[],
    const pb_byte_t version[],
    const pb_byte_t sender_address[],
    const pb_byte_t chain_id[],
    const pb_size_t chain_id_size,
    const pb_byte_t nonce[],
    const pb_size_t nonce_size,
    const felt_t additional_data[],
    const uint8_t additional_data_size,
    felt_t hash);

/**
 * @brief Prepares additional_data parameter required by
 * @ref calculate_transaction_hash_common for DEPLOY txns
 *
 * @param hash result hash
 */
static void calculate_deploy_transaction_hash(
    const starknet_sign_txn_deploy_account_txn_t *txn,
    felt_t hash);

/**
 * @brief Prepares additional_data parameter required by
 * @ref calculate_transaction_hash_common for INVOKE txns
 *
 * @param hash result hash
 */
static void calculate_invoke_transaction_hash(
    const starknet_sign_txn_invoke_txn_t *txn,
    felt_t hash);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void hex_to_felt_t(const uint8_t hex[],
                          const uint8_t hex_size,
                          felt_t felt) {
  uint8_t buf[32] = {0};
  memcpy(buf + (32 - hex_size), hex, hex_size);
  int offset = 0;
  for (int i = 0; i < 4; i++) {
    felt[3 - i] = U64_READ_BE_ARRAY(buf + offset);
    offset += 8;
  }
}

static void mpz_to_felt(felt_t felt, const mpz_t mpz) {
  uint8_t buf[32] = {0};
  mpz_to_byte_array(mpz, buf, 32);
  hex_to_felt_t(buf, 32, felt);
}

static void clear_state(felt_t *state, int size) {
  int i;

  for (i = 0; i < size; i++) {
    state[i][0] = 0;
    state[i][1] = 0;
    state[i][2] = 0;
    state[i][3] = 0;
  }
}

static void poseidon_hash_many(const felt_t state[],
                               const uint8_t state_size,
                               felt_t res) {
  ASSERT(state_size + 2 < 16);

  const uint8_t m = 3, rate = 2;
  felt_t padded[16];    ///< TODO: Update with macro
  clear_state(padded, 16);

  if (state != NULL) {
    for (int i = 0; i < state_size; i++) {
      f251_copy(padded[i], state[i]);
    }
  }
  uint8_t padded_offset = state_size;

  // padd one to mark end of ip
  felt_t felt_one = {1, 0, 0, 0};
  f251_copy(padded[padded_offset++], felt_one);
  // padd with zeros to make multiple of rate
  felt_t felt_zero = {0, 0, 0, 0};
  while (padded_offset % rate != 0) {
    f251_copy(padded[padded_offset++], felt_zero);
  }

  felt_t result[m];
  clear_state(result, m);

  for (int i = 0; i < padded_offset; i += rate) {
    for (int j = 0; j < rate; j++) {
      f251_add(result[j], result[j], padded[i + j]);
    }
    permutation_3(result);
  }
  // copt result
  f251_copy(res, result[0]);
}

static void encode_resource_bounds_l1(const starknet_resource_bounds_t bounds,
                                      felt_t out) {
  mpz_t temp1, temp2, result;
  mpz_init(result);
  mpz_init(temp1);
  mpz_init(temp2);

  // Perform the encoding:
  // (L1_GAS_NAME << RESOURCE_VALUE_OFFSET) + (bounds.level_1.max_amount <<
  // MAX_PRICE_PER_UNIT_BITS) + bounds.level_1.max_price_per_unit

  // L1_GAS_NAME << RESOURCE_VALUE_OFFSET
  mpz_set_str(result, L1_GAS_NAME, 16);
  mpz_mul_2exp(result, result, RESOURCE_VALUE_OFFSET);

  // bounds.level_1.max_amount << MAX_PRICE_PER_UNIT_BITS
  mpz_import(temp1,
             bounds.level_1.max_amount.size,
             1,
             1,
             1,
             0,
             bounds.level_1.max_amount.bytes);
  mpz_mul_2exp(temp1, temp1, MAX_PRICE_PER_UNIT_BITS);

  // result += temp1
  mpz_add(result, result, temp1);

  mpz_import(temp2,
             bounds.level_1.max_price_per_unit.size,
             1,
             1,
             1,
             0,
             bounds.level_1.max_price_per_unit.bytes);
  mpz_add(result, result, temp2);    // result += temp2

  mpz_to_felt(out, result);
  mpz_clear(temp1);
  mpz_clear(temp2);
  mpz_clear(result);
}

static void encode_resource_bounds_l2(const starknet_resource_bounds_t bounds,
                                      felt_t out) {
  mpz_t temp1, temp2, result;

  mpz_init(result);
  mpz_init(temp1);
  mpz_init(temp2);

  // Perform the encoding:
  // (L2_GAS_NAME << RESOURCE_VALUE_OFFSET) + (bounds.level_1.max_amount <<
  // MAX_PRICE_PER_UNIT_BITS) + bounds.level_2.max_price_per_unit

  // L1_GAS_NAME << RESOURCE_VALUE_OFFSET
  mpz_set_str(result, L2_GAS_NAME, 16);
  mpz_mul_2exp(result, result, RESOURCE_VALUE_OFFSET);

  // bounds.level_1.max_amount << MAX_PRICE_PER_UNIT_BITS
  mpz_import(temp1,
             bounds.level_2.max_amount.size,
             1,
             1,
             1,
             0,
             bounds.level_2.max_amount.bytes);
  mpz_mul_2exp(temp1, temp1, MAX_PRICE_PER_UNIT_BITS);

  // result += temp1
  mpz_add(result, result, temp1);

  mpz_import(temp2,
             bounds.level_2.max_price_per_unit.size,
             1,
             1,
             1,
             0,
             bounds.level_2.max_price_per_unit.bytes);
  mpz_add(result, result, temp2);    // result += temp2

  mpz_to_felt(out, result);

  mpz_clear(temp1);
  mpz_clear(temp2);
  mpz_clear(result);
}

static void hash_fee_field(const pb_byte_t tip,
                           const starknet_resource_bounds_t bounds,
                           felt_t result) {
  felt_t res_l1, res_l2;
  encode_resource_bounds_l1(bounds, res_l1);
  encode_resource_bounds_l2(bounds, res_l2);
  felt_t tip_felt = {tip, 0, 0, 0};

  felt_t state[3];
  clear_state(state, 3);
  f251_copy(state[0], tip_felt);
  f251_copy(state[1], res_l1);
  f251_copy(state[2], res_l2);

  poseidon_hash_many(state, 3, result);
}

static void hash_DAMode(const pb_byte_t nonce_DAMode,
                        const pb_byte_t fee_DAMode,
                        felt_t res) {
  mpz_t temp_nonce, temp_fee, result;

  mpz_init(result);
  mpz_init(temp_nonce);
  mpz_init(temp_fee);

  // Set the mpz_t variables from the pb_byte_t inputs
  mpz_set_ui(temp_nonce, nonce_DAMode);
  mpz_set_ui(temp_fee, fee_DAMode);

  // result = (temp_nonce << DATA_AVAILABILITY_MODE_BITS) + temp_fee
  mpz_mul_2exp(result, temp_nonce, DATA_AVAILABILITY_MODE_BITS);
  mpz_add(result, result, temp_fee);

  mpz_to_felt(res, result);

  mpz_clear(temp_nonce);
  mpz_clear(temp_fee);
  mpz_clear(result);
}

static void calculate_transaction_hash_common(
    const felt_t transaction_hash_prefix,
    const pb_byte_t tip[],
    const starknet_resource_bounds_t resource_bound,
    const pb_byte_t nonce_data_availability_mode[],
    const pb_byte_t fee_data_availability_mode[],
    const pb_byte_t version[],
    const pb_byte_t sender_address[],
    const pb_byte_t chain_id[],
    const pb_size_t chain_id_size,
    const pb_byte_t nonce[],
    const pb_size_t nonce_size,
    const felt_t additional_data[],
    const uint8_t additional_data_size,
    felt_t hash) {
  felt_t fee_field_hash = {0}, DAMode_hash = {0};
  hash_fee_field(tip[0], resource_bound, fee_field_hash);
  hash_DAMode(nonce_data_availability_mode[0],
              fee_data_availability_mode[0],
              DAMode_hash);

  // prepare data to hash array
  const uint8_t state_max = 15;
  felt_t state[state_max];
  uint8_t offset = 0;
  clear_state(state, state_max);

  f251_copy(state[offset++], transaction_hash_prefix);
  hex_to_felt_t(version, 1, state[offset++]);
  hex_to_felt_t(sender_address, 32, state[offset++]);
  f251_copy(state[offset++], fee_field_hash);
  felt_t paymaster_data_res = {0};
  poseidon_hash_many(
      NULL,
      0,
      paymaster_data_res);    ///< paymaster_data parameter for future use
                              ///< refer:
                              ///< https://docs.starknet.io/architecture-and-concepts/network-architecture/transactions/#v3_transaction_fields
  f251_copy(state[offset++], paymaster_data_res);
  hex_to_felt_t(chain_id, chain_id_size, state[offset++]);
  hex_to_felt_t(nonce, nonce_size, state[offset++]);
  f251_copy(state[offset++], DAMode_hash);
  if (additional_data != NULL) {
    for (uint8_t i = 0; i < additional_data_size; i++) {
      f251_copy(state[offset++], additional_data[i]);
      ASSERT(offset < state_max);
    }
  }

  poseidon_hash_many(state, offset, hash);
}

static void calculate_deploy_transaction_hash(
    const starknet_sign_txn_deploy_account_txn_t *txn,
    felt_t hash) {
  uint8_t hex[14] = DEPLOY_ACCOUNT_PREFIX;
  felt_t transaction_hash_prefix = {0};
  hex_to_felt_t(hex, 14, transaction_hash_prefix);

  // prepare additional data array
  const uint8_t data_max_count = 3;
  felt_t additional_data[data_max_count];
  clear_state(additional_data, data_max_count);

  // copy call data
  const uint8_t call_data_max_count = 10;
  felt_t call_data_felt[call_data_max_count];
  clear_state(call_data_felt, call_data_max_count);

  uint8_t count = txn->constructor_call_data.value_count;
  uint8_t offset;
  for (offset = 0; offset < count; offset++) {
    hex_to_felt_t(txn->constructor_call_data.value[offset].bytes,
                  txn->constructor_call_data.value[offset].size,
                  call_data_felt[offset]);
  }
  poseidon_hash_many(call_data_felt, offset, additional_data[0]);

  hex_to_felt_t(txn->class_hash, 32, additional_data[1]);
  hex_to_felt_t(txn->salt, 32, additional_data[2]);

  calculate_transaction_hash_common(transaction_hash_prefix,
                                    txn->tip,
                                    txn->resource_bounds,
                                    txn->nonce_data_availability_mode,
                                    txn->fee_data_availability_mode,
                                    txn->version,
                                    txn->contract_address,
                                    txn->chain_id.bytes,
                                    txn->chain_id.size,
                                    txn->nonce.bytes,
                                    txn->nonce.size,
                                    additional_data,
                                    3,
                                    hash);
}

static void calculate_invoke_transaction_hash(
    const starknet_sign_txn_invoke_txn_t *txn,
    felt_t hash) {
  uint8_t hex[6] = INVOKE_TXN_PREFIX;
  felt_t transaction_hash_prefix = {0};
  hex_to_felt_t(hex, 6, transaction_hash_prefix);

  // prepare additional data array
  const uint8_t data_max_count = 2;
  felt_t additional_data[data_max_count];
  clear_state(additional_data, data_max_count);

  // Note: currently account_deployment_data is unused
  felt_t account_deployment_data = {0};
  poseidon_hash_many(NULL, 0, account_deployment_data);
  f251_copy(additional_data[0], account_deployment_data);

  // copy call data
  const uint8_t call_data_max_count = 10;
  felt_t call_data_felt[call_data_max_count];
  clear_state(call_data_felt, call_data_max_count);

  uint8_t count = txn->calldata.value_count;
  uint8_t offset;
  for (offset = 0; offset < count; offset++) {
    hex_to_felt_t(txn->calldata.value[offset].bytes,
                  txn->calldata.value[offset].size,
                  call_data_felt[offset]);
  }
  poseidon_hash_many(call_data_felt, offset, additional_data[1]);

  calculate_transaction_hash_common(transaction_hash_prefix,
                                    txn->tip,
                                    txn->resource_bound,
                                    txn->nonce_data_availability_mode,
                                    txn->fee_data_availability_mode,
                                    txn->version,
                                    txn->sender_address,
                                    txn->chain_id.bytes,
                                    txn->chain_id.size,
                                    txn->nonce.bytes,
                                    txn->nonce.size,
                                    additional_data,
                                    2,
                                    hash);
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void felt_t_to_hex(const felt_t felt, uint8_t hex[32]) {
  int offset = 0;
  for (int i = 0; i < 4; i++) {
    uint64_t value = felt[3 - i];

    for (int j = 0; j < 8; j++) {
      hex[offset + j] = (uint8_t)((value >> (56 - j * 8)) & 0xFF);
    }
    offset += 8;
  }
}

void calculate_txn_hash(void *txn, pb_size_t type, felt_t hash) {
  ASSERT(txn != NULL);
  switch (type) {
    case STARKNET_SIGN_TXN_UNSIGNED_TXN_INVOKE_TXN_TAG: {
      calculate_invoke_transaction_hash((starknet_sign_txn_invoke_txn_t *)txn,
                                        hash);
    } break;

    case STARKNET_SIGN_TXN_UNSIGNED_TXN_DEPLOY_TXN_TAG: {
      calculate_deploy_transaction_hash(
          (starknet_sign_txn_deploy_account_txn_t *)txn, hash);

    } break;
  }
}