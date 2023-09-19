/**
 * @file    btc_helper_tests.c
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

#include "btc_app.h"
#include "btc_helpers.h"
#include "btc_priv.h"
#include "curves.h"
#include "flash_config.h"
#include "unity_fixture.h"
#include "utils.h"

TEST_GROUP(btc_helper_test);

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(btc_helper_test) {
  g_btc_app = get_btc_app_desc()->app_config;
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(btc_helper_test) {
  g_btc_app = NULL;
}

TEST(btc_helper_test, btc_helper_get_segwit_addr_even_y) {
  /* BIP32 Root Key :
   * zprvAWgYBBk7JR8Gj9xv7S1Lar8UXzJBeRxh6aMG2e2DQhAuryXz6PEjnGQue6oQ38WBH9mjbG8GvKtahcw9VPyQeyy5KTPfoVDwqgvw7epjGsi
   * PATH : m/84'/0'/0'/0/0
   */
  uint8_t pub_key[65];
  char addr[50] = "";
  const char expected[] = "bc1qv9mvxjhuwqmk389sfsvrty53z84gk2u5wwmdmt";

  // compressed
  hex_string_to_byte_array(
      "0272f5ad20087473a0548ada65536a2f7242cc366d45473f0604a921a9c9ce70aa",
      33 * 2,
      pub_key);

  TEST_ASSERT_EQUAL_UINT(1, btc_get_segwit_addr(pub_key, 33, "bc", addr));
  TEST_ASSERT_EQUAL_STRING(expected, addr);

  // ucompressed
  hex_string_to_byte_array(
      "0472f5ad20087473a0548ada65536a2f7242cc366d45473f0604a921a9c9ce70aa2da361"
      "5596dc20b0a88579a5b5f328a4cbc970247f0716151a8e9052b98f2010",
      65 * 2,
      pub_key);

  TEST_ASSERT_EQUAL_UINT(1, btc_get_segwit_addr(pub_key, 65, "bc", addr));
  TEST_ASSERT_EQUAL_STRING(expected, addr);
}

TEST(btc_helper_test, btc_helper_get_segwit_addr_odd_y) {
  /* BIP32 Root Key :
   * zprvAWgYBBk7JR8Gj9xv7S1Lar8UXzJBeRxh6aMG2e2DQhAuryXz6PEjnGQue6oQ38WBH9mjbG8GvKtahcw9VPyQeyy5KTPfoVDwqgvw7epjGsi
   * PATH : m/84'/0'/0'/0/2
   */
  uint8_t pub_key[65];
  char addr[50] = "";
  const char expected[] = "bc1q54l2zdaqsl4pzxmnm7j8f90hxugujkmzvt7d9l";

  // compressed
  hex_string_to_byte_array(
      "03d34ebbe1436b5b48d959b7644055687f80fac3fc5f261c76871a1870301617d9",
      33 * 2,
      pub_key);

  TEST_ASSERT_EQUAL_UINT(1, btc_get_segwit_addr(pub_key, 33, "bc", addr));
  TEST_ASSERT_EQUAL_STRING(expected, addr);

  // ucompressed
  hex_string_to_byte_array(
      "04d34ebbe1436b5b48d959b7644055687f80fac3fc5f261c76871a1870301617d977bac5"
      "787a51073f9e0d27ea2e3f65f8bb9b7b0d9f1dd641ebdc481e2ecd2185",
      65 * 2,
      pub_key);

  TEST_ASSERT_EQUAL_UINT(1, btc_get_segwit_addr(pub_key, 65, "bc", addr));
  TEST_ASSERT_EQUAL_STRING(expected, addr);
}

