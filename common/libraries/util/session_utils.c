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

const ecdsa_curve *curve;
const uint32_t session_key_rotation[2] = {6, 7};
Session session;

void session_curve_init() {
  curve = get_curve_by_name(SECP256K1_NAME)->params;
}

static bool derive_server_public_key() {
  HDNode node;
  char xpub[112] = {'\0'};

  base58_encode_check(get_card_root_xpub(),
                      FS_KEYSTORE_XPUB_LEN,
                      nist256p1_info.hasher_base58,
                      xpub,
                      112);
  hdnode_deserialize_public(
      (char *)xpub, 0x0488b21e, NIST256P1_NAME, &node, NULL);

  uint8_t index = 0;
  hdnode_public_ckd(&node, session_key_rotation[index]);

  index += 1;
  hdnode_public_ckd(&node, session_key_rotation[index]);

  memcpy(session.derived_server_public_key, node.public_key, PUBLIC_KEY_SIZE);

  return true;
}

bool verify_session_signature(uint8_t *payload,
                              uint8_t payload_size,
                              uint8_t *signature) {
  uint8_t hash[32] = {0};
  sha256_Raw(payload, payload_size, hash);

  // TODO: uncomment while integrating the sdk
  // uint8_t status = ecdsa_verify_digest(
  // &nist256p1, session.derived_server_public_key, signature, hash);
  uint8_t status = ecdsa_verify_digest(
      &nist256p1, session.server_random_public, signature, hash);
  return (status == 0);
};

bool derive_session_id() {
  curve_point random_point;
  memcpy(
      &random_point, &session.server_random_public_point, sizeof(curve_point));
  point_add(curve, &session.device_random_public_point, &random_point);

  printf("\nsession id random_point: ");
  bn_print(&random_point.x);
  bn_print(&random_point.y);

  uint8_t temp[2 * PRIVATE_KEY_SIZE];
  bn_write_be(&random_point.x, temp);
  bn_write_be(&random_point.y, temp + PRIVATE_KEY_SIZE);

  print_arr("session id temp", temp, 2 * PRIVATE_KEY_SIZE);

  sha256_Raw(temp, 2 * PUBLIC_KEY_SIZE, session.session_id);

  print_arr("session.session_id", session.session_id, SESSION_ID_SIZE);

  return true;
}

bool derive_session_key() {
  if (ecdh_multiply(curve,
                    session.device_random,
                    session.server_random_public,
                    session.session_key) != 0) {
    printf("ERROR: Session key not generated");
    return false;
  }
  print_arr("session.session_key", session.session_key, SESSION_KEY_SIZE);
  return true;
}

void session_append_signature(uint8_t *payload, size_t payload_size) {
  uint8_t hash[32] = {0};
  sha256_Raw(payload, payload_size, hash);
  auth_data_t signed_data = atecc_sign(hash);

  uint8_t offset = payload_size;
  memcpy(payload + offset, signed_data.signature, SIGNATURE_SIZE);
  offset += SIGNATURE_SIZE;
  print_arr("sig detail", signed_data.signature, SIGNATURE_SIZE);
  memcpy(payload + offset, signed_data.postfix1, POSTFIX1_SIZE);
  offset += POSTFIX1_SIZE;
  print_arr("sig detail", signed_data.postfix1, POSTFIX1_SIZE);
  memcpy(payload + offset, signed_data.postfix2, POSTFIX2_SIZE);
  offset += POSTFIX2_SIZE;
  print_arr("sig detail", signed_data.postfix2, POSTFIX2_SIZE);
}

bool session_get_random_keys(uint8_t *random,
                             uint8_t *random_public,
                             curve_point random_public_point) {
  memzero(random, PRIVATE_KEY_SIZE);
  memzero(random_public, PUBLIC_KEY_SIZE);
  memset(&random_public_point, 0, sizeof(random_public_point));

#if USE_SIMULATOR == 0
  random_generate(random, PRIVATE_KEY_SIZE);
#else
  uint8_t get_ec_random[32] = {0x0b, 0x78, 0x9a, 0x1e, 0xb8, 0x0b, 0x7a, 0xac,
                               0x97, 0xa1, 0x54, 0xd7, 0x0c, 0x5a, 0x53, 0x95,
                               0x6f, 0x9c, 0xed, 0x97, 0x6f, 0xc7, 0xed, 0x7f,
                               0xf9, 0x10, 0x01, 0xc1, 0xa8, 0x30, 0xde, 0xb1};
  memcpy(random, get_ec_random, PRIVATE_KEY_SIZE);
#endif
  ecdsa_get_public_key33(curve, random, random_public);
  print_arr("private key", random, PRIVATE_KEY_SIZE);
  print_arr("public key", random_public, PUBLIC_KEY_SIZE);

  if (!ecdsa_read_pubkey(curve, random_public, &random_public_point)) {
    printf("\nERROR: Random public key point not read");
    return false;
  }

  bn_print(&random_public_point.x);
  bn_print(&random_public_point.y);

  return true;
}

