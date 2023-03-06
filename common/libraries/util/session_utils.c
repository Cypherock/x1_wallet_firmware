/**
 * @file    session_utils.c
 * @author  Cypherock X1 Team
 * @brief   Definition of the session utility functions
 *          This file defines the functions used to create and manage the
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

#include "session_utils.h"
#include "controller_level_four.h"

uint32_t session_key_derv_data[3] = {0};
Session session;

static void derive_public_key() {
    HDNode session_node;
    uint32_t index;
    char xpub[112] = {'\0'};

    base58_encode_check(get_card_root_xpub(), FS_KEYSTORE_XPUB_LEN,
                        nist256p1_info.hasher_base58, xpub, 112);
    hdnode_deserialize_public((char *) xpub,
                              0x0488b21e,
                              NIST256P1_NAME,
                              &session_node,
                              NULL);

    index = session_key_derv_data[0];
    hdnode_public_ckd(&session_node, index);

    index = session_key_derv_data[1];
    hdnode_public_ckd(&session_node, index);

    index = session_key_derv_data[2];
    hdnode_public_ckd(&session_node, index);

    memcpy(session.public_key, session_node.public_key,
           sizeof(session.public_key));
}

static void derive_session_id() {
    uint8_t payload[SESSION_RANDOM_SIZE + DEVICE_RANDOM_SIZE];
    memcpy(payload, session.session_random, SESSION_RANDOM_SIZE);
    memcpy(payload + SESSION_RANDOM_SIZE, session.device_random,
           DEVICE_RANDOM_SIZE);

    sha256_Raw(payload, sizeof(payload), session.session_id);
}

bool verify_session_signature(uint8_t *payload, uint8_t
payload_length, uint8_t *buffer) {

    uint8_t hash[32] = {0};
    sha256_Raw(payload, payload_length, hash);

    uint8_t
        status = ecdsa_verify_digest(&nist256p1,
                                     session.public_key,
                                     buffer,
                                     hash);

    return (status == 0);
};

void append_signature(uint8_t *payload, uint8_t payload_length, uint8_t
*signature_details) {
    uint8_t hash[32] = {0};
    sha256_Raw(payload, payload_length, hash);
    auth_data_t signed_data = atecc_sign(hash);
    uint8_t offset = 0;
    memcpy(signature_details, signed_data.signature, SIGNATURE_SIZE);
    offset += SIGNATURE_SIZE;
    memcpy(signature_details + offset, signed_data.postfix1, POSTFIX1_SIZE);
    offset += POSTFIX1_SIZE;
    memcpy(signature_details + offset, signed_data.postfix2, POSTFIX2_SIZE);
    offset += POSTFIX2_SIZE;
}

void byte_array_to_session_message(uint8_t *data_array, uint16_t msg_size,
                                   Message *msg) {
    msg->message_size = msg_size - SIGNATURE_SIZE;
    msg->message = (uint8_t *) malloc(msg->message_size);
    memcpy(msg->message, data_array, msg->message_size);
    memcpy(msg->signature, data_array + msg->message_size,
           SIGNATURE_SIZE);
}

uint8_t session_message_to_byte_array(Message message, uint8_t *data_array) {
    uint8_t data_array_size = 0;
    memcpy(data_array, message.message, message.message_size);
    data_array_size += message.message_size;
    memcpy(data_array + data_array_size, message.signature, SIGNATURE_SIZE);
    data_array_size += SIGNATURE_SIZE;
    memcpy(data_array + data_array_size, message.postfix1, POSTFIX1_SIZE);
    data_array_size += POSTFIX1_SIZE;
    memcpy(data_array + data_array_size, message.postfix2, POSTFIX2_SIZE);
    data_array_size += POSTFIX2_SIZE;

    return data_array_size;
}

void session_pre_init(uint8_t *session_details_data_array) {
    random_generate(session.device_random, DEVICE_RANDOM_SIZE);
    derive_public_key();

    get_device_serial();
    memcpy(session.device_id, atecc_data.device_serial, DEVICE_SERIAL_SIZE);

    uint8_t session_details_data_array_size = 0;

    memcpy(session_details_data_array,
           session.device_random,
           DEVICE_RANDOM_SIZE);
    session_details_data_array_size += DEVICE_RANDOM_SIZE;

    memcpy(session_details_data_array + session_details_data_array_size,
           session.device_id,
           DEVICE_SERIAL_SIZE);
    session_details_data_array_size += DEVICE_SERIAL_SIZE;

    // Payload: Device Random + Device Id 
    append_signature(session_details_data_array,
                     session_details_data_array_size,
                     session_details_data_array
                         + session_details_data_array_size);
}

bool session_init(uint8_t *session_init_details, uint8_t
*verification_details) {
    // Message contains: Session Random + Session Age
    // Signature Payload: Session Random + Session Age + Device Id + Device Random
    uint8_t offset = 0;
    memcpy(session.session_random,
           session_init_details,
           SESSION_RANDOM_SIZE);
    offset += SESSION_RANDOM_SIZE;

    memcpy(&session.session_age, session_init_details + offset, sizeof
        (session.session_age));
    offset += sizeof(session.session_age);

    uint8_t payload[SESSION_RANDOM_SIZE + sizeof(session.session_age) +
        DEVICE_SERIAL_SIZE + DEVICE_RANDOM_SIZE];

    size_t payload_length = 0;
    memcpy(payload, session_init_details, offset);
    payload_length += offset;
    memcpy(payload + payload_length, session.device_id,
           DEVICE_SERIAL_SIZE);
    payload_length += DEVICE_SERIAL_SIZE;
    memcpy(payload + payload_length, session.device_random,
           DEVICE_RANDOM_SIZE);
    payload_length += DEVICE_RANDOM_SIZE;

    if (!verify_session_signature(payload,
                                  payload_length,
                                  session_init_details + offset)) {
        return false;
    }

    derive_session_id();

    // Verification details: Device Id + Signature + Postfix1 + Postfix2
    memcpy(verification_details, session.device_id, DEVICE_SERIAL_SIZE);
    offset = DEVICE_SERIAL_SIZE;
    append_signature(payload, payload_length,
                     verification_details + offset);

    return true;
}
