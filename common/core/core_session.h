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

#include "application_startup.h"
#include "atecc_utils.h"
#include "base58.h"
#include "bip32.h"
#include "card_fetch_data.h"
#include "core_api.h"
#include "curves.h"
#include "inheritance_main.h"
#include "nfc.h"
#include "rand.h"

#define SESSION_BUFFER_SIZE 1024
#define SESSION_PUB_KEY_SIZE 33
#define SESSION_PRIV_KEY_SIZE 32
#define SESSION_AGE_SIZE 4

#define SESSION_SERVER_SIGNATURE_SIZE 64

#define SESSION_IV_SIZE 16
#define SESSION_KEY_SIZE 32

extern const uint32_t session_key_rotation[1];

typedef enum {
  SESSION_OK = 0,
  SESSION_ERR_INVALID,
  SESSION_ERR_DEVICE_KEY,
  SESSION_ERR_SERVER_KEY,
  SESSION_ERR_ENCRYPT,
  SESSION_ERR_DECRYPT,
  SESSION_ENCRYPT_PACKET_SUCCESS,
  SESSION_DECRYPT_PACKET_SUCCESS,
  SESSION_ENCRYPT_PACKET_KEY_ERR,
  SESSION_ENCRYPT_PACKET_ERR,
  SESSION_DECRYPT_PACKET_KEY_ERR,
  SESSION_DECRYPT_PACKET_ERR
} session_error_type_e;

/**
 * @brief Stores the session information
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t device_id[DEVICE_SERIAL_SIZE];
  uint8_t device_random[SESSION_PRIV_KEY_SIZE];
  uint8_t device_random_public[SESSION_PUB_KEY_SIZE];
  curve_point device_random_public_point;

  uint8_t derived_server_public_key[SESSION_PUB_KEY_SIZE];
  uint8_t server_random_public[SESSION_PUB_KEY_SIZE];
  curve_point server_random_public_point;
  uint8_t session_age[SESSION_AGE_SIZE];
  uint8_t server_signature[SESSION_SERVER_SIGNATURE_SIZE];

  const char wallet_name[NAME_SIZE];

  uint8_t session_iv[SESSION_IV_SIZE];
  uint8_t session_key[SESSION_PRIV_KEY_SIZE];

} session_config_t;
#pragma pack(pop)

extern session_config_t session;

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
void core_session_parse_start_message(const core_msg_t *core_msg);

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
