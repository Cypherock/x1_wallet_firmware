/**
 * @author  Cypherock X1 Team
 * @brief   Definition of the session functions
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

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "core_session.h"

#include "core.pb.h"
#include "inheritance_main.h"
#include "logger.h"
#include "options.h"
#include "pb_decode.h"
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
const ecdsa_curve *curve;
const uint32_t session_key_rotation[2] = {6, 7};
session_config_t CONFIDENTIAL session = {0};

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Receives and processes the server's key as part of the session
 * initialization.
 *
 * @param server_message The buffer containing the server's message, which
 * includes the server's random public key, session age, and device ID.
 * @return true if the server's key is successfully received and processed.
 * @return false otherwise.
 */
static bool session_receive_server_key(uint8_t *server_message);

/**
 * @brief Initializes the curve parameters for the session.
 */
static void session_curve_init();

/**
 * @brief Derives the server's public key from the extended public key.
 *
 * @return true if the server's public key is successfully derived.
 * @return false otherwise.
 */
static bool derive_server_public_key();

/**
 * @brief Verifies the session signature using the provided payload and
 * signature.
 *
 * @param payload The payload data to verify.
 * @param payload_size The size of the payload.
 * @param signature The signature to verify.
 * @return true if the signature is valid.
 * @return false otherwise.
 */
static bool verify_session_signature(const uint8_t *payload,
                                     const size_t payload_size,
                                     const uint8_t *signature);

/**
 * @brief Derives the session initialization vector (IV) using the server's and
 * device's random public points.
 */
static void derive_session_iv();

/**
 * @brief Derives the session key using ECDH multiplication.
 *
 * @return true if the session key is successfully derived.
 * @return false otherwise.
 */
static bool derive_session_key();

/**
 * @brief Derives both the session IV and session key.
 *
 * @return true if both the session IV and session key are successfully derived.
 * @return false otherwise.
 */
static bool derive_session_iv_and_session_key();

/**
 * @brief Appends a signature to the payload.
 *
 * @param payload The payload to append the signature to.
 * @param payload_size The size of the payload.
 */
static void session_append_signature(uint8_t *payload,
                                     const size_t payload_size);

/**
 * @brief Generates random keys for the session.
 *
 * @param random The buffer to store the generated random key.
 * @param random_public The buffer to store the generated random public key.
 * @param random_public_point The curve point to store the random public key
 * point.
 * @return true if the random keys are successfully generated.
 * @return false otherwise.
 */
static bool session_get_random_keys(uint8_t *random,
                                    uint8_t *random_public,
                                    curve_point random_public_point);

/**
 * @brief Sends the device key as part of the session initialization.
 *
 * @param payload The buffer to store the payload data.
 * @return true if the device key is successfully sent.
 * @return false otherwise.
 */
static bool session_send_device_key(uint8_t *payload);

/**
 * @brief Converts a 32-bit unsigned integer to a 4-byte array.
 *
 * @param value The 32-bit unsigned integer to convert.
 * @param arr The array to store the converted bytes.
 */
static void uint32_to_uint8_array(uint32_t value, uint8_t arr[4]);

/**
 * @brief Initiates a request by sending the device key.
 */
static void initiate_request(void);

/**
 * @brief Starts a request by processing the core message.
 *
 * @param core_msg The core message containing the request data.
 */
static void start_request(const core_msg_t *core_msg);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static void session_curve_init() {
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

  memcpy(
      session.derived_server_public_key, node.public_key, SESSION_PUB_KEY_SIZE);

  return true;
}

static bool verify_session_signature(const uint8_t *payload,
                                     const size_t payload_size,
                                     const uint8_t *signature) {
  uint8_t hash[32] = {0};
  sha256_Raw(payload, payload_size, hash);

  bool is_signature_valid =
      ecdsa_verify_digest(
          &nist256p1, session.derived_server_public_key, signature, hash) == 0;

  return is_signature_valid;
}

