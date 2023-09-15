/**
 * @file    solana_add_account_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for Solana Add Account Flow
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

#include "solana_api.h"
#include "solana_helpers.h"
#include "solana_priv.h"
#include "unity_fixture.h"
#include "wallet_list.h"

bool validate_request_data(solana_get_public_keys_request_t *request,
                           const pb_size_t which_request);
bool fill_public_keys(const solana_get_public_keys_derivation_path_t *paths,
                      const uint8_t *seed,
                      uint8_t public_keys[][SOLANA_PUB_KEY_SIZE],
                      pb_size_t count);

TEST_GROUP(solana_add_account_test);

TEST_SETUP(solana_add_account_test) {
}

TEST_TEAR_DOWN(solana_add_account_test) {
  return;
}

TEST(solana_add_account_test, solana_validate_req_action) {
  solana_query_t query = {
      .which_request = 1,
      .get_public_keys = {
          .which_request = 1,
          .initiate = {
              .derivation_paths_count = 1,
              .derivation_paths = {{
                  .path_count = 3,
                  .path = {SOLANA_PURPOSE_INDEX, SOLANA_COIN_INDEX, 0x80000000},
              }},
              .wallet_id = {},
          }}};

  const pb_size_t which_request = query.which_request;
  TEST_ASSERT_TRUE(
      validate_request_data(&query.get_public_keys, which_request));
}

TEST(solana_add_account_test, solana_get_addr_action) {
  solana_query_t query = {
      .which_request = 1,
      .get_public_keys = {
          .which_request = 1,
          .initiate = {
              .derivation_paths_count = 1,
              .derivation_paths = {{
                  .path_count = 3,
                  .path = {SOLANA_PURPOSE_INDEX, SOLANA_COIN_INDEX, 0x80000000},
              }},
              .wallet_id = {},
          }}};

  uint8_t seed[512 / 8];

  uint8_t public_keys[sizeof(query.get_public_keys.initiate.derivation_paths) /
                      sizeof(solana_get_public_keys_derivation_path_t)]
                     [SOLANA_PUB_KEY_SIZE] = {0};

  const char expected_addr[] = "AFnpWBz6yjbYnqUKjo5dSN2gDneo62TmeaFJUraLTP9V";
  char address[100] = "";
  size_t address_size = sizeof(address);

  hex_string_to_byte_array("0090ed0e7eccc22e9d60396e3debb38c"
                           "95128186493bb4d67750d7b2babf99fc"
                           "c9ca81fd0ae7719982611a0a28cf46b2"
                           "db1309c81f62be189bd2d1e5d06cd6fe",
                           128,
                           seed);

  TEST_ASSERT_TRUE(
      fill_public_keys(query.get_public_keys.initiate.derivation_paths,
                       seed,
                       public_keys,
                       query.get_public_keys.initiate.derivation_paths_count));

  b58enc(address, &address_size, (char *)(public_keys[0]), 32);

  TEST_ASSERT_EQUAL_STRING(expected_addr, address);
}
