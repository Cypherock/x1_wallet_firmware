/**
 * @file    starknet_txn_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for EVM txn helper functions
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

#include "curves.h"
#include "flash_config.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "starknet_api.h"
#include "starknet_context.h"
#include "starknet_crypto.h"
#include "starknet_main.h"
#include "starknet_priv.h"
#include "unity_fixture.h"
#include "usb_api_priv.h"
#include "utils.h"

TEST_GROUP(starknet_txn_test);

extern starknet_txn_context_t *txn_context;

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(starknet_txn_test) {
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(starknet_txn_test) {
  return;
}

TEST(starknet_txn_test, starknet_get_public_key_action) {
  starknet_query_t query = {
      .which_request = 1,
      .get_public_keys = {.which_request = 1,
                          .initiate = {
                              .derivation_paths_count = 1,
                              .derivation_paths = {{
                                  .path_count = 5,
                                  .path = {STARKNET_PURPOSE_INDEX,
                                           STARKNET_COIN_INDEX,
                                           STARKNET_ACCOUNT_INDEX,
                                           STARKNET_CHANGE_INDEX,
                                           1},
                              }},
                              .wallet_id = {},
                          }}};

  uint8_t seed[512 / 8];

  uint8_t public_keys[sizeof(query.get_public_keys.initiate.derivation_paths) /
                      sizeof(starknet_get_public_keys_derivation_path_t)]
                     [STARKNET_PUB_KEY_SIZE] = {0};

  const uint8_t expected_public_key[32];
  hex_string_to_byte_array(
      "4e7924acdb8f28d7997ac80c84891cd92599c1457510970e"    // stark private key
      "c5c08d4252d4790e",
      64,
      expected_public_key);
  hex_string_to_byte_array(
      "a185e44359c94014fa23b86741d089cdf7b75fa22a7b819e227a726d0cf19d29b19b16b4"
      "e9bd9d6f7d52e67d46eb2faa7d7258b6886b75aeb5e7825e97f26ea3",
      128,
      seed);

  TEST_ASSERT_TRUE(fill_starknet_public_keys(
      query.get_public_keys.initiate.derivation_paths,
      seed,
      public_keys,
      query.get_public_keys.initiate.derivation_paths_count));

  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_public_key, public_keys[0], 32);
}