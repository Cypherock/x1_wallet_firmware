/**
 * @author  Cypherock X1 Team
 * @brief   Header file containing the session functions
 *          This file declares the functions used to create and manage the
 *          session, send authentication requests and verify the responses.
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef CORE_SESSION
#define CORE_SESSION

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "core.pb.h"
#include "device_authentication_api.h"
#include "ecdsa.h"
#include "wallet.h"

#define SESSION_PUB_KEY_SIZE 33
#define SESSION_PRIV_KEY_SIZE 32
#define SESSION_AGE_SIZE 4

#define SESSION_KEY_INDEX 0

#define SESSION_SERVER_SIGNATURE_SIZE 64

#define SESSION_IV_SIZE 16
#define SESSION_KEY_SIZE 32

extern const uint32_t session_key_rotation[1];

typedef enum {
  SESSION_DEFAULT_ERROR = 0,
  SESSION_MEMORY_ALLOCATION_ERROR,
  SESSION_INPUT_INVALID_ERROR,
  SESSION_KEY_GENERATION_ERROR,
  SESSION_GET_DEVICE_ID_ERROR,
  SESSION_DEVICE_ID_INVALID_ERROR,
  SESSION_SIGNATURE_VERIFICATION_ERROR,
  SESSION_INVALID_STATE_ERROR,    ///< Indicates partial/corrupt session
                                  ///< state
  SESSION_UNKNOWN_ERROR,
  SESSION_ENCRYPTION_ERROR,
  SESSION_DECRYPTION_ERROR,
  SESSION_ENCRYPTION_OK,
  SESSION_DECRYPTION_OK,
  SESSION_OK,
} session_error_type_e;

typedef enum {
  SESSION_INIT = 0,
  SESSION_AWAIT,    ///< Session partial state; device keys generated,
                    ///< await server pub key
  SESSION_LIVE
} session_state_type_e;

#pragma pack(push, 1)
typedef struct {
  uint8_t device_id[DEVICE_SERIAL_SIZE];
  uint8_t random_priv_key[SESSION_PRIV_KEY_SIZE];
  uint8_t random_pub_key[SESSION_PUB_KEY_SIZE];
} session_device_config_t;

typedef struct {
  const core_session_start_begin_request_t *request_pointer;
} session_server_config_t;

typedef struct {
  session_device_config_t device;
  session_server_config_t server;
} session_ctx_t;

typedef struct {
  uint8_t device_id[DEVICE_SERIAL_SIZE];
  uint8_t device_random_priv_key[SESSION_PRIV_KEY_SIZE];
  uint8_t device_random_pub_key[SESSION_PUB_KEY_SIZE];
  uint8_t session_iv[SESSION_IV_SIZE];
  uint8_t session_key[SESSION_PRIV_KEY_SIZE];
  session_state_type_e state;    ///< Indicates session current state.
} session_private_t;
#pragma pack(pop)

extern session_private_t session;

/**
 * @brief Clears the metadata related to the session configuration.
 *
 * This function zeroes out the memory occupied by the session configuration
 * structure (`session_config_t`). It is typically called when closing or
 * resetting a session.
 */
void core_session_clear_metadata();

/**
 * @brief Parses the start message received from the host.
 *
 * This function processes the session start message and determines the type of
 * request (initiate or start). Depending on the request type, it invokes the
 * corresponding handler functions (`initiate_request()` or `start_request()`).
 *
 * @param core_msg Pointer to the core message containing the session start
 * data.
 */
session_error_type_e core_session_parse_start_message(
    const core_msg_t *core_msg);

/**
 * @brief Encrypts data using AES-CBC mode.
 *
 * This function encrypts the input data using AES-CBC (Cipher Block Chaining)
 * mode. It operates on the provided data in-place, updating the `InOut_data`
 * buffer with the encrypted result. The function also handles padding and IV
 * (Initialization Vector).
 *
 * @param InOut_data Pointer to the data to be encrypted (input and output).
 * @param len Pointer to the length of the data (input and output).
 * @return The encryption status (success or error code).
 */
session_error_type_e session_aes_encrypt(uint8_t *InOut_data, uint16_t *len);

/**
 * @brief Decrypts data using AES-CBC mode.
 *
 * This function decrypts the input data using the session key and IV.
 *
 * @param InOut_data Pointer to the input data (encrypted) and output data
 * (decrypted).
 * @param len Pointer to the length of the data (updated with actual decrypted
 * length).
 * @return The session error type indicating the decryption result.
 */
session_error_type_e session_aes_decrypt(uint8_t *InOut_data, uint16_t *len);

#endif    // CORE_SESSION
