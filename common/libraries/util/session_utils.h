/**
 * @file    session_utils.h
 * @author  Cypherock X1 Team
 * @brief   Header file containing the session utility functions
 *          This file declares the functions used to create and manage the
 *          session, send authentication requests and verify the responses.
 *
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
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
#define SESSION_RANDOM_SIZE 32


/**
 * @brief Stores the session information
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t device_random[DEVICE_RANDOM_SIZE];
  uint8_t session_random[SESSION_RANDOM_SIZE];
  uint8_t device_id[DEVICE_SERIAL_SIZE];
  uint8_t session_id[SESSION_ID_SIZE];
  uint8_t public_key[33];
  uint32_t session_age;
} Session;
#pragma pack(pop)

/**
 * @brief A generic message structure to send and receive authenticated
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
  uint8_t signature[SIGNATURE_SIZE];
  uint8_t postfix1[POSTFIX1_SIZE];
  uint8_t postfix2[POSTFIX2_SIZE];
} Message;
#pragma pack(pop)

extern uint8_t session_key_derv_data[12];
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
bool verify_session_signature(uint8_t *payload, uint8_t payload_length,
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
void session_pre_init(uint8_t *session_details_data_array);

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
bool session_init(uint8_t *session_init_details, uint8_t
*verification_details);

/**
 * @brief Deserializes the an authentication message to Message structure
 * @param data_array The serialized message
 * @param msg_size The size of the serialized message
 * @param msg The Message structure to store the deserialized message
 *
 * @since v1.0.0
 */
void byte_array_to_session_message(uint8_t *data_array, uint16_t msg_size,
                                   Message *msg);

/**
 * @brief Serializes the Message structure to an authentication message to
 * be sent to the server
 * @param message The Message structure to be serialized
 * @param data_array The buffer to store the serialized message
 * @return uint8_t The size of the serialized message
 *
 * @since v1.0.0
 */
uint8_t session_message_to_byte_array(Message message, uint8_t *data_array);

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
void append_signature(uint8_t *payload, uint8_t payload_length, uint8_t
*signature_details);

#endif //SESSION_UTILS
