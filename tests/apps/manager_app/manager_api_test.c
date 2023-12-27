/**
 * @file    manager_api_test.c
 * @author  Cypherock X1 Team
 * @brief   Defines helpers apis for manager app.
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
#include "manager_api.h"

#include <stdbool.h>
#include <string.h>

#include "unity_fixture.h"

TEST_GROUP(manager_api_test);

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(manager_api_test) {
}

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(manager_api_test) {
}

TEST(manager_api_test, decode_valid_manager_bs) {
  manager_query_t query_out = {0};
  uint8_t byte_stream[] = {34,  36,  18,  34,  10,  32,  6,   4,   37,  226,
                           45,  95,  166, 129, 63,  247, 201, 188, 151, 71,
                           113, 179, 160, 120, 135, 252, 54,  30,  198, 28,
                           152, 73,  180, 129, 55,  26,  238, 246};
  uint8_t challenge[] = {
      0x06, 0x04, 0x25, 0xe2, 0x2d, 0x5f, 0xa6, 0x81, 0x3f, 0xf7, 0xc9,
      0xbc, 0x97, 0x47, 0x71, 0xb3, 0xa0, 0x78, 0x87, 0xfc, 0x36, 0x1e,
      0xc6, 0x1c, 0x98, 0x49, 0xb4, 0x81, 0x37, 0x1a, 0xee, 0xf6,
  };
  TEST_ASSERT_TRUE(
      decode_manager_query(byte_stream, sizeof(byte_stream), &query_out));
  TEST_ASSERT_EQUAL_UINT16(MANAGER_QUERY_AUTH_CARD_TAG,
                           query_out.which_request);
  TEST_ASSERT_EQUAL_UINT16(MANAGER_AUTH_CARD_REQUEST_CHALLENGE_TAG,
                           query_out.auth_card.which_request);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(
      challenge, query_out.auth_card.challenge.challenge, 32);
}

TEST(manager_api_test, decode_invalid_manager_bs_incorrect_size) {
  manager_query_t query_out = {0};
  uint8_t byte_stream[] = {34,  29,  18,  27,  10,  25,  6,   4,   37, 226, 45,
                           95,  166, 129, 63,  247, 201, 188, 151, 71, 113, 179,
                           160, 120, 135, 252, 54,  30,  198, 28,  152};
  TEST_ASSERT_FALSE(
      decode_manager_query(byte_stream, sizeof(byte_stream), &query_out));
}

TEST(manager_api_test, encode_valid_manager_result) {
  manager_result_t result = {
      .which_response = MANAGER_RESULT_AUTH_CARD_TAG,
      .auth_card.which_response = MANAGER_AUTH_CARD_REQUEST_INITIATE_TAG,
      .auth_card.serial_signature.serial = {0x00, 0x00, 0x00, 0x01, 0x02},
      .auth_card.serial_signature.signature = {
          0x4c, 0xfe, 0x2f, 0x70, 0x43, 0x76, 0x90, 0xc0, 0x16, 0x14, 0x71,
          0xac, 0xab, 0xb1, 0x6c, 0x49, 0x3a, 0xe0, 0xb5, 0xc8, 0x65, 0x5f,
          0x1a, 0x3a, 0xbd, 0x4c, 0x9c, 0x57, 0x93, 0x54, 0xb7, 0x19, 0x09,
          0x8b, 0x9e, 0x33, 0xcf, 0xef, 0xca, 0x09, 0x6d, 0x5b, 0x02, 0xe4,
          0xdb, 0xca, 0xd5, 0xb5, 0xf0, 0xc1, 0xec, 0x3f, 0x38, 0xcb, 0xf7,
          0xcd, 0xbb, 0xc3, 0xb3, 0x86, 0x10, 0x33, 0x47, 0x72,
      }};
  uint8_t byte_stream[MANAGER_AUTH_CARD_RESPONSE_SIZE + 10];
  size_t bytes_written = 0;
  TEST_ASSERT_TRUE(encode_manager_result(
      &result, byte_stream, sizeof(byte_stream), &bytes_written));
  TEST_ASSERT_NOT_EQUAL_UINT32(0, bytes_written);
  TEST_ASSERT_LESS_THAN_UINT32(sizeof(byte_stream), bytes_written);
}

TEST(manager_api_test, encode_invalid_size_manager_result) {
  manager_result_t result = {
      .which_response = MANAGER_RESULT_AUTH_CARD_TAG,
      .auth_card.which_response = MANAGER_AUTH_CARD_REQUEST_INITIATE_TAG,
      .auth_card.serial_signature.serial = {0x00, 0x00, 0x00, 0x01, 0x02},
      .auth_card.serial_signature.signature = {
          0x4c, 0xfe, 0x2f, 0x70, 0x43, 0x76, 0x90, 0xc0, 0x16, 0x14, 0x71,
          0xac, 0xab, 0xb1, 0x6c, 0x49, 0x3a, 0xe0, 0xb5, 0xc8, 0x65, 0x5f,
          0x1a, 0x3a, 0xbd, 0x4c, 0x9c, 0x57, 0x93, 0x54, 0xb7, 0x19, 0x09,
          0x8b, 0x9e, 0x33, 0xcf, 0xef, 0xca, 0x09, 0x6d, 0x5b, 0x02, 0xe4,
          0xdb, 0xca, 0xd5, 0xb5, 0xf0, 0xc1, 0xec, 0x3f, 0x38, 0xcb, 0xf7,
          0xcd, 0xbb, 0xc3, 0xb3, 0x86, 0x10, 0x33, 0x47, 0x72,
      }};
  uint8_t byte_stream[10];
  size_t bytes_written = 0;
  TEST_ASSERT_FALSE(encode_manager_result(
      &result, byte_stream, sizeof(byte_stream), &bytes_written));
  TEST_ASSERT_EQUAL_UINT32(0, bytes_written);
}
