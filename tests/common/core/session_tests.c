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
#include <stdint.h>
#include <string.h>

#include "base58.h"
#include "core_session.h"
#include "curves.h"
#include "device_authentication_api.h"
#include "events.h"
#include "nist256p1.h"
#include "sha2.h"
#include "unity.h"
#include "unity_fixture.h"
#include "utils.h"

uint8_t private_key[SESSION_PRIV_KEY_SIZE] = {0};

void session_test_derive_priv_key() {
  hex_string_to_byte_array(
      "9FA623ED2545C4497B98FECAFC319C445637F46E8B1E5E724AAC2ED14605241D",
      SESSION_PRIV_KEY_SIZE * 2,
      private_key);
}

TEST_GROUP(core_session_test);

TEST_SETUP(core_session_test) {
  core_session_clear_metadata();
  session_test_derive_priv_key();
}

TEST_TEAR_DOWN(core_session_test) {
}

// server details used for tests
// priv_key=9FA623ED2545C4497B98FECAFC319C445637F46E8B1E5E724AAC2ED14605241D
// pub_key=
// xpub=0488B21E04CEBCC85D000000003B6442F1969C775AC2C3D23640F25F747AF3FD87660732CEB20A896BFB1AA00103902E2A4C22C1D74CEFD903BC39E7F9173FA940EBA0116CA8336ED24DF424AE3D
// session_random_pub_key=03dffa3474ddcda39ee629f17b74821d899ac5e9d0c5459be9cf431bb634b9c6e4

TEST(core_session_test, core_session_initate_ok) {
  core_msg_t core_msg = {
      .which_type = CORE_MSG_SESSION_START_TAG,
      {.session_start.which_cmd = CORE_SESSION_START_CMD_REQUEST_TAG,
       .session_start.request.which_request =
           CORE_SESSION_START_REQUEST_INITIATE_TAG}};
  session_error_type_e status = core_session_parse_start_message(&core_msg);
  TEST_ASSERT_EQUAL(SESSION_OK, status);
}

TEST(core_session_test, core_session_start_fail) {
  core_msg_t core_msg = {
      .which_type = CORE_MSG_SESSION_START_TAG,
      {.session_start.which_cmd = CORE_SESSION_START_CMD_REQUEST_TAG,
       .session_start.request.which_request =
           CORE_SESSION_START_REQUEST_START_TAG}};    ///< session start req
                                                      ///< without initiation
  session_error_type_e status = core_session_parse_start_message(&core_msg);
  TEST_ASSERT_EQUAL(SESSION_INVALID_STATE_ERROR, status);
}

TEST(core_session_test, core_session_aes_encrypt_session_state_terminated) {
  uint8_t test_bytes[33] = {0};
  hex_string_to_byte_array(
      "03dffa3474ddcda39ee629f17b74821d899ac5e9d0c5459be9cf431bb634b9c6e4",
      66,
      test_bytes);
  uint16_t len = 32;
  session_error_type_e status = session_aes_encrypt(test_bytes, &len);
  TEST_ASSERT_EQUAL(SESSION_INVALID_STATE_ERROR, status);
}

TEST(core_session_test, core_session_aes_decrypt_session_state_terminated) {
  uint8_t test_bytes[33] = {0};
  hex_string_to_byte_array(
      "03149c51bc8ddd582e447408bfc6b1fa96ac92a0059428c2884c29898b325dcc9f",
      66,
      test_bytes);
  uint16_t len = 32;
  session_error_type_e status = session_aes_decrypt(test_bytes, &len);
  TEST_ASSERT_EQUAL(SESSION_INVALID_STATE_ERROR, status);
}

