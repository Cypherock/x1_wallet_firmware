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
#include "unity_fixture.h"
#include "wallet_list.h"

bool validate_request_data(bittensor_get_public_keys_request_t *request,
                           const pb_size_t which_request);
bool fill_public_keys(const bittensor_get_public_keys_derivation_path_t *paths,
                      const uint8_t *seed,
                      uint8_t public_keys[][BITTENSOR_PUB_KEY_SIZE],
                      pb_size_t count);

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

#define SS58_BLAKE_PREFIX (const unsigned char *)"SS58PRE"
#define SS58_BLAKE_PREFIX_LEN 7
#define SS58_ADDRESS_MAX_LEN 60u
#define PK_LEN_25519 32u
uint16_t crypto_SS58EncodePubkey(uint8_t *buffer,
                                 uint16_t buffer_len,
                                 uint16_t addressType,
                                 const uint8_t *pubkey);
TEST(bittensor_add_account_test, bittensor_get_addr_action) {
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

  uint8_t public_keys[sizeof(query.get_public_keys.initiate.derivation_paths) /
                      sizeof(bittensor_get_public_keys_derivation_path_t)]
                     [BITTENSOR_PUB_KEY_SIZE] = {0};

  char address[100] = "";
  size_t address_size = sizeof(address);

  uint8_t seed[512 / 8];
  uint8_t pubkey[100];
  char PubHash[64];

  char *mnemonic = "sample split bamboo west visual approve brain fox arch "
                   "impact relief smile";
  const char expected_addr[] =
      "5CSbZ7wG456oty4WoiX6a1J88VUbrCXLhrKVJ9q95BsYH4TZ";

  mnemonic_to_seed(mnemonic, "", seed, 0);

  hex_string_to_byte_array(
      "9fa1ab1d37025d8c3cd596ecbf50435572eeaeb1785a0c9ed2b22afa4c378d6a",
      64,
      seed)

      // get public key
      TEST_ASSERT_TRUE(fill_public_keys(
          query.get_public_keys.initiate.derivation_paths,
          seed,
          public_keys,
          query.get_public_keys.initiate.derivation_paths_count));

  u8ToHexStr("pubkey received", (uint8_t *)(public_keys[0]), PK_LEN_25519);
  u8ToHexStr("seed", seed, 512 / 8);

  // Print the values of the public_keys array
  // printf("\npubkey: %s", (char *)(public_keys[0]));
  for (size_t i = 0; i < 1; ++i) {
    printf("\n1st PubKey : 0x%zu", i);
    for (size_t j = 0; j < BITTENSOR_PUB_KEY_SIZE; ++j) {
      pubkey[i] = public_keys[i][j];
      printf("%02X", public_keys[i][j]);
    }
    printf("\n");
  }

  // get address from pubkey
  memzero(address, PK_LEN_25519);
  uint16_t outlen = crypto_SS58EncodePubkey(
      address, PK_LEN_25519, 0, (uint8_t *)(public_keys[0]));

  printf("\naddress func: %s\n\n", address);
  // TEST_ASSERT_EQUAL_STRING(expected_addr, address);

  // manual conversion
  char pk_hex[(36 * 2) + 1] = "0010b22ebe89b321370bee8d39d5c5d411daf1e8fc91c9d1"
                              "534044590f1f966ebc0000000";
  uint8_t pk[36];
  hex_string_to_byte_array(pk_hex, (36 * 2) + 1, pk);
  int prefixSize = 1;
  if (0 == ss58hash(pk, 32, PubHash, 64)) {
    u8ToHexStr("pubHash", PubHash, 64);

    pk[33] = PubHash[0];
    pk[34] = PubHash[1];

    u8ToHexStrr("pk", pk, 36, pk_hex);

    memzero(address, PK_LEN_25519);
    b58enc(address, &address_size, pk_hex, 33 + 2);
  }
  printf("\naddress manual: %s\n\n", address);
  TEST_ASSERT_EQUAL_STRING(expected_addr, address);
}

int ss58hash(const unsigned char *in,
             unsigned int inLen,
             unsigned char *out,
             unsigned int outLen) {
  blake2b_state s;
  blake2b_Init(&s, 64);
  blake2b_Update(&s, SS58_BLAKE_PREFIX, SS58_BLAKE_PREFIX_LEN);
  blake2b_Update(&s, in, inLen);
  blake2b_Final(&s, out, outLen);
  return 0;
}

uint16_t crypto_SS58EncodePubkey(uint8_t *buffer,
                                 uint16_t buffer_len,
                                 uint16_t addressType,
                                 const uint8_t *pubkey) {
  // based on https://docs.substrate.io/v3/advanced/ss58/
  if (buffer == NULL || buffer_len < SS58_ADDRESS_MAX_LEN) {
    return 0;
  }
  if (pubkey == NULL) {
    return 0;
  }
  memzero(buffer, buffer_len);

  uint8_t hash[64] = {0};
  uint8_t unencoded[36] = {0};

  const uint8_t prefixSize = crypto_SS58CalculatePrefix(addressType, unencoded);
  if (prefixSize == 0) {
    return 0;
  }

  memcpy(unencoded + prefixSize, pubkey, 32);    // account id
  u8ToHexStr("unencoded p", unencoded, 36);
  if (ss58hash((uint8_t *)unencoded, 32 + prefixSize, hash, 64) != 0) {
    memzero(unencoded, sizeof(unencoded));
    return 0;
  }
  unencoded[32 + prefixSize] = hash[0];
  unencoded[33 + prefixSize] = hash[1];

  u8ToHexStr("hash", hash, 64);
  u8ToHexStr("unencoded", unencoded, 36);

  char unencoded_str[36 * 2 + 1];
  u8ToHexStrr("unencoded str", unencoded, 36, unencoded_str);

  size_t outLen = buffer_len;
  if (b58enc(buffer, &buffer_len, unencoded_str, 34 + prefixSize) != 0) {
    memzero(unencoded, sizeof(unencoded));
    return 0;
  }

  return outLen;
}

uint8_t crypto_SS58CalculatePrefix(uint16_t addressType, uint8_t *prefixBytes) {
  if (addressType > 16383) {
    return 0;
  }

  if (addressType > 63) {
    prefixBytes[0] = 0x40 | ((addressType >> 2) & 0x3F);
    prefixBytes[1] = ((addressType & 0x3) << 6) + ((addressType >> 8) & 0x3F);
    return 2;
  }

  prefixBytes[0] = addressType & 0x3F;    // address type
  return 1;
}

void u8ToHexStr(const char *name, const uint8_t *data, size_t size) {
  char hexstring[size * 2 + 1];
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