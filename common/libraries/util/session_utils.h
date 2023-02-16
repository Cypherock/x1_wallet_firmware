/**
 * @file session_utils.h
 * @author  Cypherock X1 Team
 * @brief Functions for session management
 * @version 0.1
 * @date 2023-02-07
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef SESSION_UTILS
#define SESSION_UTILS

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "bip32.h"
#include "base58.h"
#include "nist256p1.h"
#include "nfc.h"
#include "atecc_utils.h"
#include "rand.h"

#define SESSION_ID_SIZE    32
#define DEVICE_RANDOM_SIZE 32

#pragma pack(push, 1)
typedef struct {
  uint8_t device_random[DEVICE_RANDOM_SIZE];
  uint8_t device_id[DEVICE_SERIAL_SIZE];
  uint8_t session_id[SESSION_ID_SIZE];
  uint8_t public_key[33];
  uint16_t session_age;
} Session;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint16_t message_size;
  uint8_t *message;
  uint8_t signature[SIGNATURE_SIZE];
  uint8_t postfix1[POSTFIX1_SIZE];
  uint8_t postfix2[POSTFIX2_SIZE];
} Message;
#pragma pack(pop)

extern uint8_t session_key_derv_data[12];

void derive_public_key(Session *session);

bool verify_session_signature(Session *session, uint8_t *payload, uint16_t
payload_length, uint8_t *buffer);

void session_pre_init(Session *session, Message *session_pre_init_details);

bool session_init(Session *session, Message *session_init_details);

void byte_array_to_session_message(uint8_t *data_array, uint16_t msg_size,
                                   Message *msg);

uint8_t session_message_to_byte_array(Message msg, uint8_t *data_array);

void append_signature(uint8_t *payload, uint16_t payload_length, Message
*message);

#endif //SESSION_UTILS
