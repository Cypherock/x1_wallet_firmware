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

void test_uint32_to_uint8_array(uint32_t value, uint8_t arr[4]);
void test_generate_server_data(uint8_t *server_message);

TEST(session_tests, session_send_device_key_action) {
  uint8_t payload_size = PUBLIC_KEY_SIZE + DEVICE_SERIAL_SIZE + SIGNATURE_SIZE +
                         POSTFIX1_SIZE + POSTFIX2_SIZE;
  uint8_t payload[payload_size];
  char hex[payload_size * 2 + 1];

  session_send_device_key(session, payload);

  byte_array_to_hex_string(payload, payload_size, hex, sizeof(hex));
  printf("\nsession_send_device_key_action : %s", hex);
}

TEST(session_tests, session_receive_server_key_action) {
  uint8_t payload_size =
      DEVICE_SERIAL_SIZE + SIGNATURE_SIZE + POSTFIX1_SIZE + POSTFIX2_SIZE;
  uint8_t payload[payload_size];
  char hex[payload_size * 2 + 1];

  // Generate Server_Message
  uint8_t *server_message;
  test_generate_server_data(server_message);

  // On Server Request: Get Server_Random_Public from Server [payload: Device Id
  // (32) + Signature (64) + Postfix1 + Postfix2]
  payload_size =
      DEVICE_SERIAL_SIZE + SIGNATURE_SIZE + POSTFIX1_SIZE + POSTFIX2_SIZE;

  if (!session_receive_server_key(session, server_message)) {
    LOG_CRITICAL("xxec %d", __LINE__);
    comm_reject_invalid_cmd();
    clear_message_received_data();
  }

  byte_array_to_hex_string(payload, payload_size, hex, sizeof(hex));
  printf("session_receive_server_key_action : %s", hex);

  // uint8_t device_random[PUBLIC_KEY_SIZE];
  // random_generate(device_random, PUBLIC_KEY_SIZE);
  // memzero(hex, 200);

  // char hex[200]="";
  // memzero(hex, 200);
  // byte_array_to_hex_string(
  //     device_random,
  //     PUBLIC_KEY_SIZE,
  //     hex,
  //     65);
  // printf("\nverification_details : %s\n", hex);
}

void test_uint32_to_uint8_array(uint32_t value, uint8_t arr[4]) {
  arr[0] = (value >> 24) & 0xFF;    // Extract the highest byte
  arr[1] = (value >> 16) & 0xFF;    // Extract the second highest byte
  arr[2] = (value >> 8) & 0xFF;     // Extract the second lowest byte
  arr[3] = value & 0xFF;            // Extract the lowest byte
}

void test_generate_server_data(uint8_t *server_message) {
  uint8_t server_random[PRIVATE_KEY_SIZE];
  uint8_t server_random_public[PUBLIC_KEY_SIZE];
  curve_point server_random_public_point = {0};
  session_get_random_keys(
      server_random, server_random_public, server_random_public_point);

  uint32_t session_age_int = 1234;
  uint8_t session_age[4];
  test_uint32_to_uint8_array(session_age_int, session_age);

  uint8_t offset = 0;
  memcpy(server_message, server_random_public, PUBLIC_KEY_SIZE);
  offset += PUBLIC_KEY_SIZE;
  memcpy(server_message + offset, session_age, SESSION_AGE_SIZE);
  offset += SESSION_AGE_SIZE;
  memcpy(server_message + offset, atecc_data.device_serial, DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;
}

void session_initiation() {
  uint8_t payload_size;
  uint8_t *payload;
  char hex[500];

  // On Cysync Request: Send Device_Random to server [Device Random (32) +
  // Device Id (32) + Signature (64) + Postfix1 + Postfix2]
  payload_size = PUBLIC_KEY_SIZE + DEVICE_SERIAL_SIZE + SIGNATURE_SIZE +
                 POSTFIX1_SIZE + POSTFIX2_SIZE;

  session_send_device_key(session, payload);

  byte_array_to_hex_string(&payload, payload_size, hex, payload_size * 2 + 1);
  printf("session_details_data_array : %s", hex);

  // Generate Server_Message
  uint8_t *server_message;
  USE_SIMULATOR
  test_generate_server_data(server_message);

  // On Server Request: Get Server_Random_Public from Server [payload: Device Id
  // (32) + Signature (64) + Postfix1 + Postfix2]
  payload_size =
      DEVICE_SERIAL_SIZE + SIGNATURE_SIZE + POSTFIX1_SIZE + POSTFIX2_SIZE;
  payload = (uint8_t *)malloc(payload_size * sizeof(uint8_t));

  if (!session_receive_server_key(session, server_message)) {
    LOG_CRITICAL("xxec %d", __LINE__);
    comm_reject_invalid_cmd();
    clear_message_received_data();
  }

  memzero(hex, 500);
  byte_array_to_hex_string(payload, payload_size, hex, payload_size * 2 + 1);
  printf("verification_details : %s", hex);

  // derive_session_id();
  // derive_session_key();
}