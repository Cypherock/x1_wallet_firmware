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
#include "card_fetch_data.h"
#include "curves.h"
#include "nfc.h"
#include "rand.h"

#define SESSION_BUFFER_SIZE 1024
#define SESSION_PUB_KEY_SIZE 33
#define SESSION_PRIV_KEY_SIZE 32
#define SESSION_AGE_SIZE 4

#define SESSION_ID_SIZE 16
#define SESSION_KEY_SIZE 32

#define SESSION_MSG_SIZE ((MSG_PRIVATE_SIZE + MSG_PUBLIC_SIZE) * 2)
#define SESSION_MSG_MAX 5

#define SESSION_MSG_IS_PRIVATE  0x01

extern const uint32_t session_key_rotation[2];

typedef enum {
  SESSION_MSG_SEND_DEVICE_KEY,
  SESSION_MSG_RECEIVE_SERVER_KEY,
  SESSION_MSG_ENCRYPT,
  SESSION_MSG_DECRYPT,
  SESSION_CLOSE,
  NUM_SESSION_MSG_TYPE_E
} session_msg_type_e;
typedef enum {
  SESSION_OK = 0,
  SESSION_ERR_INVALID = 0x01,
  SESSION_ERR_DEVICE_KEY = 0x02,
  SESSION_ERR_SERVER_KEY = 0x03,
  SESSION_ERR_ENCRYPT = 0x04,
  SESSION_ERR_DECRYPT = 0x05,
} session_error_type_e;

/**
 * @brief Stores the session information
 * @since v1.0.0
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

  uint8_t session_id[SESSION_ID_SIZE];
  uint8_t session_key[SESSION_PRIV_KEY_SIZE];

  SessionMsg SessionMsgs[SESSION_MSG_MAX];

  const char wallet_name[NAME_SIZE];

  session_error_type_e status;
} Session;
#pragma pack(pop)

extern Session session;

typedef struct {
  // session_init
  session_msg_type_e type;
  uint8_t server_message[SESSION_BUFFER_SIZE];

  // session_enc
  uint8_t msg_array_size;
  SessionMsg SessionMsgs[SESSION_MSG_MAX];
  uint8_t pass_key[SESSION_ID_SIZE];
  uint8_t wallet_id[WALLET_ID_SIZE];

  // output
  uint8_t device_message[SESSION_BUFFER_SIZE];
} inheritance_query_t;

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
                              size_t payload_size,
                              uint8_t *signature);

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
void session_append_signature(uint8_t *payload, size_t payload_size);

/**
 * @brief Starts the session creation process
 * @details It generates the device random, derives and stores the device
 * public key. It also generates the payload to be sent to the server.
 * @param session_details_data_array The buffer to store the payload to be sent
 *
 * @see SESSION_INIT
 * @since v1.0.0
 */
bool session_send_device_key(uint8_t *payload);

/**
 * @brief Completes the session creation process
 * @details It verifies the server response and stores the session id and
 * session random.
 * @param session_init_details The server response
 * @param verification_details The buffer to store the details
 * to be send back to the server to confirm
 * /home/parnika/Documents/GitHub/x1_wallet_firmware/common/libraries/util/session_utils.c:394:6the
 * verification.
 * @return true if the session is created, false otherwise
 *
 * @see SESSION_ESTABLISH
 * @since v1.0.0
 */
bool session_receive_server_key(uint8_t *server_message);

// TODO: Remove after testing
void print_msg(SessionMsg msg, uint8_t index);
char *print_arr(char *name, uint8_t *bytearray, size_t size);
void test_session_main(session_msg_type_e type);

#endif    // SESSION_UTILS