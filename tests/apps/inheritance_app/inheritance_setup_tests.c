/**
 * @file    inheritance_script_tests.c
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

#include "inheritance_api.h"
#include "inheritance_main.h"
#include "unity_fixture.h"

TEST_GROUP(inheritance_setup_test);

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(inheritance_setup_test) {
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(inheritance_setup_test) {
}

TEST(inheritance_setup_test, inheritance_setup_test_first) {
  inheritance_query_t query = INHERITANCE_QUERY_INIT_ZERO;
  query.which_request = INHERITANCE_QUERY_SETUP_TAG;
  uint8_t message_1[300] = {0};
  // Message #1
  hex_string_to_byte_array(
      "010000000100000000000000000000000000000000000000000000000000000000000000"
      "00ffffffff0704ffff001d0134ffffffff0100f2052a0100000043410411db93e1dcdb8a"
      "016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5cb2e0eaddfb84ccf9744464"
      "f82e160bfa9b8b64f9d4c03f999b8643f656b412a3ac00000000",
      268,
      message_1);
  memcpy(query.setup.plain_data[0].message.bytes, message_1, 134);
  query.setup.plain_data[0].message.size = 134;
  query.setup.plain_data_count = 1;

  // // Message #2
  // uint8_t message_2[200] = {0};
  // hex_string_to_byte_array(
  //   "3FA46B1EC2589D7346BD8A1F4C79DE2AB35C6E7109FA23B8C4E65F9D2A7B4F3C1D5E8A912FA47CB26E195D3C4B6A8F2D7E4B9C17F2A4E9B5C78D3F1A6B8E5C2D4A1F",
  //   132,
  //   message_2);
  // memcpy(query.setup.plain_data[1].message.bytes, message_2, 66);
  // query.setup.plain_data[1].message.size = 66;
  // query.setup.plain_data_count = 2;
  inheritance_setup(&query);

  inheritance_query_t query_r = INHERITANCE_QUERY_INIT_ZERO;
  uint8_t encrpyted_result[600];
  hex_string_to_byte_array(
      "7d7447ef906212bc4f6211c0378659968a835492790be0726bc45fed332390bb13d96c87"
      "b202b97bc16f4d27c91804d0632bd4976de0e0e8ea0537fb6b621de63163ae9823d5c572"
      "1ec35ce53c55079af1363a88da8c8275062b76ee338019d62f1baa8959a9e6d2a4da615e"
      "8ef53a641e2e811532940a5a7a0f9cf48ce1470a9806bcdb8a7734d44ee48cc81c02bba"
      "4",
      // "7d7447ef906212bc4f6211c0378659968a835492790be0726bc45fed332390bb13d96c87b202b97bc16f4d27c91804d0632bd4976de0e0e8ea0537fb6b621de63163ae9823d5c5721ec35ce53c55079af1363a88da8c8275062b76ee338019d62f1baa8959a9e6d2a4da615e8ef53a641e2e811532940a5a7a0f9cf48ce1470aea4f8513d647b3e56bea00a2f69504c36b2c95af67e24611e7b28e0ef11c16637a1417ef8f0d6ca849cf18252ec0abaeec20d6f6e4323f286f892ad4f7a3e394e9ad327205fcaade72850b6800a12d7ab88490fc4736f2c357f1c60d193c737d",
      288,
      encrpyted_result);
  query_r.recovery.has_encrypted_data = true;
  query_r.which_request = INHERITANCE_QUERY_RECOVERY_TAG;
  memcpy(query_r.recovery.encrypted_data.packet.bytes, encrpyted_result, 144);
  query_r.recovery.encrypted_data.packet.size = 144;

  inheritance_recovery(&query_r);

  TEST_ASSERT_EQUAL_INT(1, 1);
}