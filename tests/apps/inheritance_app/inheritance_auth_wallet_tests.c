/**
 * @file    inheritance_helper_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for Bitcoin helper functions
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
#include <string.h>

#include "inheritance/core.pb.h"
#include "inheritance_main.h"
#include "inheritance_priv.h"
#include "unity.h"
#include "unity_fixture.h"
#include "wallet.h"

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
 * STATIC VARIABLES
 *****************************************************************************/
static auth_wallet_config_t *auth = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
TEST_GROUP(inheritance_auth_wallet_tests);

TEST_SETUP(inheritance_auth_wallet_tests) {
}

TEST_TEAR_DOWN(inheritance_auth_wallet_tests) {
}

// Wallet details used for testing:
// wallet_name="DEV002"
// wallet_id="cf19475f04c3b78f7d03f76f624e1ae426e6cd17ae1585e65c85a064312b2b88"

TEST(inheritance_auth_wallet_tests, auth_wallet_valid_input_fail) {
  inheritance_query_t query = {
      .auth_wallet = {INHERITANCE_AUTH_WALLET_REQUEST_INITIATE_TAG,
                      .initiate =
                          {
                              .do_seed_based = false,
                              .do_wallet_based = false,
                          }},
  };
  hex_string_to_byte_array("cf19475f04c3b78f7d03f76f624e1ae426e6cd17ae1585e65c8"
                           "5a064312b2b88",    // DEV002
                           64,
                           query.auth_wallet.initiate.wallet_id);
  hex_string_to_byte_array("c35697f94b98b722f94283c1be4674b9bc2b89eae4ed590888a"
                           "7b9e7379bc80d",    // SHA256("challenge001")
                           64,
                           query.auth_wallet.initiate.challenge.bytes);
  query.auth_wallet.initiate.challenge.size = 32;

  auth_wallet_error_type_e status = inheritance_auth_wallet(&query);
  TEST_ASSERT_EQUAL(AUTH_WALLET_INVALID_INPUT_ERROR, status);
}

TEST(inheritance_auth_wallet_tests, auth_wallet_which_request_fail) {
  printf("auth wallet running");
  inheritance_query_t query;
  memset(&query, 0, sizeof(query));
  hex_string_to_byte_array("cf19475f04c3b78f7d03f76f624e1ae426e6cd17ae1585e65c8"
                           "5a064312b2b88",    // DEV002
                           64,
                           query.auth_wallet.initiate.wallet_id);
  hex_string_to_byte_array("c35697f94b98b722f94283c1be4674b9bc2b89eae4ed590888a"
                           "7b9e7379bc80d",    // SHA256("challenge001")
                           64,
                           query.auth_wallet.initiate.challenge.bytes);
  query.auth_wallet.initiate.challenge.size = 32;

  auth_wallet_error_type_e status = inheritance_auth_wallet(&query);
  TEST_ASSERT_EQUAL(AUTH_WALLET_INVALID_INPUT_ERROR, status);
}

TEST(inheritance_auth_wallet_tests, auth_wallet_valid_wallet_id_fail) {
  inheritance_query_t query = {
      .auth_wallet = {INHERITANCE_AUTH_WALLET_REQUEST_INITIATE_TAG,
                      .initiate =
                          {
                              .do_seed_based = false,
                              .do_wallet_based = true,
                          }},
  };
  hex_string_to_byte_array("aff4423b5b3061beb079941bfc4cc8d039083721575d3859748"
                           "fa4781c6d6c57",    //  wallet not simulated
                           64,
                           query.auth_wallet.initiate.wallet_id);

  hex_string_to_byte_array("c35697f94b98b722f94283c1be4674b9bc2b89eae4ed590888a"
                           "7b9e7379bc80d",    // SHA256("challenge001")
                           64,
                           query.auth_wallet.initiate.challenge.bytes);
  query.auth_wallet.initiate.challenge.size = 32;
#ifdef DEV_BUILD
  ekp_enqueue(LV_KEY_DOWN, DEFAULT_DELAY);
  ekp_enqueue(LV_KEY_ENTER, DEFAULT_DELAY);
#endif
  auth_wallet_error_type_e status = inheritance_auth_wallet(&query);
  TEST_ASSERT_EQUAL(AUTH_WALLET_INVALID_WALLET_ID_ERROR, status);
}

TEST(inheritance_auth_wallet_tests, auth_wallet_min_challenge_length_fail) {
  inheritance_query_t query = {
      .auth_wallet = {INHERITANCE_AUTH_WALLET_REQUEST_INITIATE_TAG,
                      .initiate =
                          {
                              .do_seed_based = false,
                              .do_wallet_based = true,
                          }},
  };
  hex_string_to_byte_array("cf19475f04c3b78f7d03f76f624e1ae426e6cd17ae1585e65c8"
                           "5a064312b2b88",    // DEV002
                           64,
                           query.auth_wallet.initiate.wallet_id);
  hex_string_to_byte_array(
      "c35697f94b98b722f94283c1be4674",    // [:14]SHA256("challenge001")
      30,
      query.auth_wallet.initiate.challenge.bytes);
  query.auth_wallet.initiate.challenge.size = 15;

  auth_wallet_error_type_e status = inheritance_auth_wallet(&query);
  TEST_ASSERT_EQUAL(AUTH_WALLET_INVALID_INPUT_ERROR, status);
}

TEST(inheritance_auth_wallet_tests, auth_wallet_max_challenge_length_fail) {
  inheritance_query_t query = {
      .auth_wallet = {INHERITANCE_AUTH_WALLET_REQUEST_INITIATE_TAG,
                      .initiate =
                          {
                              .do_seed_based = false,
                              .do_wallet_based = true,
                          }},
  };
  // For Simulator testing, the wallet id should be of DEV002
  hex_string_to_byte_array("aff4423b5b3061beb079941bfc4cc8d039083721575d3859748"
                           "fa4781c6d6c57",
                           64,
                           query.auth_wallet.initiate.wallet_id);
  hex_string_to_byte_array("c35697f94b98b722f94283c1be4674b9bc2b89eae4ed590888a"
                           "7b9e7379bc80dff",    // SHA256("challenge001") + ff
                           66,
                           query.auth_wallet.initiate.challenge.bytes);
  query.auth_wallet.initiate.challenge.size = 33;

  auth_wallet_error_type_e status = inheritance_auth_wallet(&query);
  TEST_ASSERT_EQUAL(AUTH_WALLET_INVALID_INPUT_ERROR, status);
}

TEST(inheritance_auth_wallet_tests, auth_wallet_ok) {
  inheritance_query_t query = {
      .auth_wallet = {INHERITANCE_AUTH_WALLET_REQUEST_INITIATE_TAG,
                      .initiate =
                          {
                              .do_seed_based = false,
                              .do_wallet_based = true,
                          }},
  };
  hex_string_to_byte_array("cf19475f04c3b78f7d03f76f624e1ae426e6cd17ae1585e65c8"
                           "5a064312b2b88",    // DEV002
                           64,
                           query.auth_wallet.initiate.wallet_id);
  hex_string_to_byte_array("c35697f94b98b722f94283c1be4674b9bc2b89eae4ed590888a"
                           "7b9e7379bc80d",    // SHA256("challenge001")
                           64,
                           query.auth_wallet.initiate.challenge.bytes);
  query.auth_wallet.initiate.challenge.size = 32;

  auth_wallet_error_type_e status = inheritance_auth_wallet(&query);
  TEST_ASSERT_EQUAL(AUTH_WALLET_OK, status);
}
