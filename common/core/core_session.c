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
#include "assert_conf.h"
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
#include "secp256k1.h"
#include "sha2.h"
#include "utils.h"
/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef struct {
  session_error_type_e type;
} session_error_t;
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static session_ctx_t *session_ctx = NULL;
/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/
session_private_t CONFIDENTIAL session = {0};
session_error_t session_error;

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define SET_ERROR_TYPE(x) session_error.type = x;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Derives the server's public key from the extended public key.
 *
 * @return true if the server's public key is successfully derived.
 * @return false otherwise.
 */
static void derive_server_public_key(uint8_t *server_verification_pub_key);

/**
 * @brief Verifies the session signature, critical for proceeding.
 */
static bool session_verify_server_signature();

/**
 * @brief Derives both the session IV and session key.
 *
 * @return true if both the session IV and session key are successfully derived.
 * @return false otherwise.
 */
static bool derive_session_iv_and_session_key();

/**
 * @brief Generates the device key and signs the payload as part of the session
 * initialization.
 *
 * @param payload The buffer to store the payload data.
 * @return true if the device key is successfully generated.
 * @return false otherwise.
 */
static bool session_create_device_payload(uint8_t *payload);

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
static void session_set_defaults() {
  SET_ERROR_TYPE(SESSION_DEFAULT_ERROR);
  core_session_clear_metadata();
}

static void session_handle_errors() {
  if (session_error.type == SESSION_OK) {
    return;
  }
  session_error_type_e type = session_error.type;
  LOG_ERROR("core_session_parse_start_message error_code:%d\n", type);
  switch (type) {
    case SESSION_DEFAULT_ERROR:
    case SESSION_MEMORY_ALLOCATION_ERROR:
    case SESSION_INPUT_INVALID_ERROR:
    case SESSION_KEY_GENERATION_ERROR:
    case SESSION_GET_DEVICE_ID_ERROR:
    case SESSION_DEVICE_ID_INVALID_ERROR:
    case SESSION_INVALID_STATE_ERROR:
    case SESSION_UNKNOWN_ERROR:
    case SESSION_SIGNATURE_VERIFICATION_ERROR: {
      // TODO: Update core error macros for session
      send_core_error_msg_to_host(CORE_DEVICE_SESSION_INVALID);
      core_session_clear_metadata();
    } break;

    default: {
      send_core_error_msg_to_host(CORE_DEVICE_SESSION_INVALID);
      core_session_clear_metadata();
    } break;
  }
}

static void derive_server_public_key(uint8_t *server_verification_pub_key) {
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
  memcpy(server_verification_pub_key, node.public_key, SESSION_PUB_KEY_SIZE);
}

static bool derive_session_iv_and_session_key() {
  // generate session iv
  uint8_t buffer[SESSION_PUB_KEY_SIZE * 2] = {0};
  uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
  memcpy(buffer, session_ctx->device.random_pub_key, SESSION_PUB_KEY_SIZE);
  memcpy(buffer + SESSION_PUB_KEY_SIZE,
         session_ctx->server.request_pointer->session_random_public,
         SESSION_PUB_KEY_SIZE);
  sha256_Raw(buffer, sizeof(buffer), hash);
  memzero(session.session_iv, sizeof(session.session_iv));
  memcpy(session.session_iv, hash, sizeof(session.session_iv));
  // generate session key
  if (ecdh_multiply(&secp256k1,
                    session_ctx->device.random_priv_key,
                    session_ctx->server.request_pointer
                        ->session_random_public,    // TODO: Update proto name
                                                    // to server_random_pub_key
                    session.session_key) != 0) {
    SET_ERROR_TYPE(SESSION_KEY_GENERATION_ERROR);
    return false;
  }
  return true;
}

