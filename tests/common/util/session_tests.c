RUN_TEST_CASE(inheritance_session_test, session_initiate_action);

/**
 * @file    session_tests.c
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

#include "session_utils.h"
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
TEST_GROUP(session_tests);

TEST_SETUP(session_tests) {
  // array_list.array = &array_buffer[0];
  // array_list.current_index = 0;
  // array_list.num_of_elements = 0;
  // array_list.max_capacity = sizeof(array_buffer) / sizeof(array_buffer[0]);
  // array_list.size_of_element = sizeof(array_buffer[0]);
  // memset(&array_buffer[0], 0, sizeof(array_buffer));
}

TEST_TEAR_DOWN(session_tests) {
  // memset(&array_buffer[0], 0, sizeof(array_buffer));
}

// Test insertion
TEST(session_tests, session_pre_initiate_action) {
  // Send: Device Random (32) + Device Id (32) + Signature (64) + Postfix1 +
  // Postfix2
  uint8_t session_details_data_array[DEVICE_RANDOM_SIZE + DEVICE_SERIAL_SIZE +
                                     SIGNATURE_SIZE + POSTFIX1_SIZE +
                                     POSTFIX2_SIZE];

  session_pre_init(session_details_data_array);

  char hex[200];
  byte_array_to_hex_string(session_details_data_array,
                           DEVICE_RANDOM_SIZE + DEVICE_SERIAL_SIZE +
                               SIGNATURE_SIZE + POSTFIX1_SIZE + POSTFIX2_SIZE,
                           hex,
                           (DEVICE_RANDOM_SIZE + DEVICE_SERIAL_SIZE +
                            SIGNATURE_SIZE + POSTFIX1_SIZE + POSTFIX2_SIZE) *
                                   2 +
                               1);
  printf("session_details_data_array : %s", hex);
}

TEST(session_tests, session_initiate_action) {
  uint8_t device_random[DEVICE_RANDOM_SIZE];
  random_generate(device_random, DEVICE_RANDOM_SIZE);
  uint32_t session_age = 1234;

  uint8_t data_array[DEVICE_RANDOM_SIZE + sizeof(session_age)];
  memcpy(data_array, device_random, DEVICE_SERIAL_SIZE);
  memcpy(data_array + DEVICE_RANDOM_SIZE, &session_age, sizeof(session_age));

  char hex[200];
  byte_array_to_hex_string(data_array,
                           DEVICE_RANDOM_SIZE + sizeof(session_age),
                           hex,
                           (DEVICE_RANDOM_SIZE + sizeof(session_age)) * 2 + 1);
  printf("data_array : %s", hex);

  // Send: Device Id (32) + Signature (64) + Postfix1 + Postfix2
  uint8_t verification_details[DEVICE_SERIAL_SIZE + SIGNATURE_SIZE +
                               POSTFIX1_SIZE + POSTFIX2_SIZE];

  if (!session_init(data_array, verification_details)) {
    LOG_CRITICAL("xxec %d", __LINE__);
    comm_reject_invalid_cmd();
    clear_message_received_data();
  }

  memzero(hex, 200);
  byte_array_to_hex_string(
      verification_details,
      DEVICE_SERIAL_SIZE + SIGNATURE_SIZE + POSTFIX1_SIZE + POSTFIX2_SIZE,
      hex,
      (DEVICE_SERIAL_SIZE + SIGNATURE_SIZE + POSTFIX1_SIZE + POSTFIX2_SIZE) *
              2 +
          1);
  printf("verification_details : %s", hex);
}