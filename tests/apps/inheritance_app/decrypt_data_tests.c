/**
 * @file    decrypt_data_tests.c
 * @author  Cypherock X1 Team
 * @brief
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
#include <stdint.h>

#include "inheritance/core.pb.h"
#include "inheritance_priv.h"
#include "unity.h"
#include "unity_fixture.h"
#include "utils.h"

TEST_GROUP(inheritance_decryption_tests);

TEST_SETUP(inheritance_decryption_tests) {
}

TEST_TEAR_DOWN(inheritance_decryption_tests) {
}

// Wallet details used for testing:
// wallet_name="DEV002" - pin set
// wallet_id="cf19475f04c3b78f7d03f76f624e1ae426e6cd17ae1585e65c85a064312b2b88"

TEST(inheritance_decryption_tests, decryption_invalid_request) {
  inheritance_query_t query = {
      .encrypt = {
          .which_request =
              INHERITANCE_DECRYPT_DATA_WITH_PIN_REQUEST_ENCRYPTED_DATA_TAG}};    ///< Invalid request
  decryption_error_type_e status = inheritance_decrypt_data(&query);
  TEST_ASSERT_EQUAL(DECRYPTION_INVALID_REQUEST_ERROR, status);
}

// TODO: Valid test case, after get_encrypted_data();
// TEST(inheritance_decryption_tests, decryption_invalid_wallet_id) {
//   inheritance_query_t query = {
//       .encrypt = {.which_request =
//                       INHERITANCE_DECRYPT_DATA_WITH_PIN_REQUEST_INITIATE_TAG}};
//   hex_string_to_byte_array("cf19475f04c3b78f7d03f76f624e1ae426e6cd17ae1585e65c8"
//                            "5a064312b2b11",    ///< Invalid wallet id
//                            64,
//                            query.encrypt.initiate.wallet_id);
//   decryption_error_type_e status = inheritance_decrypt_data(&query);
//   TEST_ASSERT_EQUAL(DECRYPTION_INVALID_WALLET_ID_ERROR, status);
// }