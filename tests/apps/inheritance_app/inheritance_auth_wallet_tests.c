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
// inavlid request--
// both baswd false--
// invalid wallet id
// inalvid challenge length
// Abort error
// dev001 - no pin no pass phrase
// dev002 - pin no passphrase
// dev003 - pin passphrase
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
  // For Simulator testing, the wallet id should be of DEV002
  hex_string_to_byte_array("aff4423b5b3061beb079941bfc4cc8d039083721575d3859748"
                           "fa4781c6d6c57",    //  wallet not simulated
                           64,
                           query.auth_wallet.initiate.wallet_id);

  hex_string_to_byte_array("c35697f94b98b722f94283c1be4674b9bc2b89eae4ed590888a"
                           "7b9e7379bc80d",    // SHA256("challenge001")
                           64,
                           query.auth_wallet.initiate.challenge.bytes);
  query.auth_wallet.initiate.challenge.size = 32;

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

// TEST(inheritance_auth_wallet_tests, wallet_auth_get_pairs_action) {
//   auth->entropy_size = 68;
//   hex_string_to_byte_array(
//       "2146edf67877bfb47614e82fce98d16e588400af276b7e5032752189d747ff6d1efe",
//       auth->entropy_size,
//       auth->entropy);

//   wallet_auth_get_pairs(auth);

//   uint8_t expected_public_key[32] = {0};
//   hex_string_to_byte_array(
//       "a0d13296a72d3d9dfce4adf908368864a33c6a39767aa040906fa728bf08109c",
//       64,
//       expected_public_key);

//   uint8_t public_key[64] = {0};
//   TEST_ASSERT_EQUAL_UINT8_ARRAY(
//       expected_public_key, auth->public_key, sizeof(expected_public_key));
// }

// TEST(inheritance_auth_wallet_tests, wallet_auth_get_signature_action) {
//   hex_string_to_byte_array(
//       "edf67877bfb47614e82fce98d16e588400af276b7e5032752189d747ff6d1efec",
//       WALLET_ID_SIZE * 2,
//       auth->wallet_id);
//   hex_string_to_byte_array(
//       "947fce34ee9faddce60e141fbcb667f7d0a1d0ae277a3604140f7facd078349e",
//       sizeof(ed25519_secret_key) * 2,
//       auth->private_key);
//   hex_string_to_byte_array(
//       "a0d13296a72d3d9dfce4adf908368864a33c6a39767aa040906fa728bf08109c",
//       sizeof(ed25519_public_key) * 2,
//       auth->public_key);
//   auth->challenge_size = 28;
//   memcpy(auth->challenge, "This is an example challenge",
//   auth->challenge_size);

//   wallet_auth_get_signature(auth);

//   uint8_t expected_signature[64] = {0};
//   hex_string_to_byte_array(
//       "b522e317d22a17177969383e204c6bef1762dde4218d823bac3a7c400f4f303690e103a4"
//       "b9160d15e3df0ea2aadf4dee6a7cf4bb3c9db3e69cd04cf14edb6c06",
//       128,
//       expected_signature);

//   TEST_ASSERT_EQUAL_UINT8_ARRAY(
//       expected_signature, auth->signature, sizeof(expected_signature));
// }
