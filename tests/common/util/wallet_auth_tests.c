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
static wallet_auth_t auth = {0};
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
  memset(&auth[0], 0, sizeof(auth));
}

TEST(wallet_auth_tests, wallet_auth_get_pairs) { 
  hex_string_to_byte_array("",auth.entropy);

  get_pairs(auth);

  char hex[64];
  byte_array_to_hex_string(auth.public_key, sizeof(ed25519_public_key), hex, sizeof(hex));
  printf("\nwallet key auth public_key: %s", hex);

  // uint8_t expected_public_key[32] = {0};
  // hex_string_to_byte_array(
  //   "",
  //   64,
  //   expected_public_key);

  // uint8_t public_key[64] = {0};
  // TEST_ASSERT_EQUAL_UINT8_ARRAY(
  //     expected_public_key, auth.public_key, sizeof(expected_public_key));
}

TEST(wallet_auth_tests, wallet_auth_get_signature) {
  auth.challenge_size = 32;
  hex_string_to_byte_array("This is an example challenge", auth.challenge*2+1, auth.challenge);
  hex_string_to_byte_array("", WALLET_ID_SIZE, auth.wallet_id);
  hex_string_to_byte_array("9fa1ab1d37025d8c3cd596ecbf50435572eeaeb1785a0c9ed2b22afa4c378d6a", 32, auth.private_key);
  hex_string_to_byte_array("10b22ebe89b321370bee8d39d5c5d411daf1e8fc91c9d1534044590f1f966ebc", 32, auth.public_key);
  
  get_signature(auth);

  char hex[128];
  byte_array_to_hex_string(auth.signature, sizeof(ed25519_signature), hex, sizeof(hex));
  printf("\nwallet key auth signature: %s", hex);

  // uint8_t expected_signature[64] = {0};
  // hex_string_to_byte_array(
  //   "",
  //   128,
  //   expected_signature);

  // TEST_ASSERT_EQUAL_UINT8_ARRAY(
  //     expected_signature, auth.signature, sizeof(expected_signature));
}