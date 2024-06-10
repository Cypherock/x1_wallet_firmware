/**
 * @file    session_utils.c
 * @author  Cypherock X1 Team
 * @brief   Definition of the session utility functions
 *          This file defines the functions used to create and manage the
 *          session, send authentication requests and verify the responses.
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
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

#include "session_utils.h"

const uint32_t session_key_derv_data[2] = {6, 7};
Session session;

static void derive_public_key() {
  HDNode node;
  uint32_t index;
  char xpub[112] = {'\0'};

  base58_encode_check(get_card_root_xpub(),
                      FS_KEYSTORE_XPUB_LEN,
                      nist256p1_info.hasher_base58,
                      xpub,
                      112);
  hdnode_deserialize_public(
      (char *)xpub, 0x0488b21e, NIST256P1_NAME, &node, NULL);

  index = 0;
  hdnode_public_ckd(&node, session_key_derv_data[index]);

  index += 1;
  hdnode_public_ckd(&node, session_key_derv_data[index]);

  memcpy(session.server_random_public, node.public_key, SERVER_RANDOM_SIZE);
}

static void derive_session_id() {
  uint8_t payload[SERVER_RANDOM_SIZE + DEVICE_RANDOM_SIZE];
  memcpy(payload, session.session_random, SERVER_RANDOM_SIZE);
  memcpy(
      payload + SERVER_RANDOM_SIZE, session.device_random, DEVICE_RANDOM_SIZE);

  sha256_Raw(payload, sizeof(payload), session.session_id);
}

bool verify_session_signature(uint8_t *payload,
                              uint8_t payload_length,
                              uint8_t *buffer) {
  uint8_t hash[32] = {0};
  sha256_Raw(payload, payload_length, hash);

  uint8_t status =
      ecdsa_verify_digest(&nist256p1, session.public_key, buffer, hash);

  return (status == 0);
};

void session_append_signature(uint8_t *payload,
                              uint8_t payload_length,
                              uint8_t *signature_details) {
  uint8_t hash[32] = {0};
  sha256_Raw(payload, payload_length, hash);
  auth_data_t signed_data = atecc_sign(hash);

  uint8_t offset = 0;
  memcpy(signature_details, signed_data.signature, SIGNATURE_SIZE);
  offset += SIGNATURE_SIZE;
  memcpy(signature_details + offset, signed_data.postfix1, POSTFIX1_SIZE);
  offset += POSTFIX1_SIZE;
  memcpy(signature_details + offset, signed_data.postfix2, POSTFIX2_SIZE);
  offset += POSTFIX2_SIZE;
}

void session_send_device_key(Session *session, uint8_t *payload) {
  // Output Payload: Device Random (32) + Device Id (32) + Signature (64) +
  // Postfix1 + Postfix2
  uint8_t session_details_data_array_size = 0;

  // Generate device randoms
  random_generate(session.device_random, DEVICE_RANDOM_SIZE);
  derive_public_key();

  // Get device_id
  get_device_serial();

  // Construct outout payload
  uint8_t offset = 0;
  memcpy(session_details_data_array, session.device_random, DEVICE_RANDOM_SIZE);
  offset += DEVICE_RANDOM_SIZE;
  memcpy(session.device_id, atecc_data.device_serial, DEVICE_SERIAL_SIZE);
  memcpy(session_details_data_array + offset,
         session.device_id,
         DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;
  session_append_signature(
      session_details_data_array, offset, session_details_data_array + offset);
}

bool session_get_server_key(Session *session, uint8_t *payload) {
  // Input  payload (session_init_details): Session Random + Session Age
  // Output Payload: Session Random + Session Age + Device Id + Device Random
  uint8_t offset = 0;
  memcpy(session.session_random, session_init_details, SERVER_RANDOM_SIZE);
  offset += SESSION_RANDOM_SIZE;

  memcpy(&session.session_age,
         session_init_details + offset,
         sizeof(session.session_age));
  offset += sizeof(session.session_age);

  uint8_t payload[SERVER_RANDOM_SIZE + sizeof(session.session_age) +
                  DEVICE_SERIAL_SIZE + DEVICE_RANDOM_SIZE];

  size_t offset = 0;
  memcpy(payload, session_init_details, offset);
  payload_length += offset;
  memcpy(payload + payload_length, session.device_id, DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;
  memcpy(payload + payload_length, session.device_random, DEVICE_RANDOM_SIZE);
  offset += DEVICE_RANDOM_SIZE;

  if (!verify_session_signature(
          payload, payload_length, session_init_details + offset)) {
    return false;
  }

  // Verification details: Device Id + Signature + Postfix1 + Postfix2
  memcpy(verification_details, session.device_id, DEVICE_SERIAL_SIZE);
  offset = DEVICE_SERIAL_SIZE;
  session_append_signature(
      payload, payload_length, verification_details + offset);

  return true;
}

void test_uint32_to_uint8_array(uint32_t value, uint8_t arr[4]) {
  arr[0] = (value >> 24) & 0xFF;    // Extract the highest byte
  arr[1] = (value >> 16) & 0xFF;    // Extract the second highest byte
  arr[2] = (value >> 8) & 0xFF;     // Extract the second lowest byte
  arr[3] = value & 0xFF;            // Extract the lowest byte
}

void test_generate_server_data(uint8_t *server_message) {
  uint8_t device_random[SERVER_RANDOM_SIZE];
  random_generate(server_random, SERVER_RANDOM_SIZE);

  uint32_t session_age_value = 1234;
  uint8_t session_age[4];
  test_uint32_to_uint8_array(session_age_value, session_age);

  uint8_t offset = 0;
  memcpy(server_message, device_random, SERVER_RANDOM_SIZE);
  offset += SERVER_RANDOM_SIZE;
  memcpy(server_message + offset, session_age, SESSION_AGE_SIZE);
}

void session_initiation() {
  // On Cysync Request: Send Device_Random to server [Device Random (32) +
  // Device Id (32) + Signature (64) + Postfix1 + Postfix2]
  uint8_t payload_size = DEVICE_RANDOM_SIZE + DEVICE_SERIAL_SIZE +
                         SIGNATURE_SIZE + POSTFIX1_SIZE + POSTFIX2_SIZE;
  uint8_t session_details_data_array[payload_size];

  session_send_device_key(session_details_data_array);

  char hex[200];
  byte_array_to_hex_string(
      session_details_data_array, payload_size, hex, payload_size * 2 + 1);
  printf("session_details_data_array : %s", hex);

  // Generate Server_Message
  uint8_t *server_message;
  test_generate_server_message(server_message);

  // On Server Request: Get Server_Random_Public from Server [payload: Device Id
  // (32) + Signature (64) + Postfix1 + Postfix2]
  payload_size =
      DEVICE_SERIAL_SIZE + SIGNATURE_SIZE + POSTFIX1_SIZE + POSTFIX2_SIZE;
  uint8_t verification_details[payload_size];

  if (!session_server_response(server_message, verification_details)) {
    LOG_CRITICAL("xxec %d", __LINE__);
    comm_reject_invalid_cmd();
    clear_message_received_data();
  }

  memzero(hex, 200);
  byte_array_to_hex_string(
      verification_details, payload_size, hex, payload_size * 2 + 1);
  printf("verification_details : %s", hex);

  //
}