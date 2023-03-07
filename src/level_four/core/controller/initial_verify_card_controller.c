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
#include "controller_tap_cards.h"
#if USE_SIMULATOR == 0
#include "stm32l4xx_it.h"
#endif

#define CARD_AUTH_RAND_NUMBER_SIZE 32

#if X1WALLET_INITIAL

static uint8_t signature[ECDSA_SIGNATURE_SIZE] = {0};
static uint16_t length = 0;

/**
 * @brief Track the Family-ID of the card during one device session.
 * @details Initialized to allow any card for a new session.
 * The actual Family-ID is read from the card during the first session (at VERIFY_CARD_FETCH_RANDOM_NUMBER in
 * get_card_serial_number()).
 *
 * @see VERIFY_CARD_FETCH_RANDOM_NUMBER, get_card_serial_number()
 */
static uint8_t cyi_verify_fid[FAMILY_ID_SIZE] = {DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH};

uint8_t auth_card_number = 0;

static void _tap_card_backend(uint8_t card_number);

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

    switch (flow_level.level_three) {

    case VERIFY_CARD_START_MESSAGE:
        tap_card_data.desktop_control = true;
        tap_card_data.active_cmd_type = START_CARD_AUTH;
        flow_level.level_three = VERIFY_CARD_ESTABLISH_CONNECTION_FRONTEND;
        break;

    case VERIFY_CARD_ESTABLISH_CONNECTION_FRONTEND:
        tap_card_data.retries = 5;
        flow_level.level_three = VERIFY_CARD_ESTABLISH_CONNECTION_BACKEND;
        break;

    case VERIFY_CARD_ESTABLISH_CONNECTION_BACKEND:

        memcpy(tap_card_data.family_id, cyi_verify_fid, FAMILY_ID_SIZE);

        tap_card_data.lvl3_retry_point = VERIFY_CARD_ESTABLISH_CONNECTION_FRONTEND;
        while (1) {
            tap_card_data.acceptable_cards = (1 << (auth_card_number - 1));
            tap_card_data.tapped_card = 0;
            if (!tap_card_applet_connection()) {
                if (counter.level == LEVEL_ONE)
                    flow_level.level_one = 6;   // take to get-started screen
                break;
            }
            length = get_card_serial_number(tap_card_data.family_id, 15 ^ tap_card_data.tapped_card, signature /*Serial Number Out*/);
            tap_card_data.status = nfc_ecdsa(signature, &length);

            if (tap_card_data.status == SW_NO_ERROR) {
                uint8_t data_out[ECDSA_SIGNATURE_SIZE + CARD_ID_SIZE];

                memcpy(data_out, signature, ECDSA_SIGNATURE_SIZE);
                get_card_serial_number(tap_card_data.family_id, 15 ^ tap_card_data.tapped_card, data_out + ECDSA_SIGNATURE_SIZE);
                transmit_data_to_app(SEND_SIGNATURE_TO_APP, data_out, ECDSA_SIGNATURE_SIZE + CARD_ID_SIZE);
                flow_level.level_three = VERIFY_CARD_FETCH_RANDOM_NUMBER;
                buzzer_start(BUZZER_DURATION);
                break;
            } else if (tap_card_handle_applet_errors()) {
                if (counter.level == LEVEL_ONE)
                    flow_level.level_one = 6;   // take to get-started screen
                break;
            }
        }
        break;

    case VERIFY_CARD_FETCH_RANDOM_NUMBER: {
        En_command_type_t cmd_type;
        uint8_t *data_array;
        uint16_t msg_size = 0;

        if (!get_usb_msg(&cmd_type, &data_array, &msg_size)) return;
        if (cmd_type == STATUS_PACKET && data_array && data_array[0] == 0) {
            flow_level.level_three = VERIFY_CARD_FAILED;
            clear_message_received_data();
            return;
        } else if (cmd_type != APP_SEND_RAND_NUM) {
            comm_reject_invalid_cmd();
            clear_message_received_data();
            return;
        }
        // Save the family id for this session; good point to save after server verification
        memcpy(cyi_verify_fid, tap_card_data.family_id, FAMILY_ID_SIZE);
        memcpy(signature, data_array, CARD_AUTH_RAND_NUMBER_SIZE);
        flow_level.level_three = VERIFY_CARD_SIGN_RANDOM_NUMBER_FRONTEND;
        clear_message_received_data();
    } break;

    case VERIFY_CARD_SIGN_RANDOM_NUMBER_FRONTEND:
        tap_card_data.retries = 5;
        flow_level.level_three = VERIFY_CARD_SIGN_RANDOM_NUMBER_BACKEND;
        break;

    case VERIFY_CARD_SIGN_RANDOM_NUMBER_BACKEND: {
        length = CARD_AUTH_RAND_NUMBER_SIZE;
        tap_card_data.lvl3_retry_point = VERIFY_CARD_SIGN_RANDOM_NUMBER_FRONTEND;
        while (1) {
            tap_card_data.acceptable_cards = (1 << (auth_card_number - 1));
            tap_card_data.tapped_card = 0;
            if (!tap_card_applet_connection()) {
                if (counter.level == LEVEL_ONE)
                    flow_level.level_one = 6;   // take to get-started screen
                break;
            } else {
                tap_card_data.status = nfc_ecdsa(signature, &length);
            }

            if (tap_card_data.status == SW_NO_ERROR) {
                uint8_t data_out[ECDSA_SIGNATURE_SIZE + CARD_ID_SIZE];

                memcpy(data_out, signature, ECDSA_SIGNATURE_SIZE);
                get_card_serial_number(tap_card_data.family_id, 15 ^ tap_card_data.tapped_card, data_out + ECDSA_SIGNATURE_SIZE);
                transmit_data_to_app(SIGNED_CHALLENGE, data_out, ECDSA_SIGNATURE_SIZE + CARD_ID_SIZE);
                buzzer_start(BUZZER_DURATION);

                flow_level.level_three = VERIFY_CARD_AUTH_STATUS;
                break;
            } else if (tap_card_handle_applet_errors()) {
                if (counter.level == LEVEL_ONE)
                    flow_level.level_one = 6;   // take to get-started screen
                break;
            }
        }
    } break;

    case VERIFY_CARD_AUTH_STATUS: {
        En_command_type_t cmd_type;
        uint8_t *data_array;
        uint16_t msg_size = 0;

        if (!get_usb_msg(&cmd_type, &data_array, &msg_size)) return;
        if (cmd_type != STATUS_PACKET) {
            comm_reject_invalid_cmd();
            clear_message_received_data();
            return;
        }

        flow_level.level_three = data_array[0] == STATUS_CMD_SUCCESS ? VERIFY_CARD_PAIR_FRONTEND : VERIFY_CARD_FAILED;
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
        comm_process_complete();
        reset_flow_level();
        flow_level.level_one = 6;
        break;

    default:
        break;
    }
}

