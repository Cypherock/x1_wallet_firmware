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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "aes.h"
#include "application_startup.h"
#include "atecc_utils.h"
#include "base58.h"
#include "bignum.h"
#include "bip32.h"
#include "core.pb.h"
#include "core_api.h"
#include "curves.h"
#include "device_authentication_api.h"
#include "ecdsa.h"
#include "flash_api.h"
#include "logger.h"
#include "nist256p1.h"
#include "options.h"
#include "sha2.h"
#include "utils.h"
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
session_config_t CONFIDENTIAL session = {0};

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

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
 * @brief Receives and processes the server's key as part of the session
 * initialization.
 *
 * @param server_message The buffer containing the server's message, which
 * includes the server's random public key, session age, and device ID.
 * @return true if the server's key is successfully received and processed.
 * @return false otherwise.
 */
static bool session_receive_server_key(const uint8_t *server_message);

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
 * point.
 * @return true if the random keys are successfully generated.
 * @return false otherwise.
 */
static bool session_get_random_keys(uint8_t *random, uint8_t *random_public);

/**
 * @brief Generates the device key as part of the session initialization.
 *
 * @param payload The buffer to store the payload data.
 * @return true if the device key is successfully generated.
 * @return false otherwise.
 */
static bool session_generate_device_key(uint8_t *payload);

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

  hdnode_public_ckd(&node, SESSION_KEY_INDEX);

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
  uint8_t buffer[SESSION_PUB_KEY_SIZE * 2] = {0};
  uint8_t hash[SHA256_DIGEST_LENGTH] = {0};

  memcpy(buffer, session.device_random_public, SESSION_PUB_KEY_SIZE);
  memcpy(buffer + SESSION_PUB_KEY_SIZE,
         session.server_random_public,
         SESSION_PUB_KEY_SIZE);

  sha256_Raw(buffer, sizeof(buffer), hash);

  memzero(session.session_iv, sizeof(session.session_iv));
  memcpy(session.session_iv, hash, sizeof(session.session_iv));
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

static bool session_get_random_keys(uint8_t *random, uint8_t *random_public) {
  memzero(random, SESSION_PRIV_KEY_SIZE);
  memzero(random_public, SESSION_PUB_KEY_SIZE);

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

  return true;
}

static bool session_generate_device_key(uint8_t *payload) {
  if (!session_get_random_keys(session.device_random,
                               session.device_random_public)) {
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

static bool session_receive_server_key(const uint8_t *server_message) {
  // Input Payload: Server_Random_public + Session Age + Device Id
  if (server_message == NULL) {
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
    return false;
  }

  if (!derive_session_iv_and_session_key()) {
    LOG_ERROR("\nERROR: Generation session keys");
    return false;
  }

  return true;
}

static void initiate_request(void) {
  uint8_t payload[SESSION_BUFFER_SIZE] = {0};
  if (!session_generate_device_key(payload)) {
    // TODO: Error Handling
    LOG_ERROR("xxec %d", __LINE__);
    send_core_error_msg_to_host(CORE_UNKNOWN_APP);
    clear_message_received_data();
    return;
  }
  send_core_session_start_response_to_host(payload);
}

static void start_request(const core_msg_t *core_msg) {
  uint8_t server_message_payload[SESSION_PUB_KEY_SIZE + SESSION_AGE_SIZE +
                                 DEVICE_SERIAL_SIZE +
                                 SESSION_SERVER_SIGNATURE_SIZE];
  uint32_t offset = 0;
  core_session_start_begin_request_t request =
      core_msg->session_start.request.start;
  memcpy(server_message_payload,
         request.session_random_public,
         SESSION_PUB_KEY_SIZE);
  offset += SESSION_PUB_KEY_SIZE;

  write_be(server_message_payload + offset,
           core_msg->session_start.request.start.session_age);
  offset += SESSION_AGE_SIZE;
  memcpy(server_message_payload + offset,
         core_msg->session_start.request.start.device_id,
         DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;

  memcpy(server_message_payload + offset,
         core_msg->session_start.request.start.signature,
         64);

  if (!session_receive_server_key(server_message_payload)) {
    // TODO: Error Handling
    LOG_ERROR("xxec %d", __LINE__);
    send_core_error_msg_to_host(CORE_UNKNOWN_APP);
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
      send_core_error_msg_to_host(CORE_UNKNOWN_APP);
      clear_message_received_data();
      break;
  }
}

session_error_type_e session_aes_encrypt(uint8_t *InOut_data, uint16_t *len) {
  ASSERT(InOut_data != NULL);
  ASSERT(len != NULL);

  uint16_t size = *len;
  uint8_t payload[size];
  memzero(payload, size);

  memcpy(payload, InOut_data, size);
  memzero(InOut_data, size);

  uint8_t last_block[AES_BLOCK_SIZE] = {0};
  uint8_t remainder = size % AES_BLOCK_SIZE;

  // Round down to last whole block
  size -= remainder;
  // Copy old last block
  memcpy(last_block, payload + size, remainder);
  // Pad new last block with number of missing bytes
  memset(last_block + remainder,
         AES_BLOCK_SIZE - remainder,
         AES_BLOCK_SIZE - remainder);

  // the IV gets mutated, so we make a copy not to touch the original
  uint8_t initialization_vector[AES_BLOCK_SIZE] = {0};
  memcpy(initialization_vector, session.session_iv, AES_BLOCK_SIZE);

  aes_encrypt_ctx ctx = {0};

  if (aes_encrypt_key256(session.session_key, &ctx) != EXIT_SUCCESS) {
    return SESSION_ENCRYPT_PACKET_KEY_ERR;
  }

  if (aes_cbc_encrypt(payload, InOut_data, size, initialization_vector, &ctx) !=
      EXIT_SUCCESS) {
    return SESSION_ENCRYPT_PACKET_ERR;
  }

  if (aes_cbc_encrypt(last_block,
                      InOut_data + size,
                      sizeof(last_block),
                      initialization_vector,
                      &ctx) != EXIT_SUCCESS) {
    return SESSION_ENCRYPT_PACKET_ERR;
  }

  size += sizeof(last_block);
  *len = size;

  memzero(&ctx, sizeof(ctx));

  return SESSION_ENCRYPT_PACKET_SUCCESS;
}

session_error_type_e session_aes_decrypt(uint8_t *InOut_data, uint16_t *len) {
  ASSERT(InOut_data != NULL);
  ASSERT(len != NULL);

  size_t size = *len;

  uint8_t payload[size];
  memcpy(payload, InOut_data, size);
  memzero(InOut_data, size);

  aes_decrypt_ctx ctx = {0};

  if (EXIT_SUCCESS != aes_decrypt_key256(session.session_key, &ctx)) {
    return SESSION_DECRYPT_PACKET_KEY_ERR;
  }

  if (aes_cbc_decrypt(payload, InOut_data, size, session.session_iv, &ctx) !=
      EXIT_SUCCESS) {
    return SESSION_DECRYPT_PACKET_ERR;
  }

  *len = size;
  memzero(&ctx, sizeof(ctx));

  return SESSION_DECRYPT_PACKET_SUCCESS;
}
