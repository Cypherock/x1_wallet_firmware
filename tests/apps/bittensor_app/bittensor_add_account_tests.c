/**
 * @file    bittensor_add_account_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for Bittensor Add Account Flow
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

#include "bittensor_api.h"
#include "bittensor_helpers.h"
#include "bittensor_priv.h"
#include "ed25519-hash-custom.h"
#include "pbkdf2.h"
#include "unity_fixture.h"
#include "wallet_list.h"

#define SS58_BLAKE_PREFIX (const unsigned char *)"SS58PRE"
#define SS58_BLAKE_PREFIX_LEN 7
#define SS58_ADDRESS_MAX_LEN 60u
#define PK_LEN_25519 32u

bool validate_request_data(bittensor_get_public_keys_request_t *request,
                           const pb_size_t which_request);
bool fill_public_keys(const bittensor_get_public_keys_derivation_path_t *paths,
                      const uint8_t *seed,
                      uint8_t public_keys[][BITTENSOR_PUB_KEY_SIZE],
                      pb_size_t count);
bool ss58enc(char *buffer,
             uint16_t buffer_len,
             uint16_t addressType,
             const uint8_t *pubkey);

TEST_GROUP(bittensor_add_account_test);

TEST_SETUP(bittensor_add_account_test) {
}

TEST_TEAR_DOWN(bittensor_add_account_test) {
  return;
}

TEST(bittensor_add_account_test, bittensor_validate_req_action) {
  bittensor_query_t query = {
      .which_request = 1,
      .get_public_keys = {.which_request = 1,
                          .initiate = {
                              .derivation_paths_count = 1,
                              .derivation_paths = {{
                                  .path_count = 3,
                                  .path = {BITTENSOR_PURPOSE_INDEX,
                                           BITTENSOR_COIN_INDEX,
                                           0x80000000},
                              }},
                              .wallet_id = {},
                          }}};
  const pb_size_t which_request = query.which_request;
  TEST_ASSERT_TRUE(
      validate_request_data(&query.get_public_keys, which_request));
}

TEST(bittensor_add_account_test, bittensor_get_seckey_action) {
  uint8_t expected_seckey[32] = {0};
  hex_string_to_byte_array(
      "9fa1ab1d37025d8c3cd596ecbf50435572eeaeb1785a0c9ed2b22afa4c378d6a",
      64,
      expected_seckey);

  char *mnemonic = "sample split bamboo west visual approve brain fox arch "
                   "impact relief smile";
  uint8_t entropy[64] = {0};
  mnemonic_to_entropy(mnemonic, entropy);
  entropy[16] = 0;

  uint8_t secret_key[32] = {0};
  mnemonic_to_seed(entropy, "", secret_key, NULL);

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_seckey, secret_key, 32);
}

TEST(bittensor_add_account_test, bittensor_get_pubkey_action) {
  uint8_t expected_pubkey[32] = {0};
  hex_string_to_byte_array(
      "10b22ebe89b321370bee8d39d5c5d411daf1e8fc91c9d1534044590f1f966ebc",
      64,
      expected_pubkey);

  uint8_t secret_key[32] = {0};
  hex_string_to_byte_array(
      "9fa1ab1d37025d8c3cd596ecbf50435572eeaeb1785a0c9ed2b22afa4c378d6a",
      64,
      secret_key);

  uint8_t public_key[32] = {0};
  ed25519_publickey(secret_key, public_key);

  // char public_key_hex[65] = {0};
  // byte_array_to_hex_string(public_key, 32, public_key_hex, 65);

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_pubkey, public_key, 32);
}

TEST(bittensor_add_account_test, bittensor_get_addr_action) {
  uint8_t expected_addr[24] = {0};
  hex_string_to_byte_array(
      "5CSbZ7wG456oty4WoiX6a1J88VUbrCXLhrKVJ9q95BsYH4TZ", 48, expected_addr);

  uint8_t public_key[32] = {0};
  hex_string_to_byte_array(
      "10b22ebe89b321370bee8d39d5c5d411daf1e8fc91c9d1534044590f1f966ebc",
      64,
      public_key);

  char address[100] = "";
  size_t address_size = sizeof(address);
  ss58enc(address, &address_size, 42, public_key);

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_addr, address, 24);
}

TEST(bittensor_add_account_test, bittensor_get_sig_action) {
  uint8_t expected_sig[64] = {0};
  hex_string_to_byte_array(
      "98a70222f0b8121aa9d30f813d683f809e462b469c7ff87639499bb94e6dae4131f85042"
      "463c2a355a2003d062adf5aaa10b8c61e636062aaad11c2a26083406",
      128,
      expected_sig);

  size_t unsigned_txn_size = 3;
  uint8_t unsigned_txn[30] = {0};
  hex_string_to_byte_array("616263", 6, unsigned_txn);
  u8ToHexStr("unsigned_txn", unsigned_txn, 3);

  ed25519_public_key public_key = {0};
  hex_string_to_byte_array(
      "ec172b93ad5e563bf4932c70e1245034c35467ef2efd4d64ebf819683467e2bf",
      64,
      public_key);
  u8ToHexStr("public_key", public_key, 32);

  ed25519_secret_key secret_key = {0};
  hex_string_to_byte_array(
      "833fe62409237b9d62ec77587520911e9a759cec1d19755b7da901b96dca3d42",
      64,
      secret_key);
  u8ToHexStr("secret_keys", secret_key, 32);

  // #define ed25519_hash_context blake256
  // #define ed25519_hash_init(ctx) blake256_Init(ctx)
  // #define ed25519_hash_update(ctx, in, inlen) blake256_Update((ctx),
  // (in),(inlen)) #define ed25519_hash_final(ctx, hash) blake256_Final((ctx),
  // (hash)) #define ed25519_hash(hash, in, inlen) blake256_Raw((in), (inlen),
  // (hash))

  ed25519_signature sig_ed25519 = {0};
  ed25519_sign(
      unsigned_txn, unsigned_txn_size, secret_key, public_key, sig_ed25519);

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_sig, sig_ed25519, 64);
}

void u8ToHexStrr(const char *name,
                 const uint8_t *data,
                 size_t size,
                 char *hexstring) {
  // char hexstring[size*2+1];
  for (size_t i = 0; i < size; ++i) {
    for (size_t i = 0; i < size; ++i) {
      sprintf(hexstring + 2 * i,
              "%02x",
              data[i]);    // Each byte represented by 2 characters + '\0'
    }
  }
  hexstring[size * 2] = '\0';    // Null-terminate the string
  printf("\n%s : 0x%s", name, hexstring);
}

void uint8_to_char(uint8_t *uint8_array, char *char_array, size_t length) {
  for (size_t i = 0; i < length; i++) {
    char_array[i] = (char)uint8_array[i];
  }
}