TEST(core_session_test, core_session_wallet_id_check_fail) {
  core_msg_t core_msg = {
      .which_type = CORE_MSG_SESSION_START_TAG,
      {.session_start.which_cmd = CORE_SESSION_START_CMD_REQUEST_TAG,
       .session_start.request.which_request =
           CORE_SESSION_START_REQUEST_INITIATE_TAG}};
  // prepare session state to AWAIT
  session_error_type_e status = core_session_parse_start_message(&core_msg);
  TEST_ASSERT_EQUAL(SESSION_OK, status);

  core_msg.session_start.request.which_request =
      CORE_SESSION_START_REQUEST_START_TAG;
  hex_string_to_byte_array(
      "03dffa3474ddcda39ee629f17b74821d899ac5e9d0c5459be9cf431bb634b9c6e4",
      66,
      core_msg.session_start.request.start.session_random_public);

  uint8_t test_bytes[33] = {0};
  hex_string_to_byte_array(
      "03149c51bc8ddd582e447408bfc6b1fa96ac92a0059428c2884c29898b325dcc9f",
      66,
      test_bytes);
  memcpy(&core_msg.session_start.request.start.session_age,
         test_bytes,
         SESSION_AGE_SIZE);
  memcpy(
      core_msg.session_start.request.start.device_id,    ///< Invalid device_id
      test_bytes,
      DEVICE_SERIAL_SIZE);

  uint8_t payload[SESSION_PUB_KEY_SIZE + SESSION_AGE_SIZE + DEVICE_SERIAL_SIZE];
  uint32_t offset = 0;
  memcpy(payload,
         core_msg.session_start.request.start.session_random_public,
         SESSION_PUB_KEY_SIZE);
  offset += SESSION_PUB_KEY_SIZE;
  memcpy(payload + offset,
         &core_msg.session_start.request.start.session_age,
         SESSION_AGE_SIZE);
  offset += SESSION_AGE_SIZE;
  memcpy(payload + offset,
         core_msg.session_start.request.start.device_id,
         DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;
  uint8_t hash[SHA256_DIGEST_LENGTH];
  sha256_Raw(payload, offset, hash);

  ecdsa_sign_digest(&nist256p1,
                    private_key,
                    hash,
                    (uint8_t *)&core_msg.session_start.request.start.signature,
                    NULL,
                    NULL);

  status = core_session_parse_start_message(&core_msg);

  TEST_ASSERT_EQUAL(SESSION_DEVICE_ID_INVALID_ERROR, status);
}

TEST(core_session_test, core_session_all_ok) {
  core_msg_t core_msg = {
      .which_type = CORE_MSG_SESSION_START_TAG,
      {.session_start.which_cmd = CORE_SESSION_START_CMD_REQUEST_TAG,
       .session_start.request.which_request =
           CORE_SESSION_START_REQUEST_INITIATE_TAG}};
  // prepare session state to AWAIT
  session_error_type_e status = core_session_parse_start_message(&core_msg);
  TEST_ASSERT_EQUAL(SESSION_OK, status);

  core_msg.session_start.request.which_request =
      CORE_SESSION_START_REQUEST_START_TAG;

  hex_string_to_byte_array(
      "03dffa3474ddcda39ee629f17b74821d899ac5e9d0c5459be9cf431bb634b9c6e4",
      66,
      core_msg.session_start.request.start.session_random_public);
  uint8_t test_bytes[DEVICE_SERIAL_SIZE] = {0};
  hex_string_to_byte_array("8485011716705c7dbe8fdf69291fa3fc11f6bc30b55262e3819"
                           "6d23562707ed5",    ///< SHA256("device_id")
                           DEVICE_SERIAL_SIZE * 2,
                           test_bytes);
  memcpy(&core_msg.session_start.request.start.session_age,
         test_bytes,
         SESSION_AGE_SIZE);
  memcpy(core_msg.session_start.request.start.device_id,
         test_bytes,
         DEVICE_SERIAL_SIZE);

  uint8_t payload[SESSION_PUB_KEY_SIZE + SESSION_AGE_SIZE + DEVICE_SERIAL_SIZE];
  uint32_t offset = 0;
  memcpy(payload,
         &core_msg.session_start.request.start.session_random_public,
         SESSION_PUB_KEY_SIZE);
  offset += SESSION_PUB_KEY_SIZE;
  memcpy(payload + offset,
         &core_msg.session_start.request.start.session_age,
         SESSION_AGE_SIZE);
  offset += SESSION_AGE_SIZE;
  memcpy(payload + offset,
         &core_msg.session_start.request.start.device_id,
         DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;
  uint8_t hash[SHA256_DIGEST_LENGTH];
  sha256_Raw(payload, offset, hash);

  ecdsa_sign_digest(&nist256p1,
                    private_key,
                    hash,
                    (uint8_t *)&core_msg.session_start.request.start.signature,
                    NULL,
                    NULL);

  status = core_session_parse_start_message(&core_msg);

  TEST_ASSERT_EQUAL(SESSION_OK, status);
}