static void derive_session_iv() {
  curve_point server_random_public_point;
  memcpy(&server_random_public_point,
         &session.server_random_public_point,
         sizeof(curve_point));
  point_add(
      curve, &session.device_random_public_point, &server_random_public_point);

  uint8_t session_iv[2 * SESSION_PRIV_KEY_SIZE];
  bn_write_be(&server_random_public_point.x, session_iv);
  bn_write_be(&server_random_public_point.y,
              session_iv + SESSION_PRIV_KEY_SIZE);

  sha256_Raw(session_iv, 2 * SESSION_PUB_KEY_SIZE, session.session_iv);
}

static bool derive_session_key() {
  if (ecdh_multiply(curve,
                    session.device_random,
                    session.server_random_public,
                    session.session_key) != 0) {
    LOG_ERROR("ERROR: Session key not generated");
    return false;
  }
  return true;
}

static bool derive_session_iv_and_session_key() {
  derive_session_iv();
  return derive_session_key();
}

static void session_append_signature(uint8_t *payload,
                                     const size_t payload_size) {
  uint8_t hash[32] = {0};    // hash size reference taken from the atecc_sign
  sha256_Raw(payload, payload_size, hash);
  auth_data_t signed_data = atecc_sign(hash);

  uint8_t offset = payload_size;
  memcpy(payload + offset, signed_data.signature, SIGNATURE_SIZE);
  offset += SIGNATURE_SIZE;
  memcpy(payload + offset, signed_data.postfix1, POSTFIX1_SIZE);
  offset += POSTFIX1_SIZE;
  memcpy(payload + offset, signed_data.postfix2, POSTFIX2_SIZE);
  offset += POSTFIX2_SIZE;
}

static bool session_get_random_keys(uint8_t *random,
                                    uint8_t *random_public,
                                    curve_point random_public_point) {
  memzero(random, SESSION_PRIV_KEY_SIZE);
  memzero(random_public, SESSION_PUB_KEY_SIZE);
  memset(&random_public_point, 0, sizeof(random_public_point));

#if USE_SIMULATOR == 0
  random_generate(random, SESSION_PRIV_KEY_SIZE);
#else
  uint8_t get_ec_random[32] = {0x0b, 0x78, 0x9a, 0x1e, 0xb8, 0x0b, 0x7a, 0xac,
                               0x97, 0xa1, 0x54, 0xd7, 0x0c, 0x5a, 0x53, 0x95,
                               0x6f, 0x9c, 0xed, 0x97, 0x6f, 0xc7, 0xed, 0x7f,
                               0xf9, 0x10, 0x01, 0xc1, 0xa8, 0x30, 0xde, 0xb1};
  memcpy(random, get_ec_random, SESSION_PRIV_KEY_SIZE);
#endif
  session_curve_init();
  ecdsa_get_public_key33(curve, random, random_public);

  if (!ecdsa_read_pubkey(curve, random_public, &random_public_point)) {
    LOG_ERROR("\nERROR: Random public key point not read");
    return false;
  }

  return true;
}

