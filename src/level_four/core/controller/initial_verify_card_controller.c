/**
 * @file    initial_verify_card_controller.c
 * @author  Cypherock X1 Team
 * @brief   Verify card next controller (initial).
 *          Handles post event (only next events) operations for card verification flow initiated by desktop app.
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
#include "communication.h"
#include "controller_level_four.h"
#include "nfc.h"
#include "ui_instruction.h"
#include "buzzer.h"
#include "bip32.h"
#include "curves.h"
#include "base58.h"
#include "nist256p1.h"
#include "flash_api.h"

#define CARD_AUTH_RAND_NUMBER_SIZE 32

#if X1WALLET_INITIAL

static uint8_t family_id[FAMILY_ID_SIZE];
static uint8_t signature[ECDSA_SIGNATURE_SIZE];
static uint16_t length;

uint8_t auth_card_number = 0;

static void _tap_card_backend(uint8_t card_number);
static void _handle_pair_card_success(uint8_t *session_nonce, uint8_t *card_pairing_data);

/* Serial Number = Family ID appended with Card Number
 * Return serial number
 */
static uint8_t get_card_serial_number(uint8_t family_id[], uint8_t cards_state, uint8_t serial_number_out[])
{
    uint8_t card_number = 1;

    memcpy(serial_number_out, family_id, FAMILY_ID_SIZE);

    cards_state = 15 ^ cards_state; // If 3rd card is tapped 15 ^ cards_state will be (0100)b

    while (cards_state != 1) {
        cards_state = cards_state >> 1;
        card_number++;
    }

    memcpy(serial_number_out + FAMILY_ID_SIZE, &card_number, 1);

    return FAMILY_ID_SIZE + 1;
}
#endif