bool session_send_device_key(uint8_t *payload) {
  // Output Payload:
  // Device Random(32)+Device Id(32)+Signature(64)+Postfix1(7)+Postfix2(23)

  if (!session_get_random_keys(session.device_random,
                               session.device_random_public,
                               session.device_random_public_point)) {
    printf("\nERROR: Device Random keys not genrated");
    return false;
  }

  // Get device_id
#if USE_SIMULATOR == 0
  get_device_serial();
  memcpy(session.device_id, atecc_data.device_serial, DEVICE_SERIAL_SIZE);
#else
  memcpy(session.device_id, session.device_random, DEVICE_SERIAL_SIZE);
#endif

  uint8_t offset = 0;
  memcpy(payload + offset, session.device_random_public, PUBLIC_KEY_SIZE);
  offset += PUBLIC_KEY_SIZE;
  memcpy(payload + offset, session.device_id, DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;
  session_append_signature(payload, offset);
  print_arr("payload after signature", payload, offset + 64 + 7 + 23);

  return true;
}

bool session_receive_server_key(uint8_t *server_message) {
  // Input Payload: Server_Random_public + Session Age + Device Id
  if (!derive_server_public_key()) {
    printf("\nERROR: Server Randoms not read");
    return false;
  }

  size_t server_message_size =
      PUBLIC_KEY_SIZE + SESSION_AGE_SIZE + DEVICE_SERIAL_SIZE;
#if USE_SIMULATOR == 0
  if (!verify_session_signature(server_message,
                                server_message_size,
                                server_message + server_message_size)) {
    printf("\nERROR: Message from server not valid");
    return false;
  }
#endif

  uint8_t offset = 0;
  memcpy(
      session.server_random_public, server_message + offset, PUBLIC_KEY_SIZE);
  offset += PUBLIC_KEY_SIZE;
  memcpy(session.session_age, server_message + offset, SESSION_AGE_SIZE);
  offset += SESSION_AGE_SIZE;

  if (memcmp(session.device_id, server_message + offset, DEVICE_SERIAL_SIZE) !=
      0) {
#if USE_SIMULATOR == 0
    print_arr("session.device_id", session.device_id, DEVICE_SERIAL_SIZE);
    print_arr(
        "server_message + offset", server_message + offset, DEVICE_SERIAL_SIZE);
    return false;
#endif
  }
  offset += DEVICE_SERIAL_SIZE;

  if (!ecdsa_read_pubkey(curve,
                         session.server_random_public,
                         &session.server_random_public_point)) {
    printf("\nERROR: Server random public key point not read");
    return false;
  }
  print_arr("session.server_random_public",
            session.server_random_public,
            PUBLIC_KEY_SIZE);
  if (!derive_session_id() || !derive_session_key()) {
    printf("\nERROR: Generation session keys");
    return false;
  }

  return true;
}

bool session_close() {
  memset(&session, 0, sizeof(session));
}

void session_main(session_msg_type_e type,
                  uint8_t *in,
                  size_t in_size,
                  uint8_t *out,
                  size_t out_size) {
  char buffer[BUFFER_SIZE] = {0};

  switch (type) {
    case SESSION_MSG_SEND_DEVICE_KEY:
      if (!session_send_device_key(out)) {
        LOG_CRITICAL("xxec %d", __LINE__);
        comm_reject_invalid_cmd();
        clear_message_received_data();

        return SESSION_ERR_DEVICE_KEY;
      }

      out_size = PUBLIC_KEY_SIZE + DEVICE_SERIAL_SIZE + SIGNATURE_SIZE +
                 POSTFIX1_SIZE + POSTFIX2_SIZE;
      byte_array_to_hex_string(out, out_size, buffer, out_size * 2 + 1);
      break;

    case SESSION_MSG_RECEIVE_SERVER_KEY:
      if (!session_receive_server_key(in)) {
        LOG_CRITICAL("xxec %d", __LINE__);
        comm_reject_invalid_cmd();
        clear_message_received_data();

        return SESSION_ERR_SERVER_KEY;
      }

      in_size =
          DEVICE_SERIAL_SIZE + SIGNATURE_SIZE + POSTFIX1_SIZE + POSTFIX2_SIZE;
      byte_array_to_hex_string(in, in_size, buffer, in_size * 2 + 1);
      break;

    case SESSION_MSG_ENCRYPT:
      // if (!memcmp(session.session_id, {0}, SESSION_ID_SIZE) ||
      // !memcmp(session.session_key, {0}, SESSION_KEY_SIZE))
      session_msg_encryption(in, in_size, out, out_size);
      break;

    case SESSION_MSG_DECRYPT:
      // if (!memcmp(session.session_id, {0}, SESSION_ID_SIZE) ||
      // !memcmp(session.session_key, {0}, SESSION_KEY_SIZE))
      session_msg_decryption(in, in_size, out, out_size);
      break;

    case SESSION_CLOSE:
      session_close();
      break;

    default: {
      /* In case we ever encounter invalid query, convey to the host app */
      session_send_error();
      session.status = SESSION_ERR_INVALID;
      return;
    }
  }

  session.status = SESSION_OK;
}

void session_send_error() {
  printf("\nERROR: Invalid session query");
}

void session_msg_encryption(uint8_t *in,
                            size_t in_size,
                            uint8_t *out,
                            size_t out_size) {
  printf("\nInside Encryption");
}

void session_msg_decryption(uint8_t *in,
                            size_t in_size,
                            uint8_t *out,
                            size_t out_size) {
  printf("\nInside Decryption");
}

void test_session_main(session_msg_type_e type) {
  uint8_t in[BUFFER_SIZE];
  uint8_t out[BUFFER_SIZE];
  size_t in_size;
  size_t out_size;

  session.status = SESSION_ERR_INVALID;
  session_curve_init();

  switch (type) {
    case SESSION_MSG_SEND_DEVICE_KEY:
      break;
    case SESSION_MSG_RECEIVE_SERVER_KEY:
      in_size = PUBLIC_KEY_SIZE + SESSION_AGE_SIZE + DEVICE_SERIAL_SIZE +
                SIGNATURE_SIZE;
      test_generate_server_data(in, in_size);
      break;
    case SESSION_MSG_ENCRYPT:
      break;
    case SESSION_MSG_DECRYPT:
      break;
  }

  session_main(type, in, in_size, out, out_size);

  if (session.status != SESSION_OK) {
    printf("\nERORR: SESSION COMMAND FAILED");
  }

  printf("\nSESSION MSG STATUS: %d", session.status);

  return 0;
}

// TODO: functions to remove after testing

bool debug = true;
// TODO cleanup: delete after testing
char *print_arr(char *name, uint8_t *bytearray, size_t size) {
  char bytearray_hex[size * 2 + 1];
  if (debug == true) {
    uint8ToHexString(bytearray, size, bytearray_hex);
    printf("\n%s[%d bytes]: %s\n",
           name,
           (strlen(bytearray_hex) / 2),
           bytearray_hex);
  }
  return bytearray_hex;
}

void uint8ToHexString(const uint8_t *data, size_t size, char *hexstring) {
  for (size_t i = 0; i < size; ++i) {
    for (size_t i = 0; i < size; ++i) {
      sprintf(hexstring + 2 * i,
              "%02x",
              data[i]);    // Each byte represented by 2 characters + '\0'
    }
  }
  hexstring[size * 2] = '\0';    // Null-terminate the string
}

void test_uint32_to_uint8_array(uint32_t value, uint8_t arr[4]) {
  arr[0] = (value >> 24) & 0xFF;    // Extract the highest byte
  arr[1] = (value >> 16) & 0xFF;    // Extract the second highest byte
  arr[2] = (value >> 8) & 0xFF;     // Extract the second lowest byte
  arr[3] = value & 0xFF;            // Extract the lowest byte
}

void test_generate_server_data(uint8_t *server_message, size_t msg_size) {
  memzero(server_message, msg_size);

  uint8_t server_random[PRIVATE_KEY_SIZE];
  uint8_t server_random_public[PUBLIC_KEY_SIZE];
  curve_point server_random_public_point;
  if (!session_get_random_keys(
          server_random, server_random_public, server_random_public_point)) {
    printf("\nERROR: Server Random keys not generated");
    return false;
  }

  uint32_t session_age_int = 1234;
  uint8_t session_age[4];
  test_uint32_to_uint8_array(session_age_int, session_age);

  uint8_t offset = 0;
  memcpy(server_message + offset, server_random_public, PUBLIC_KEY_SIZE);
  offset += PUBLIC_KEY_SIZE;
  memcpy(server_message + offset, session_age, SESSION_AGE_SIZE);
  offset += SESSION_AGE_SIZE;
  memcpy(server_message + offset, atecc_data.device_serial, DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;

  uint8_t hash[32] = {0};
  sha256_Raw(server_message, offset, hash);
  auth_data_t signed_data =
      atecc_sign(hash);    // NOTE: But on server signing will be from master
                           // device derived private key
  memcpy(server_message + offset, signed_data.signature, SIGNATURE_SIZE);
  offset += SIGNATURE_SIZE;

  print_arr("sig detail", signed_data.signature, SIGNATURE_SIZE);
}