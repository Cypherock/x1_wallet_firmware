/**
 * @file    utils_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for utility functions
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

#include "unity_fixture.h"
#include "utils.h"

TEST_GROUP(utils_tests);

TEST_SETUP(utils_tests) {
  return;
}

TEST_TEAR_DOWN(utils_tests) {
  return;
}

TEST(utils_tests, der_to_sig_1) {
  uint8_t der_encoded_signature[300] = {0};
  uint8_t expected_signature[64] = {0};

  // DER signature reference: https://asecuritysite.com/digitalcert/sigs5
  hex_string_to_byte_array("3046022100e4e87c417196c6e5cd63f93e94929ccda6d04fc0a"
                           "7446922baf3070e854ec4f4022100a1ecd098008329de9bc93f"
                           "b2ded6aaceecc921f7183d6b3cfc673b3ef8af219e",
                           144,
                           der_encoded_signature);

  hex_string_to_byte_array(
      "e4e87c417196c6e5cd63f93e94929ccda6d04fc0a7446922baf3070e854ec4f4a1ecd098"
      "008329de9bc93fb2ded6aaceecc921f7183d6b3cfc673b3ef8af219e",
      128,
      expected_signature);

  uint8_t signature[64] = {0};
  der_to_sig(der_encoded_signature, signature);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      expected_signature, signature, sizeof(expected_signature));
}

TEST(utils_tests, der_to_sig_2) {
  uint8_t der_encoded_signature[300] = {0};
  uint8_t expected_signature[64] = {0};

  // Modified len: 2-bytes from r segment and 1-byte from s segment
  hex_string_to_byte_array(
      "3046021F007c417196c6e5cd63f93e94929ccda6d04fc0a7446922baf3070e854ec4f402"
      "2000ecd098008329de9bc93fb2ded6aaceecc921f7183d6b3cfc673b3ef8af219e",
      138,
      der_encoded_signature);

  hex_string_to_byte_array(
      "00007c417196c6e5cd63f93e94929ccda6d04fc0a7446922baf3070e854ec4f400ecd098"
      "008329de9bc93fb2ded6aaceecc921f7183d6b3cfc673b3ef8af219e",
      128,
      expected_signature);

  uint8_t signature[64] = {0};
  der_to_sig(der_encoded_signature, signature);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      expected_signature, signature, sizeof(expected_signature));
}