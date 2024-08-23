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

#define SESSION_MSG_MAX 5
#define SESSION_PACKET_BUFFER 400
#define SESSION_PACKET_SIZE                                                    \
  (ENCRYPTED_DATA_SIZE * SESSION_MSG_MAX) +                                    \
      SESSION_PACKET_BUFFER    // (112 * 10) * 5 + 400 = 6000

extern const uint32_t session_key_rotation[2];

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
  secure_data_t session_msgs[SESSION_MSG_MAX];
  uint8_t msg_count;

  uint8_t session_iv[SESSION_IV_SIZE];
  uint8_t session_key[SESSION_PRIV_KEY_SIZE];

  uint8_t packet[SESSION_PACKET_SIZE];

} session_config_t;
#pragma pack(pop)

extern session_config_t session;

void core_session_parse_start_message(const core_msg_t *core_msg);

void core_session_clear_metadata();

#endif    // CORE_SESSION