static void _tap_card_backend(uint8_t card_number)
{
    uint8_t card_pairing_data[128] = {0}, pairing_data_len = 44;
    uint8_t digest[64] = {0}, sig[65] = {0}, session_nonce[32] = {0};
    uint8_t invalid_self_keyid[8] = {DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH,
                                     DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH};

    random_generate(session_nonce, sizeof(session_nonce));
    memcpy(card_pairing_data, get_perm_self_key_id(), 4);
    memcpy(card_pairing_data + 4, session_nonce, sizeof(session_nonce));
    memcpy(card_pairing_data + 36, get_perm_self_key_path(), 8);
    if (memcmp(get_perm_self_key_path(), invalid_self_keyid, sizeof(invalid_self_keyid)) == 0) {
        /* Device is not provisioned */
        comm_reject_request(START_CARD_AUTH, 0);
        reset_flow_level();
        instruction_scr_destructor();
        mark_error_screen(ui_text_device_compromised);
        return;
    }

    sha256_Raw(card_pairing_data, pairing_data_len, digest);
    LOG_CRITICAL("start pair sign");
    atecc_nfc_sign_hash(digest, sig);
    pairing_data_len += ecdsa_sig_to_der(sig, card_pairing_data + pairing_data_len);
    tap_card_data.retries = 5;

    while (1) {
        tap_card_data.tapped_card = 0;
        tap_card_data.acceptable_cards = (1 << (card_number - 1));
        tap_card_data.lvl3_retry_point = VERIFY_CARD_PAIR_FRONTEND;
        memcpy(tap_card_data.family_id, cyi_verify_fid, FAMILY_ID_SIZE);
        if (!tap_card_applet_connection()) {
            if (counter.level == LEVEL_ONE)
                flow_level.level_one = 6;   // take to get-started screen
            break;
        }
        if (is_paired(tap_card_data.card_key_id) > -1) {
            transmit_one_byte_confirm(START_CARD_AUTH);
            buzzer_start(BUZZER_DURATION);
            instruction_scr_destructor();
            flow_level.level_three++;
            return;
        }
        tap_card_data.status = nfc_pair(card_pairing_data, &pairing_data_len);
        if (tap_card_data.status == SW_NO_ERROR) {
            buzzer_start(BUZZER_DURATION);
            if (U32_READ_BE_ARRAY(get_family_id()) != U32_READ_BE_ARRAY(cyi_verify_fid))
                set_family_id_flash(cyi_verify_fid);
            instruction_scr_change_text(ui_text_remove_card_prompt, true);
            nfc_detect_card_removal();
            handle_pair_card_success(card_number, session_nonce, card_pairing_data);
            transmit_one_byte_confirm(START_CARD_AUTH);
            instruction_scr_destructor();
            break;
        } else if (tap_card_handle_applet_errors()) {
            if (counter.level == LEVEL_ONE)
                flow_level.level_one = 6;   // take to get-started screen
            break;
        }
    }
#endif
}

