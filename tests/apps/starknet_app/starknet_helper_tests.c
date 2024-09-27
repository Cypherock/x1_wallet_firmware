/**
 * @file    starknet_helper_tests.c
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
#include "starknet_helpers.h"
#include "starknet_main.h"
#include "starknet_priv.h"
#include "unity_fixture.h"
#include "usb_api_priv.h"
#include "utils.h"

TEST_GROUP(starknet_helper_test);

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(starknet_helper_test) {
  starknet_init();
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(starknet_helper_test) {
  return;
}

TEST(starknet_helper_test, starknet_pedersen_hash) {
  // Ref1:
  // https://github.com/starkware-libs/starkex-for-spot-trading/blob/607f0b4ce507e1d95cd018d206a2797f6ba4aab4/src/starkware/crypto/starkware/crypto/signature/test/config/signature_test_data.json
  // Ref2:
  // https://github.com/xJonathanLEI/starknet-rs/blob/f31e426a65225b9830bbf3c148f7ea05bf9dc257/starknet-crypto/src/pedersen_hash.rs

  uint8_t x[32], y[32], hash[32];

  hex_string_to_byte_array(
      "03d937c035c878245caf64531a5756109c53068da139362728feb561405371cb",
      64,
      x);
  hex_string_to_byte_array(
      "0208a0a10250e382e1e4bbe2880906c2791bf6275695e02fbbc6aeff9cd8b31a",
      64,
      y);

  const uint8_t expected_hash[32];
  hex_string_to_byte_array(
      "030e480bed5fe53fa909cc0f8c4d99b8f9f2c016be4c41e13a4848797979c662",
      64,
      expected_hash);

  TEST_ASSERT_TRUE(pederson_hash(x, y, 32, hash));

  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_hash, hash, 32);
}