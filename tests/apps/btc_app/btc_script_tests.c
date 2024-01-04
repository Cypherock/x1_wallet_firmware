/**
 * @file    btc_script_tests.c
 * @author  Cypherock X1 Team
 * @brief   Unit tests for Bitcoin scripts
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

#include "btc_helpers.h"
#include "btc_priv.h"
#include "curves.h"
#include "flash_config.h"
#include "ltc_app.h"
#include "unity_fixture.h"
#include "utils.h"

TEST_GROUP(btc_script_test);

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(btc_script_test) {
  // g_btc_app = get_btc_app_desc()->app_config;
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(btc_script_test) {
  g_btc_app = NULL;
}

TEST(btc_script_test, btc_script_ltc_nsegwit_address) {
  const char *expected_address = "ltc1qk373tk0kmmss5gadrjhgyvz5tss8luf9g4zua2";
  const char actual_address[110] = "";
  uint8_t script_pub[110] = {0};

  g_btc_app = get_ltc_app_desc()->app_config;

  hex_string_to_byte_array(
      "0014b47d15d9f6dee10a23ad1cae8230545c207ff125", 44, script_pub);
  int addr_len = btc_get_script_pub_address(
      script_pub, 22, actual_address, sizeof(actual_address));
  TEST_ASSERT_GREATER_THAN_INT32_MESSAGE(
      0, addr_len, "Address encoding failure");
  TEST_ASSERT_EQUAL_STRING(expected_address, actual_address);
}

TEST(btc_script_test, btc_script_ltc_legacy_address) {
  const char *expected_address = "LhVoABz2VSdqKHgRuySGuh5oebebFocZBm";
  const char actual_address[110] = "";
  uint8_t script_pub[110] = {0};

  g_btc_app = get_ltc_app_desc()->app_config;

  hex_string_to_byte_array(
      "76a914f451d2b87717395e1cca0586820f5daa078f7c0288ac", 50, script_pub);
  int addr_len = btc_get_script_pub_address(
      script_pub, 25, actual_address, sizeof(actual_address));
  TEST_ASSERT_GREATER_THAN_INT32_MESSAGE(
      0, addr_len, "Address encoding failure");
  TEST_ASSERT_EQUAL_STRING(expected_address, actual_address);
}

TEST(btc_script_test, btc_script_ltc_p2sh_address1) {
  const char *expected_address = "MPWqFgbPvEvLTAvia2fYntkcEFXGepARFt";
  const char actual_address[110] = "";
  uint8_t script_pub[110] = {0};

  g_btc_app = get_ltc_app_desc()->app_config;

  hex_string_to_byte_array(
      "a914ab4904de514195e0604143955396462dbb06c22287", 46, script_pub);
  int addr_len = btc_get_script_pub_address(
      script_pub, 23, actual_address, sizeof(actual_address));
  TEST_ASSERT_GREATER_THAN_INT32_MESSAGE(
      0, addr_len, "Address encoding failure");
  TEST_ASSERT_EQUAL_STRING(expected_address, actual_address);
}