static bool session_send_device_key(uint8_t *payload) {
  if (!session_get_random_keys(session.device_random,
                               session.device_random_public,
                               session.device_random_public_point)) {
    LOG_ERROR("\nERROR: Device Random keys not generated");
    return false;
  }

// TODO: standardize simulator handling for hardware specific functionality
#if USE_SIMULATOR == 0
  if (get_device_serial() != 0) {
    LOG_ERROR("\nERROR: Device Serial fetch failed");
    return false;
  }
  memcpy(session.device_id, atecc_data.device_serial, DEVICE_SERIAL_SIZE);
#else
  memcpy(session.device_id, session.device_random, DEVICE_SERIAL_SIZE);
#endif

  uint32_t offset = 0;
  memcpy(payload + offset, session.device_random_public, SESSION_PUB_KEY_SIZE);
  offset += SESSION_PUB_KEY_SIZE;
  memcpy(payload + offset, session.device_id, DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;
  session_append_signature(payload, offset);

  return true;
}

static bool session_receive_server_key(uint8_t *server_message) {
  // Input Payload: Server_Random_public + Session Age + Device Id
  if (server_message != NULL) {
    LOG_ERROR("\nERROR: server_message not set");
    return false;
  }

  if (!derive_server_public_key()) {
    LOG_ERROR("\nERROR: Server Randoms not read");
    return false;
  }

  size_t server_message_size =
      SESSION_PUB_KEY_SIZE + SESSION_AGE_SIZE + DEVICE_SERIAL_SIZE;

  if (!verify_session_signature(server_message,
                                server_message_size,
                                server_message + server_message_size)) {
    LOG_ERROR("\nERROR: Message from server invalid");
    return false;
  }

  uint8_t offset = 0;
  memcpy(session.server_random_public,
         server_message + offset,
         SESSION_PUB_KEY_SIZE);
  offset += SESSION_PUB_KEY_SIZE;
  memcpy(session.session_age, server_message + offset, SESSION_AGE_SIZE);
  offset += SESSION_AGE_SIZE;
  // Verify Device ID
  if (memcmp(session.device_id, server_message + offset, DEVICE_SERIAL_SIZE) !=
      0) {
  }
  offset += DEVICE_SERIAL_SIZE;

  if (!ecdsa_read_pubkey(curve,
                         session.server_random_public,
                         &session.server_random_public_point)) {
    LOG_ERROR("\nERROR: Server random public key point not read");
    return false;
  }

  if (!derive_session_iv_and_session_key()) {
    LOG_ERROR("\nERROR: Generation session keys");
    return false;
  }

  return true;
}

// TODO: convert this into macro
static void uint32_to_uint8_array(uint32_t value, uint8_t arr[4]) {
  arr[0] = (value >> 24) & 0xFF;    // Extract the highest byte
  arr[1] = (value >> 16) & 0xFF;    // Extract the second highest byte
  arr[2] = (value >> 8) & 0xFF;     // Extract the second lowest byte
  arr[3] = value & 0xFF;            // Extract the lowest byte
}

static void initiate_request(void) {
  uint8_t payload[SESSION_BUFFER_SIZE] = {0};
  if (!session_send_device_key(payload)) {
    // TODO: Error Handling
    LOG_ERROR("xxec %d", __LINE__);
    comm_reject_invalid_cmd();
    clear_message_received_data();
    return;
  }
  send_core_session_start_response_to_host(payload);
}

static void start_request(const core_msg_t *core_msg) {
  uint8_t server_message_payload[SESSION_PUB_KEY_SIZE + SESSION_AGE_SIZE +
                                 DEVICE_SERIAL_SIZE];
  uint32_t offset = 0;
  core_session_start_begin_request_t request =
      core_msg->session_start.request.start;
  memcpy(server_message_payload,
         request.session_random_public,
         SESSION_PUB_KEY_SIZE);
  offset += SESSION_PUB_KEY_SIZE;

  uint32_to_uint8_array(core_msg->session_start.request.start.session_age,
                        server_message_payload + SESSION_PUB_KEY_SIZE);
  offset += SESSION_AGE_SIZE;
  memcpy(server_message_payload + offset,
         core_msg->session_start.request.start.device_id,
         DEVICE_SERIAL_SIZE);

  if (!session_receive_server_key(server_message_payload)) {
    // TODO: Error Handling
    LOG_ERROR("xxec %d", __LINE__);
    comm_reject_invalid_cmd();
    clear_message_received_data();

    return;
  }
  send_core_session_start_ack_to_host();
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void core_session_clear_metadata() {
  memzero(&session, sizeof(session_config_t));
}

void core_session_parse_start_message(const core_msg_t *core_msg) {
  size_t request_type = core_msg->session_start.request.which_request;
  switch (request_type) {
    case CORE_SESSION_START_REQUEST_INITIATE_TAG: {
      initiate_request();
    } break;

    case CORE_SESSION_START_REQUEST_START_TAG: {
      start_request(core_msg);
    } break;

    default:
      // TODO: Error Handling
      comm_reject_invalid_cmd();
      clear_message_received_data();
      break;
  }
}