void initial_verify_card_controller()
{
#if X1WALLET_INITIAL
    ISO7816 status_word;
    uint8_t retries = 5;
//    set_task_handler(&contunue_loop);
    set_instant_abort(&abort_from_desktop);
    set_abort_now(&_abort_);

    switch (flow_level.level_three) {

    case VERIFY_CARD_START_MESSAGE:
        transmit_one_byte_confirm(START_CARD_AUTH);
        flow_level.level_three = VERIFY_CARD_ESTABLISH_CONNECTION_FRONTEND;
        break;

    case VERIFY_CARD_ESTABLISH_CONNECTION_FRONTEND:
        flow_level.level_three = VERIFY_CARD_ESTABLISH_CONNECTION_BACKEND;
        break;

    case VERIFY_CARD_ESTABLISH_CONNECTION_BACKEND:

        memset(family_id, DEFAULT_VALUE_IN_FLASH, FAMILY_ID_SIZE);

        while (1) {
            if (nfc_select_card() != STM_SUCCESS)       //Stuck here until card is detected
                return;

            uint8_t cards_state = (1 << (auth_card_number - 1));
            status_word = nfc_select_applet(family_id, &cards_state, NULL, NULL); // Family ID of card tapped is returned
            cards_state = 15 - (1 << (auth_card_number - 1));
            if (status_word == SW_NO_ERROR) {
                length = get_card_serial_number(family_id, cards_state, signature /*Serial Number Out*/);
                status_word = nfc_ecdsa(signature, &length);

                if (status_word == SW_NO_ERROR) {
                    uint8_t data_out[ECDSA_SIGNATURE_SIZE + FAMILY_ID_SIZE + 1];

                    memcpy(data_out, signature, ECDSA_SIGNATURE_SIZE);
                    get_card_serial_number(family_id, cards_state, data_out + ECDSA_SIGNATURE_SIZE);

                    transmit_data_to_app(SEND_SIGNATURE_TO_APP, data_out, ECDSA_SIGNATURE_SIZE + FAMILY_ID_SIZE + 1);
                    flow_level.level_three = VERIFY_CARD_FETCH_RANDOM_NUMBER;
                    break;
                } else {
                    if (!(--retries)) {
                        LOG_ERROR("err (0x%02X%02X)\n", status_word >> 8, status_word & 0xff);
                        instruction_scr_destructor();
                        mark_error_screen(ui_text_unknown_error_contact_support);
                        reset_flow_level();
                        flow_level.level_one = 6;   // take to get-started screen
                        transmit_one_byte_reject(START_CARD_AUTH);
                        break;
                    }
                }
            } else if (status_word == SW_CONDITIONS_NOT_SATISFIED) {
                mark_error_screen(ui_text_invalid_card_tap_card[auth_card_number - 1]);
                instruction_scr_destructor();
                flow_level.level_three = VERIFY_CARD_ESTABLISH_CONNECTION_FRONTEND;
                break;
            } else {
                if (!(--retries)) {
                    LOG_ERROR("err (0x%02X%02X)\n", status_word >> 8, status_word & 0xff);
                    instruction_scr_destructor();
                    mark_error_screen(ui_text_unknown_error_contact_support);
                    reset_flow_level();
                    flow_level.level_one = 6;   // take to get-started screen
                    transmit_one_byte_reject(START_CARD_AUTH);
                    break;
                }
            }
        }
        buzzer_start(150);
        break;

    case VERIFY_CARD_FETCH_RANDOM_NUMBER: {
        En_command_type_t msg_type;
        uint8_t *data_array;
        uint16_t msg_size;
        get_usb_msg(&msg_type, &data_array, &msg_size);

        if(msg_type == APP_SEND_RAND_NUM){
            memcpy(signature, data_array, CARD_AUTH_RAND_NUMBER_SIZE);
            flow_level.level_three = VERIFY_CARD_SIGN_RANDOM_NUMBER_FRONTEND;
        } else {
            flow_level.level_three = VERIFY_CARD_FAILED;
        }
        clear_message_received_data();
    } break;

    case VERIFY_CARD_SIGN_RANDOM_NUMBER_FRONTEND:
        flow_level.level_three = VERIFY_CARD_SIGN_RANDOM_NUMBER_BACKEND;
        break;

    case VERIFY_CARD_SIGN_RANDOM_NUMBER_BACKEND: {
        length = CARD_AUTH_RAND_NUMBER_SIZE;
        while (1) {
            if (nfc_select_card() != STM_SUCCESS)       //Stuck here until card is detected
                return;

            uint8_t cards_state = (1 << (auth_card_number - 1));
            status_word = nfc_select_applet(family_id, &cards_state, NULL, NULL);
            cards_state = 15 - (1 << (auth_card_number - 1));
            if (status_word == SW_NO_ERROR) {
                status_word = nfc_ecdsa(signature, &length);

                if (status_word == SW_NO_ERROR) {

                    uint8_t data_out[ECDSA_SIGNATURE_SIZE + FAMILY_ID_SIZE + 1];

                    memcpy(data_out, signature, ECDSA_SIGNATURE_SIZE);
                    get_card_serial_number(family_id, cards_state, data_out + ECDSA_SIGNATURE_SIZE);

                    transmit_data_to_app(SIGNED_CHALLENGE, data_out, ECDSA_SIGNATURE_SIZE + FAMILY_ID_SIZE + 1);
                    flow_level.level_three++;
                    break;
                } else {
                    if (!(--retries)) {
                        LOG_ERROR("err (0x%02X%02X)\n", status_word >> 8, status_word & 0xff);
                        instruction_scr_destructor();
                        mark_error_screen(ui_text_unknown_error_contact_support);
                        reset_flow_level();
                        flow_level.level_one = 6;   // take to get-started screen
                        transmit_one_byte_reject(START_CARD_AUTH);
                        break;
                    }
                }
            } else if (status_word == SW_CONDITIONS_NOT_SATISFIED) {
                mark_error_screen(ui_text_invalid_card_tap_card[auth_card_number - 1]);
                instruction_scr_destructor();
                flow_level.level_three = VERIFY_CARD_SIGN_RANDOM_NUMBER_FRONTEND;
                break;
            } else {
                if (!(--retries)) {
                    LOG_ERROR("err (0x%02X%02X)\n", status_word >> 8, status_word & 0xff);
                    instruction_scr_destructor();
                    mark_error_screen(ui_text_unknown_error_contact_support);
                    reset_flow_level();
                    flow_level.level_one = 6;   // take to get-started screen
                    transmit_one_byte_reject(START_CARD_AUTH);
                    break;
                }
            }
        }
        buzzer_start(150);
    } break;

    case VERIFY_CARD_AUTH_STATUS: {
        En_command_type_t msg_type;
        uint8_t *data_array;
        uint16_t msg_size;
        get_usb_msg(&msg_type, &data_array, &msg_size);

        if (msg_type == STATUS_PACKET && data_array[0] == STATUS_CMD_SUCCESS) {
            flow_level.level_three = VERIFY_CARD_PAIR_FRONTEND;
        } else {
            flow_level.level_three = VERIFY_CARD_FAILED;
        }
        clear_message_received_data();
    } break;

    case VERIFY_CARD_PAIR_FRONTEND: {
        flow_level.level_three = VERIFY_CARD_PAIR_BACKEND;
    } break;

    case VERIFY_CARD_PAIR_BACKEND: {
        _tap_card_backend(auth_card_number);
    } break;

    case VERIFY_CARD_FINAL_MESSAGE: {
        instruction_scr_destructor();
        reset_flow_level();
        flow_level.level_one = 7;
    } break;

    case VERIFY_CARD_FAILED:
        reset_flow_level();
        flow_level.level_one = 6;
        break;

    default:
        break;
    }
}