static bool session_create_device_payload(uint8_t *payload) {
  // randomly generate private key
  random_generate(session_ctx->device.random_priv_key, SESSION_PRIV_KEY_SIZE);
  ecdsa_get_public_key33(&secp256k1,
                         session_ctx->device.random_priv_key,
                         session_ctx->device.random_pub_key);
  uint32_t offset = 0;
  // append device_pub_key
  memcpy(payload + offset,
         session_ctx->device.random_pub_key,
         SESSION_PUB_KEY_SIZE);
  offset += SESSION_PUB_KEY_SIZE;
  // TODO: standardize simulator handling for hardware specific functionality
  if (get_device_serial() != 0) {
    SET_ERROR_TYPE(SESSION_GET_DEVICE_ID_ERROR);
    return false;
  }
  memcpy(session_ctx->device.device_id,
         atecc_data.device_serial,
         DEVICE_SERIAL_SIZE);
  // append device_id
  memcpy(payload + offset, session_ctx->device.device_id, DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;
  // sign payload
  uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw(payload, offset, hash);
  auth_data_t signed_data = atecc_sign(hash);
  // append signature
  memcpy(payload + offset, signed_data.signature, SIGNATURE_SIZE);
  offset += SIGNATURE_SIZE;
  // append posfix1
  memcpy(payload + offset, signed_data.postfix1, POSTFIX1_SIZE);
  offset += POSTFIX1_SIZE;
  // append posfix2
  memcpy(payload + offset, signed_data.postfix2, POSTFIX2_SIZE);
}

static void initiate_request(void) {
  uint8_t payload[SESSION_PUB_KEY_SIZE + DEVICE_SERIAL_SIZE + SIGNATURE_SIZE +
                  POSTFIX1_SIZE + POSTFIX2_SIZE] = {0};
  if (session_create_device_payload(payload)) {
    send_core_session_start_response_to_host(payload);
    // populate private variables
    memcpy(
        session.device_id, session_ctx->device.device_id, DEVICE_SERIAL_SIZE);
    memcpy(session.device_random_priv_key,
           session_ctx->device.random_priv_key,
           SESSION_PRIV_KEY_SIZE);
    // indicate wait for server pub key
    session.state = SESSION_AWAIT;
  }
}

static bool session_verify_server_signature() {
  // create payload for verification
  uint8_t server_message_payload[SESSION_PUB_KEY_SIZE + SESSION_AGE_SIZE +
                                 DEVICE_SERIAL_SIZE +
                                 SESSION_SERVER_SIGNATURE_SIZE];
  uint32_t offset = 0;
  memcpy(server_message_payload,
         session_ctx->server.request_pointer->session_random_public,
         SESSION_PUB_KEY_SIZE);
  offset += SESSION_PUB_KEY_SIZE;
  write_be(server_message_payload + offset,
           session_ctx->server.request_pointer->session_age);
  offset += SESSION_AGE_SIZE;
  memcpy(server_message_payload + offset,
         session_ctx->server.request_pointer->device_id,
         DEVICE_SERIAL_SIZE);
  offset += DEVICE_SERIAL_SIZE;
  memcpy(server_message_payload + offset,
         session_ctx->server.request_pointer->signature,
         SESSION_SERVER_SIGNATURE_SIZE);
  // Verify Device ID
  if (memcmp(session.device_id,
             session_ctx->server.request_pointer->device_id,
             DEVICE_SERIAL_SIZE) != 0) {
    SET_ERROR_TYPE(SESSION_DEVICE_ID_INVALID_ERROR);
    return false;
  }
  // derive verification pub key
  uint8_t server_verification_pub_key[SESSION_PUB_KEY_SIZE];
  derive_server_public_key(server_verification_pub_key);
  uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw(server_message_payload, offset, hash);
  if (ecdsa_verify_digest(&nist256p1,
                          server_verification_pub_key,
                          server_message_payload + offset,
                          hash) != 0) {
    SET_ERROR_TYPE(SESSION_SIGNATURE_VERIFICATION_ERROR);
    return false;
  }

  return true;
}

static void start_request(const core_msg_t *core_msg) {
  ASSERT(core_msg != NULL);
  if (session.state !=
      SESSION_AWAIT) {    ///< Device keys must have been generated to proceed
                          ///< with session start request
    SET_ERROR_TYPE(SESSION_INVALID_STATE_ERROR);
    return;
  }
  session_ctx->server.request_pointer = &core_msg->session_start.request.start;
  if (session_verify_server_signature() &&
      derive_session_iv_and_session_key()) {
    // indicate valid session has been established
    session.state = SESSION_LIVE;
    send_core_session_start_ack_to_host();
  }
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void core_session_clear_metadata() {
  memzero(&session, sizeof(session_private_t));
  session.state = SESSION_TERMINATED;
}

session_error_type_e core_session_parse_start_message(
    const core_msg_t *core_msg) {
  session_set_defaults();
  size_t request_type = core_msg->session_start.request.which_request;
  session_ctx = (session_ctx_t *)malloc(sizeof(session_ctx_t));
  if (session_ctx == NULL) {
    SET_ERROR_TYPE(SESSION_MEMORY_ALLOCATION_ERROR);
    session_handle_errors();
    ASSERT(session_ctx != NULL);
  }
  switch (request_type) {
    case CORE_SESSION_START_REQUEST_INITIATE_TAG: {
      initiate_request();
    } break;

    case CORE_SESSION_START_REQUEST_START_TAG: {
      start_request(core_msg);
    } break;

    default:
      SET_ERROR_TYPE(SESSION_UNKNOWN_ERROR);
      break;
  }
  memzero(session_ctx, sizeof(session_ctx_t));
  free(session_ctx);
  session_handle_errors();
  return session_error.type;
}

session_error_type_e session_aes_encrypt(uint8_t *InOut_data, uint16_t *len) {
  ASSERT(InOut_data != NULL);
  ASSERT(len != NULL);
  if (session.state != SESSION_LIVE) {
    LOG_ERROR("session_aes_encrypt error_code:%d\n",
              SESSION_INVALID_STATE_ERROR);
    return SESSION_INVALID_STATE_ERROR;
  }

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
    LOG_ERROR("session_aes_encrypt error_code:%d\n", SESSION_ENCRYPTION_ERROR);
    return SESSION_ENCRYPTION_ERROR;
  }

  if (aes_cbc_encrypt(payload, InOut_data, size, initialization_vector, &ctx) !=
      EXIT_SUCCESS) {
    LOG_ERROR("session_aes_encrypt error_code:%d\n", SESSION_ENCRYPTION_ERROR);
    return SESSION_ENCRYPTION_ERROR;
  }

  if (aes_cbc_encrypt(last_block,
                      InOut_data + size,
                      sizeof(last_block),
                      initialization_vector,
                      &ctx) != EXIT_SUCCESS) {
    LOG_ERROR("session_aes_encrypt error_code:%d\n", SESSION_ENCRYPTION_ERROR);
    return SESSION_ENCRYPTION_ERROR;
  }

  size += sizeof(last_block);
  *len = size;

  memzero(&ctx, sizeof(ctx));

  return SESSION_ENCRYPTION_OK;
}

session_error_type_e session_aes_decrypt(uint8_t *InOut_data, uint16_t *len) {
  ASSERT(InOut_data != NULL);
  ASSERT(len != NULL);
  if (session.state != SESSION_LIVE) {
    LOG_ERROR("session_aes_encrypt error_code:%d\n",
              SESSION_INVALID_STATE_ERROR);
    return SESSION_INVALID_STATE_ERROR;
  }

  size_t size = *len;
  uint8_t payload[size];
  memcpy(payload, InOut_data, size);
  memzero(InOut_data, size);

  aes_decrypt_ctx ctx = {0};

  if (EXIT_SUCCESS != aes_decrypt_key256(session.session_key, &ctx)) {
    LOG_ERROR("session_aes_encrypt error_code:%d\n", SESSION_ENCRYPTION_ERROR);
    return SESSION_DECRYPTION_ERROR;
  }

  if (aes_cbc_decrypt(payload, InOut_data, size, session.session_iv, &ctx) !=
      EXIT_SUCCESS) {
    LOG_ERROR("session_aes_encrypt error_code:%d\n", SESSION_ENCRYPTION_ERROR);
    return SESSION_DECRYPTION_ERROR;
  }

  *len = size;
  memzero(&ctx, sizeof(ctx));

  return SESSION_DECRYPTION_OK;
}