uint8_t atecc_nfc_sign_hash(const uint8_t *hash, uint8_t *sign){
    atecc_data.retries = DEFAULT_ATECC_RETRIES;

    bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
    NVIC_DisableIRQ(OTG_FS_IRQn);
    do{
        if (atecc_data.status != ATCA_SUCCESS)
            LOG_CRITICAL("PAIR SG: %04x, count:%d", atecc_data.status, DEFAULT_ATECC_RETRIES - atecc_data.retries);
        atcab_init(atecc_data.cfg_atecc608a_iface);
        atecc_data.status = atcab_sign(slot_3_nfc_pair_key, hash, sign);
    }while(atecc_data.status != ATCA_SUCCESS && --atecc_data.retries);
    if(usb_irq_enable_on_entry == true)
        NVIC_EnableIRQ(OTG_FS_IRQn);

    return atecc_data.status;
}

uint8_t atecc_nfc_ecdh(const uint8_t *pub_key, uint8_t *shared_secret){
    uint8_t io_key[IO_KEY_SIZE];
    atecc_data.retries = DEFAULT_ATECC_RETRIES;

    if(get_io_protection_key(io_key) != SUCCESS_)
      return -1;

    bool usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
    NVIC_DisableIRQ(OTG_FS_IRQn);
    do{
        if (atecc_data.status != ATCA_SUCCESS)
            LOG_CRITICAL("ECDH: %04x, count:%d", atecc_data.status, DEFAULT_ATECC_RETRIES - atecc_data.retries);
        atcab_init(atecc_data.cfg_atecc608a_iface);
        atecc_data.status = atcab_ecdh_ioenc(slot_3_nfc_pair_key, pub_key, shared_secret, io_key);
    }while(atecc_data.status != ATCA_SUCCESS && --atecc_data.retries);
    if(usb_irq_enable_on_entry == true)
        NVIC_EnableIRQ(OTG_FS_IRQn);

    return atecc_data.status;
}