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

#define SESSION_IV_SIZE 16
#define SESSION_KEY_SIZE 32

#define SESSION_MSG_MAX 5
#define SESSION_PACKET_BUFFER 400
#define SESSION_PACKET_SIZE                                                    \
  (ENCRYPTED_DATA_SIZE * SESSION_MSG_MAX) +                                    \
      SESSION_PACKET_BUFFER    // (112 * 10) * 5 + 400 = 6000

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
  uint8_t server_signature[64];

  const char wallet_name[NAME_SIZE];
  secure_data_t session_msgs[SESSION_MSG_MAX];
  uint8_t msg_count;

  uint8_t session_iv[SESSION_IV_SIZE];
  uint8_t session_key[SESSION_PRIV_KEY_SIZE];

  uint8_t packet[SESSION_PACKET_SIZE];

  session_error_type_e status;
} session_config_t;
#pragma pack(pop)

extern session_config_t session;

typedef struct {
  // session_init
  session_msg_type_e type;
  uint8_t server_message[SESSION_BUFFER_SIZE];

  // session_enc
  uint8_t msg_count;
  secure_data_t session_msgs[SESSION_MSG_MAX];
  uint8_t pass_key[SESSION_IV_SIZE];
  uint8_t wallet_id[WALLET_ID_SIZE];

  // output
  uint8_t device_message[SESSION_BUFFER_SIZE];
} dummy_inheritance_query_t;

// bool session_receive_server_key(uint8_t *server_message);

bool session_plaindata_to_msg(uint8_t *plain_data[],
                              secure_data_t *msgs,
                              size_t *msg_count);
bool session_msg_to_plaindata(inheritance_plain_data_t *plain_data,
                              secure_data_t *msgs,
                              uint8_t *msg_count);

bool session_encrypt_secure_data(uint8_t *wallet_id,
                                 secure_data_t *msgs,
                                 size_t msg_count);
bool session_encrypt_packet(secure_data_t *msgs,
                            uint8_t msg_count,
                            uint8_t *packet,
                            size_t *packet_size);

bool session_decrypt_secure_data(uint8_t *wallet_id,
                                 secure_data_t *msgs,
                                 size_t msg_count);
bool session_decrypt_packet(secure_data_t *msgs,
                            uint32_t *msg_count,
                            uint8_t *packet,
                            uint16_t *packet_size);

void core_session_start_parse(const core_msg_t *core_msg);
// TODO: Remove after testing
void print_msg(secure_data_t msg, uint8_t index);
char *print_arr(char *name, uint8_t *bytearray, size_t size);
void test_session_main(session_msg_type_e type);

void session_close();

#endif    // SESSION_UTILS