static void _tap_card_backend(uint8_t card_number)
{
    uint8_t card_pairing_data[128] = {0}, pairing_data_len = 44, retries = 5;
    uint8_t digest[64], sig[64], session_nonce[32], card_key_id[4];
    ISO7816 status_word;

    random_generate(session_nonce, sizeof(session_nonce));
    memcpy(card_pairing_data, get_perm_self_key_id(), 4);
    memcpy(card_pairing_data + 4, session_nonce, sizeof(session_nonce));
    memcpy(card_pairing_data + 36, get_perm_self_key_path(), 8);

    sha256_Raw(card_pairing_data, pairing_data_len, digest);
    ecdsa_sign_digest(&nist256p1, get_priv_key(), digest, sig, NULL, NULL);
    atecc_nfc_sign_hash(digest, sig);
    pairing_data_len += ecdsa_sig_to_der(sig, card_pairing_data + pairing_data_len);

    while (1) {
        if (nfc_select_card() != STM_SUCCESS)       //Stuck here until card is detected
            return;

        uint8_t cards_state = (1 << (auth_card_number - 1));
        status_word = nfc_select_applet(family_id, &cards_state, NULL, card_key_id);
        if (status_word == SW_NO_ERROR) {
            if (is_paired(card_key_id) > -1) {
                buzzer_start(BUZZER_DURATION);
                transmit_one_byte_confirm(START_CARD_AUTH);
                instruction_scr_destructor();
                flow_level.level_three++;
                return;
            }
            status_word = nfc_pair(card_pairing_data, &pairing_data_len);

            if (status_word == SW_NO_ERROR) {
                transmit_one_byte_confirm(START_CARD_AUTH);
                buzzer_start(BUZZER_DURATION);
                _handle_pair_card_success(session_nonce, card_pairing_data);
                instruction_scr_destructor();
                break;
            } else {
                LOG_ERROR("err (0x%02X%02X)", status_word >> 8, status_word & 0xff);
                if (!(--retries)) {
                    buzzer_start(BUZZER_DURATION);
                    instruction_scr_destructor();
                    mark_error_screen(ui_text_unknown_error_contact_support);
                    reset_flow_level();
                    flow_level.level_one = 6;   // take to get-started screen
                    transmit_one_byte_reject(START_CARD_AUTH);
                    break;
                }
            }
        } else if (status_word == SW_CONDITIONS_NOT_SATISFIED) {
            buzzer_start(BUZZER_DURATION);
            mark_error_screen(ui_text_invalid_card_tap_card[auth_card_number - 1]);
            instruction_scr_destructor();
            flow_level.level_three = VERIFY_CARD_PAIR_FRONTEND;
            break;
        } else {
            LOG_ERROR("err (0x%02X%02X)", status_word >> 8, status_word & 0xff);
            if (!(--retries)) {
                buzzer_start(BUZZER_DURATION);
                instruction_scr_destructor();
                mark_error_screen(ui_text_unknown_error_contact_support);
                reset_flow_level();
                flow_level.level_one = 6;   // take to get-started screen
                transmit_one_byte_reject(START_CARD_AUTH);
                break;
            }
        }
    }
}

static void _handle_pair_card_success(uint8_t *session_nonce, uint8_t *card_pairing_data)
{
    HDNode guest_card_node;
    SHA512_CTX ctx;
    uint8_t keystore_index, digest[32] = {0}, buffer[64] = {0};
    uint8_t public_key_uncompressed[65] = {0};
    uint32_t index;
    char xpub[112] = {'\0'};

    base58_encode_check(get_card_root_xpub(),
                        FS_KEYSTORE_XPUB_LEN,
                        nist256p1_info.hasher_base58, xpub, 112);
    hdnode_deserialize_public((char *) xpub, 0x0488b21e, NIST256P1_NAME, &guest_card_node, NULL);

    index = read_be(card_pairing_data + 36);
    hdnode_public_ckd(&guest_card_node, index);

    index = read_be(card_pairing_data + 40);
    hdnode_public_ckd(&guest_card_node, index);

    der_to_sig(card_pairing_data + 44, buffer);
    sha256_Raw(card_pairing_data, 44, digest);
    if (ecdsa_verify_digest(&nist256p1, guest_card_node.public_key, buffer, digest)) {
    	reset_flow_level();
    	mark_error_screen(ui_text_cannot_verify_card_contact_support);
    	return;
    }
    keystore_index =  auth_card_number - 1;
    ecdsa_uncompress_pubkey(&nist256p1, guest_card_node.public_key, public_key_uncompressed);
    atecc_nfc_ecdh(&public_key_uncompressed[1], card_pairing_data + 45);
    sha512_Init(&ctx);
    sha512_Update(&ctx, card_pairing_data + 45, 32);
    sha512_Update(&ctx, card_pairing_data + 4, 32);
    sha512_Update(&ctx, session_nonce, 32);
    sha512_Final(&ctx, buffer);
    set_keystore_pairing_key(keystore_index, buffer, sizeof(buffer), FLASH_SAVE_LATER);
    set_keystore_key_id(keystore_index, card_pairing_data, 4, FLASH_SAVE_LATER);
    set_keystore_used_status(keystore_index, 1, FLASH_SAVE_NOW);

    flow_level.level_three++;
#endif
}
