/**
 * @file    wallet_auth_tests.c
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

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <string.h>

#include "wallet_auth_utils.h"
#include "unity_fixture.h"

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
TEST_GROUP(wallet_auth_tests);

TEST_SETUP(wallet_auth_tests) {
  return;
}

TEST_TEAR_DOWN(wallet_auth_tests) {
  memset(&auth, 0, sizeof(auth));
}

TEST(wallet_auth_tests, wallet_auth_get_entropy_action) { 
  hex_string_to_byte_array("edf67877bfb47614e82fce98d16e588400af276b7e5032752189d747ff6d1efec", WALLET_ID_SIZE*2, auth.wallet_id);

  wallet_auth_get_entropy();

  uint8_t expected_entropy[ENTROPY_SIZE_LIMIT] = {0};
  hex_string_to_byte_array(
    "2146edf67877bfb47614e82fce98d16e588400af276b7e5032752189d747ff6d1efe",
    68,
    expected_entropy);

  uint8_t public_key[64] = {0};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      expected_entropy, auth.entropy, sizeof(expected_entropy));
}

TEST(wallet_auth_tests, wallet_auth_get_pairs_action) { 
  auth.entropy_size = 68;
  hex_string_to_byte_array("2146edf67877bfb47614e82fce98d16e588400af276b7e5032752189d747ff6d1efe", auth.entropy_size, auth.entropy);

  wallet_auth_get_pairs();

  uint8_t expected_public_key[32] = {0};
  hex_string_to_byte_array(
    "a0d13296a72d3d9dfce4adf908368864a33c6a39767aa040906fa728bf08109c",
    64,
    expected_public_key);

  uint8_t public_key[64] = {0};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      expected_public_key, auth.public_key, sizeof(expected_public_key));
}

TEST(wallet_auth_tests, wallet_auth_get_signature_action) {
  hex_string_to_byte_array("edf67877bfb47614e82fce98d16e588400af276b7e5032752189d747ff6d1efec", WALLET_ID_SIZE*2, auth.wallet_id);
  hex_string_to_byte_array("947fce34ee9faddce60e141fbcb667f7d0a1d0ae277a3604140f7facd078349e", sizeof(ed25519_secret_key)*2, auth.private_key);
  hex_string_to_byte_array("a0d13296a72d3d9dfce4adf908368864a33c6a39767aa040906fa728bf08109c", sizeof(ed25519_public_key)*2, auth.public_key);
  auth.challenge_size = 28;
  memcpy(auth.challenge, "This is an example challenge", auth.challenge_size);
  
  wallet_auth_get_signature();

  uint8_t expected_signature[64] = {0};
  hex_string_to_byte_array(
    "b522e317d22a17177969383e204c6bef1762dde4218d823bac3a7c400f4f303690e103a4b9160d15e3df0ea2aadf4dee6a7cf4bb3c9db3e69cd04cf14edb6c06",
    128,
    expected_signature);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      expected_signature, auth.signature, sizeof(expected_signature));
}