TEST(btc_helper_test, btc_helper_generate_xpub_nsegwit) {
  const char expected_xpub[] =
      "zpub6snp4m6XZJZdyvM2bfff1bRFswErPnd3JkyhKNRLasDmFMcMEneVmD6rPZPJasYC3rt8"
      "aSEQbtp1JsjXbSeCraecR9sqUk1FrZpXLRntMPW";
  uint32_t path[] = {
      0x80000000 | 84, /* m/84' */
      0x80000000 | 0,  /* /0'   */
      0x80000000 | 0,  /* /0'   */
      0x00000000 | 0,  /* /0   */
  };
  uint8_t seed[64];

  hex_string_to_byte_array(
      "0e37516bfd9429d749b5b83048a52cbbbc960850f47f6973845491272a1eb8b73ca2c722"
      "2f12aa7df09f851a9aeeae84aff9af17f7369e570b397ea8d550fb2c",
      128,
      seed);

  char xpub[XPUB_SIZE];

  bool result = btc_generate_xpub(path,
                                  sizeof(path) / sizeof(uint32_t),
                                  SECP256K1_NAME,
                                  seed,
                                  g_btc_app->nsegwit_xpub_ver,
                                  xpub);

  TEST_ASSERT_EQUAL_UINT(1, result);
  TEST_ASSERT_EQUAL_STRING(expected_xpub, xpub);
}

TEST(btc_helper_test, btc_helper_generate_xpub_segwit) {
  const char expected_xpub[] =
      "ypub6aBfyV8C1Jfrxf884pGY1Se1Eza1mvkB8XkuTwYKSQeCc621ZKsrg2ENLazFr3WkpDcr"
      "MdUu3f6SPnGqLWMFbsdo8pcmcmNQqqmyVdVcrSs";
  uint32_t path[] = {
      0x80000000 | 49, /* m/49' */
      0x80000000 | 0,  /* /0'   */
      0x80000000 | 0,  /* /0'   */
      0x00000000 | 0,  /* /0    */
  };
  uint8_t seed[64];

  hex_string_to_byte_array(
      "0e37516bfd9429d749b5b83048a52cbbbc960850f47f6973845491272a1eb8b73ca2c722"
      "2f12aa7df09f851a9aeeae84aff9af17f7369e570b397ea8d550fb2c",
      128,
      seed);

  char xpub[XPUB_SIZE];

  bool result = btc_generate_xpub(path,
                                  sizeof(path) / sizeof(uint32_t),
                                  SECP256K1_NAME,
                                  seed,
                                  g_btc_app->segwit_xpub_ver,
                                  xpub);

  TEST_ASSERT_EQUAL_UINT(1, result);
  TEST_ASSERT_EQUAL_STRING(expected_xpub, xpub);
}

TEST(btc_helper_test, btc_helper_generate_xpub_legacy) {
  const char expected_xpub[] =
      "xpub6E6ASSVjmAchK4RhoG6EaqZ9e3igYMpukcNxRk5mcB4L94QbfFrcxL8aur4YY9BiwzMt"
      "6oYbtMxg2SXptA3ubyKnnBvrQEEpPZut3F69Gvf";
  uint32_t path[] = {
      0x80000000 | 44, /* m/44' */
      0x80000000 | 0,  /* /0'   */
      0x80000000 | 0,  /* /0'   */
      0x00000000 | 0,  /* /0    */
  };
  uint8_t seed[64];

  hex_string_to_byte_array(
      "0e37516bfd9429d749b5b83048a52cbbbc960850f47f6973845491272a1eb8b73ca2c722"
      "2f12aa7df09f851a9aeeae84aff9af17f7369e570b397ea8d550fb2c",
      128,
      seed);

  char xpub[XPUB_SIZE];

  bool result = btc_generate_xpub(path,
                                  sizeof(path) / sizeof(uint32_t),
                                  SECP256K1_NAME,
                                  seed,
                                  g_btc_app->legacy_xpub_ver,
                                  xpub);

  TEST_ASSERT_EQUAL_UINT(1, result);
  TEST_ASSERT_EQUAL_STRING(expected_xpub, xpub);
}
