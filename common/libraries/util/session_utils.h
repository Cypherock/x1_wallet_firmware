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

#define SESSION_ID_SIZE 32
#define DEVICE_RANDOM_SIZE 32
#define SERVER_RANDOM_SIZE 32
#define SESSION_KEY_SIZE 32

/**
 * @brief Stores the session information
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t device_id[DEVICE_SERIAL_SIZE];
  uint8_t device_random[DEVICE_RANDOM_SIZE];
  uint8_t device_random_public[DEVICE_RANDOM_SIZE];

  uint8_t session_random_public[SERVER_RANDOM_SIZE];

  uint8_t session_id[SESSION_ID_SIZE];
  uint8_t session_key[SESSION_KEY_SIZE];

  uint8_t session_age[4];
} Session;
#pragma pack(pop)

/**
 * @brief Message structure received from server
 * messages
 * @details
 * For sending an authenticated message to server all the fields have to be
 * populated. For receiving an authenticated message from the server the
 * postfix1 and postfix2 fields will be ignored.
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct {
  uint16_t message_size;
  uint8_t *message;
  uint8_t server_random_public[SERVER_RANDOM_SIZE];
  uint8_t session_age[4];
} Server_Message;
#pragma pack(pop)

extern const uint32_t session_key_derv_data[3];
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
 * @brief Starts the session creation process
 * @details It generates the device random, derives and stores the device
 * public key. It also generates the payload to be sent to the server.
 * @param session_details_data_array The buffer to store the payload to be sent
 *
 * @see SESSION_INIT
 * @since v1.0.0
 */
void session_device_initiation(uint8_t *session_details_data_array);

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
bool session_server_response(uint8_t *session_init_details,
                             uint8_t *verification_details);

/**
 * @brief Deserializes the an authentication message to Message structure
 * @param data_array The serialized message
 * @param msg_size The size of the serialized message
 * @param msg The Message structure to store the deserialized message
 *
 * @since v1.0.0
 */
void byte_array_to_session_message(uint8_t *data_array,
                                   uint16_t msg_size,
                                   Message *msg);

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

#endif    // SESSION_UTILS