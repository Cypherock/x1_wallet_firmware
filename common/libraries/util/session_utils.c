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

const uint32_t session_key_rotation[2] = {6, 7};
Session session;

static void derive_server_public_key() {
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
}

bool verify_session_signature(uint8_t *payload,
                              uint8_t payload_size,
                              uint8_t *buffer) {
  uint8_t hash[32] = {0};
  sha256_Raw(payload, payload_size, hash);

  uint8_t status = ecdsa_verify_digest(
      &nist256p1, session.derived_server_public_key, buffer, hash);

  return (status == 0);
};

static void derive_session_id() {
  uint8_t session_id[SESSION_ID_SIZE];
  memcpy(session_id, session.server_random_public, PUBLIC_KEY_SIZE);
  memcpy(session_id + PUBLIC_KEY_SIZE, session.device_random, PRIVATE_KEY_SIZE);

  sha256_Raw(session_id, SESSION_ID_SIZE, session.session_id);
}

static void derive_session_key() {
  uint8_t session_key[SESSION_KEY_SIZE];
  memcpy(session_key, session.server_random_public, PUBLIC_KEY_SIZE);
  memcpy(
      session_key + PUBLIC_KEY_SIZE, session.device_random, PRIVATE_KEY_SIZE);

  sha256_Raw(SESSION_KEY_SIZE, SESSION_KEY_SIZE, session.session_key);
}

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

void session_get_random_keys(uint8_t *random, uint8_t *random_public) {
  const ecdsa_curve *curve = get_curve_by_name(SECP256K1_NAME)->params;
  const curve_point *pubkey_point;

  random_generate(random, PUBLIC_KEY_SIZE);
  scalar_multiply(curve, random, pubkey_point);
  compress_coords(&pubkey_point, &random_public);
}

void session_send_device_key(Session session, uint8_t *payload) {
  // Output Payload: Device Random (32) + Device Id (32) + Signature (64) +
  // Postfix1 + Postfix2

  // Generate device randoms
  uint8_t device_random[PRIVATE_KEY_SIZE];
  uint8_t device_random_public[PUBLIC_KEY_SIZE];
  session_get_random_keys(device_random, device_random_public);
  // session_get_random_keys(session.device_random,
  // session.device_random_public);

  // Get device_id
  // get_device_serial();
  // memcpy(session.device_id, atecc_data.device_serial, DEVICE_SERIAL_SIZE); //
  // TODO: uncomment after testing
  memcpy(session.device_id, session.device_random, DEVICE_SERIAL_SIZE);

  // Construct output payload
  uint8_t offset = 0;
  memcpy(payload, session.device_random_public, PUBLIC_KEY_SIZE);
  offset += PUBLIC_KEY_SIZE;
  memcpy(payload + offset, session.device_id, DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;
  session_append_signature(payload, offset, payload + offset);
}

bool session_get_server_key(Session session, uint8_t *server_message) {
  // Output Payload: Server_Random_public + Session Age + Device Id
  uint8_t offset = 0;
  memcpy(session.server_random_public, server_message, PUBLIC_KEY_SIZE);
  offset += PUBLIC_KEY_SIZE;
  memcpy(session.session_age, server_message + offset, SESSION_AGE_SIZE);
  offset += SESSION_AGE_SIZE;
  if (memcmp(session.device_id, server_message + offset, DEVICE_SERIAL_SIZE)) {
    return false;
  }
  offset += DEVICE_SERIAL_SIZE;

  if (!verify_session_signature(server_message, offset, server_message)) {
    return false;
  }

  return true;
}