/**
 * @file    starknet_poseidon.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Starknet chains
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
void hex_to_felt_t(const uint8_t hex[], const uint8_t hex_size, felt_t felt) {
  uint8_t buf[32] = {0};
  memcpy(buf + (32 - hex_size), hex, hex_size);
  int offset = 0;
  for (int i = 0; i < 4; i++) {
    felt[3 - i] = U64_READ_BE_ARRAY(buf + offset);
    offset += 8;
  }
}

void felt_t_to_hex(const felt_t felt, uint8_t hex[32]) {
  int offset = 0;
  for (int i = 0; i < 4; i++) {
    uint64_t value = felt[3 - i];

    // Break the uint64_t value into 8 bytes and store in hex array in
    // Big-Endian order
    for (int j = 0; j < 8; j++) {
      hex[offset + j] = (uint8_t)((value >> (56 - j * 8)) & 0xFF);
    }

    offset += 8;
  }
}

void mpz_to_felt(felt_t felt, const mpz_t mpz) {
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

static void print_state(felt_t *state, int size) {
  int i;

  for (i = 0; i < size; i++) {
    printf("%016" PRIx64, state[i][3]);
    printf("%016" PRIx64, state[i][2]);
    printf("%016" PRIx64, state[i][1]);
    printf("%016" PRIx64, state[i][0]);
    printf("\n");
  }
}

void poseidon_hash_many(const felt_t state[],
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

void encode_resource_bounds_l1(const starknet_resource_bounds_t bounds,
                               felt_t out) {
  mpz_t temp1, temp2, result;
  mpz_init(result);
  mpz_init(temp1);
  mpz_init(temp2);

  // Perform the encoding:
  // (L1_GAS_NAME << RESOURCE_VALUE_OFFSET) + (bounds.level_1.max_amount <<
  // MAX_PRICE_PER_UNIT_BITS) + bounds.level_1.max_price_per_unit

  // L1_GAS_NAME << RESOURCE_VALUE_OFFSET
  mpz_set_ui(result, L1_GAS_NAME);
  mpz_mul_2exp(result, result, RESOURCE_VALUE_OFFSET);

  // bounds.level_1.max_amount << MAX_PRICE_PER_UNIT_BITS
  mpz_import(temp1, 5, 1, sizeof(pb_byte_t), 0, 0, bounds.level_1.max_amount);
  mpz_mul_2exp(temp1, temp1, MAX_PRICE_PER_UNIT_BITS);

  // result += temp1
  mpz_add(result, result, temp1);

  mpz_import(
      temp2, 5, 1, sizeof(pb_byte_t), 0, 0, bounds.level_1.max_price_per_unit);
  mpz_add(result, result, temp2);    // result += temp2

  mpz_to_felt(out, result);
  mpz_clear(temp1);
  mpz_clear(temp2);
}

void encode_resource_bounds_l2(const starknet_resource_bounds_t bounds,
                               felt_t out) {
  mpz_t temp1, temp2, result;

  mpz_init(result);
  mpz_init(temp1);
  mpz_init(temp2);

  // Perform the encoding:
  // (L2_GAS_NAME << RESOURCE_VALUE_OFFSET) + (bounds.level_1.max_amount <<
  // MAX_PRICE_PER_UNIT_BITS) + bounds.level_2.max_price_per_unit

  // L1_GAS_NAME << RESOURCE_VALUE_OFFSET
  mpz_set_ui(result, L2_GAS_NAME);
  mpz_mul_2exp(result, result, RESOURCE_VALUE_OFFSET);

  // bounds.level_1.max_amount << MAX_PRICE_PER_UNIT_BITS
  mpz_import(temp1, 5, 1, sizeof(pb_byte_t), 0, 0, bounds.level_2.max_amount);
  mpz_mul_2exp(temp1, temp1, MAX_PRICE_PER_UNIT_BITS);

  // result += temp1
  mpz_add(result, result, temp1);

  mpz_import(
      temp2, 5, 1, sizeof(pb_byte_t), 0, 0, bounds.level_2.max_price_per_unit);
  mpz_add(result, result, temp2);    // result += temp2

  mpz_to_felt(out, result);

  mpz_clear(temp1);
  mpz_clear(temp2);
}

void hash_fee_field(const pb_byte_t tip,
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

void hash_DAMode(const pb_byte_t nonce_DAMode,
                 const pb_byte_t fee_DAMode,
                 felt_t out) {
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

  mpz_to_felt(out, result);

  mpz_clear(temp_nonce);
  mpz_clear(temp_fee);
}

void calculate_transaction_hash_common(felt_t transaction_hash_prefix,
                                       starknet_sign_txn_unsigned_txn_t *txn,
                                       felt_t additional_data[],
                                       uint8_t additional_data_size,
                                       felt_t hash) {
  felt_t fee_field_hash = {0}, DAMode_hash = {0};
  hash_fee_field(txn->tip[0], txn->resource_bound, fee_field_hash);
  hash_DAMode(txn->nonce_data_availability_mode[0],
              txn->fee_data_availability_mode[0],
              DAMode_hash);

  // prepare data to hash array
  const uint8_t state_max = 15;
  felt_t state[state_max];
  uint8_t offset = 0;
  clear_state(state, state_max);

  f251_copy(state[offset++], transaction_hash_prefix);
  hex_to_felt_t(txn->version, 1, state[offset++]);
  hex_to_felt_t(txn->sender_address, 32, state[offset++]);
  f251_copy(state[offset++], fee_field_hash);
  felt_t paymaster_data_res = {0};
  poseidon_hash_many(
      NULL,
      0,
      paymaster_data_res);    ///< paymaster_data parameter for future use
                              ///< refer:
                              ///< https://docs.starknet.io/architecture-and-concepts/network-architecture/transactions/#v3_transaction_fields
  f251_copy(state[offset++], paymaster_data_res);
  hex_to_felt_t(txn->chain_id, 1, state[offset++]);
  hex_to_felt_t(txn->nonce, 10, state[offset++]);
  f251_copy(state[offset++], DAMode_hash);
  if (additional_data != NULL) {
    for (uint8_t i = 0; i < additional_data_size; i++) {
      f251_copy(state[offset++], additional_data[i]);
      ASSERT(offset < state_max);
    }
  }

  poseidon_hash_many(state, offset, hash);
}