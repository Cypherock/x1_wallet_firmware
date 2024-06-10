/**
 * @file    session_utils.h
 * @author  Cypherock X1 Team
 * @brief   Header file containing the session utility functions
 *          This file declares the functions used to create and manage the
 *          session, send authentication requests and verify the responses.
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef SESSION_UTILS
#define SESSION_UTILS

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "atecc_utils.h"
#include "base58.h"
#include "bip32.h"
#include "curves.h"
#include "nfc.h"
#include "rand.h"

#define PUBLIC_KEY_SIZE 32
#define PRIVATE_KEY_SIZE 32
#define SESSION_AGE_SIZE 32

#define SESSION_ID_SIZE 64
#define SESSION_KEY_SIZE 64

/**
 * @brief Stores the session information
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t device_id[DEVICE_SERIAL_SIZE];
  uint8_t device_random[PRIVATE_KEY_SIZE];
  uint8_t device_random_public[PUBLIC_KEY_SIZE];

  uint8_t derived_server_public_key[PUBLIC_KEY_SIZE];
  uint8_t server_random_public[PUBLIC_KEY_SIZE];

  uint8_t session_id[SESSION_ID_SIZE];
  uint8_t session_key[PRIVATE_KEY_SIZE];

  uint8_t session_age[SESSION_AGE_SIZE];
} Session;
#pragma pack(pop)

extern const uint32_t session_key_rotation[2];
extern Session session;

/**
 * @brief Verified the signature of the payload
 * @param payload The payload to be verified
 * @param payload_length The length of the payload
 * @param buffer The buffer to store the signature
 * @return true if the signature is verified, false otherwise
 *
 * @see session_init()
 * @since v1.0.0
 */
bool verify_session_signature(uint8_t *payload,
                              uint8_t payload_length,
                              uint8_t *buffer);

/**
 * @brief Generates the payload to be sent to the server.
 * @details It generates the signature on the payload and appends the
 * signature to the payload. It further appends the postfix1 and postfix2
 * for verification on the server side.
 * @param payload The payload to be sent to the server
 * @param payload_length The length of the payload
 * @param signature_details The buffer to store the signature and the postfixes
 *
 * @see session_pre_init(), session_init()
 * @since v1.0.0
 */
void session_append_signature(uint8_t *payload,
                              uint8_t payload_length,
                              uint8_t *signature_details);

/**
 * @brief Starts the session creation process
 * @details It generates the device random, derives and stores the device
 * public key. It also generates the payload to be sent to the server.
 * @param session_details_data_array The buffer to store the payload to be sent
 *
 * @see SESSION_INIT
 * @since v1.0.0
 */
void session_send_device_key(Session session, uint8_t *payload);

/**
 * @brief Completes the session creation process
 * @details It verifies the server response and stores the session id and
 * session random.
 * @param session_init_details The server response
 * @param verification_details The buffer to store the details
 * to be send back to the server to confirm the verification.
 * @return true if the session is created, false otherwise
 *
 * @see SESSION_ESTABLISH
 * @since v1.0.0
 */
bool session_get_server_key(Session session, uint8_t *server_message);

void session_initiation();

#endif    // SESSION_UTILS