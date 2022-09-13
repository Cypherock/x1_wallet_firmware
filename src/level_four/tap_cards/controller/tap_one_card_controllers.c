/**
 * @file    tap_one_card_controllers.c
 * @author  Cypherock X1 Team
 * @brief   Tap one card controller.
 *          This file contains the implementation of the tap one card controllers.
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
#include "apdu.h"
#include "communication.h"
#include "constant_texts.h"
#include "controller_main.h"
#include "controller_tap_cards.h"
#include "flash_api.h"
#include "nfc.h"
#include "tasks.h"
#include "ui_instruction.h"
#include "controller_tap_cards.h"
#include "pow_utilities.h"
#include "ui_delay.h"
#include "utils.h"


extern char card_id_fetched[];
extern char card_version[];
bool no_wallet_on_cards = false;

#if X1WALLET_MAIN
static void _handle_wallet_fetch_success(uint8_t *recv_apdu, uint8_t recv_len);
#endif

void tap_a_card_and_sync_controller()
{
#if X1WALLET_MAIN
    switch (flow_level.level_three) {
    case TAP_ONE_CARD_TAP_A_CARD_FRONTEND:
        tap_card_data.lvl3_retry_point = TAP_ONE_CARD_TAP_A_CARD_FRONTEND;
        tap_card_data.lvl4_retry_point = 1;
        tap_card_data.desktop_control = false;
        tap_card_data.tapped_card = 0;
        flow_level.level_three = TAP_ONE_CARD_TAP_A_CARD_BACKEND;
        break;
    case TAP_ONE_CARD_TAP_A_CARD_BACKEND: {
        uint8_t recv_apdu[400] = {0};
        uint16_t recv_len = 236;

        tap_card_data.retries = 5;
        while (1) {
            tap_card_data.acceptable_cards = 15;
            memcpy(tap_card_data.family_id, get_family_id(), FAMILY_ID_SIZE);
            tap_card_data.tapped_card = 0;
            if (!tap_card_applet_connection())
                break;

            tap_card_data.status = nfc_list_all_wallet(recv_apdu, &recv_len);
            if (tap_card_data.status == SW_NO_ERROR || tap_card_data.status == SW_RECORD_NOT_FOUND) {
                no_wallet_on_cards = false;
                flash_delete_all_wallets();
                if (tap_card_data.status == SW_NO_ERROR)
                    _handle_wallet_fetch_success(recv_apdu, recv_len);
                else
                    no_wallet_on_cards = true;
                flow_level.level_three = TAP_ONE_CARD_SUCCESS_MESSAGE;
                buzzer_start(BUZZER_DURATION);
                lv_obj_clean(lv_scr_act());
                break;
            } else if (tap_card_handle_applet_errors()) {
                break;
            }
        }
    } break;
    case TAP_ONE_CARD_SUCCESS_MESSAGE:
        reset_flow_level();
        counter.level = LEVEL_THREE;
        flow_level.level_one = LEVEL_TWO_ADVANCED_SETTINGS;
        flow_level.level_two = LEVEL_THREE_SYNC_SELECT_WALLET;
        break;
    default:
        reset_flow_level();
    }
#endif
}

void controller_read_card_id()
{
    switch (flow_level.level_three) {
        case TAP_ONE_CARD_TAP_A_CARD_FRONTEND:
            flow_level.level_three = TAP_ONE_CARD_TAP_A_CARD_BACKEND;
            break;
        case TAP_ONE_CARD_TAP_A_CARD_BACKEND:
            tap_card_data.retries = 5;
            while (1) {
                uint8_t version[CARD_VERSION_SIZE];
                // todo log
                nfc_select_card(); //Stuck here until card is detected
                // todo log
                uint8_t no_restrictions[6] = {DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH};
                uint8_t all_cards = 15;
                tap_card_data.status = nfc_select_applet(no_restrictions, &all_cards, version, NULL, NULL);

                if (tap_card_data.status == SW_NO_ERROR) {
                    no_restrictions[CARD_ID_SIZE - 1] = all_cards ^ 15;
                    byte_array_to_hex_string(no_restrictions, CARD_ID_SIZE, card_id_fetched, 2 * CARD_ID_SIZE + 1);
                    byte_array_to_hex_string(version, CARD_VERSION_SIZE, card_version, 2 * CARD_VERSION_SIZE + 1);
                    flow_level.level_three = TAP_ONE_CARD_SUCCESS_MESSAGE;
                    break;
                } else if (tap_card_handle_applet_errors()) {
                    break;
                }
            }
            buzzer_start(BUZZER_DURATION);
            lv_obj_clean(lv_scr_act());
            break;
        case TAP_ONE_CARD_SUCCESS_MESSAGE:
        default:
            counter.level = LEVEL_TWO;
            flow_level.level_two = 1;
            flow_level.level_three = 1;
            break;
    }
}

void controller_update_card_id()
{
    switch (flow_level.level_three) {
        case 1:
            flow_level.level_three++;
            break;

        case 2:
            flow_level.level_three++;
            break;

        case 3: {
            uint8_t send_apdu[10] = {0x00, 0xC7, 0x00, 0x00, 0x05};
            hex_string_to_byte_array(flow_level.screen_input.input_text, 2 * CARD_ID_SIZE, send_apdu + 5);

            uint8_t recv_apdu[255];
            uint16_t recv_len = 236;
            while (1) {
                // todo log
                nfc_select_card(); //Stuck here until card is detected
                // todo log
                uint8_t no_restrictions[6] = {DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH, DEFAULT_VALUE_IN_FLASH};
                uint8_t all_cards = 15;
                nfc_select_applet(no_restrictions, &all_cards, NULL, NULL, NULL);

                if (nfc_exchange_apdu(send_apdu, sizeof(send_apdu), recv_apdu, &recv_len) == STM_SUCCESS) {
                    flow_level.level_three++;
                    break;
                }
            }
            lv_obj_clean(lv_scr_act());
        }
        break;
        case 4:
            reset_flow_level();
            break;
        default:
            break;
    }
}

#if X1WALLET_MAIN
static void _handle_wallet_fetch_success(uint8_t *recv_apdu, uint8_t recv_len)
{
    int i = 0;
    for (; i < recv_apdu[0]; i++) {
        Flash_Wallet wallet;
        wallet.state = VALID_WALLET_WITHOUT_DEVICE_SHARE;
        wallet.wallet_info = recv_apdu[56 * i + 2];
        wallet.is_wallet_locked = recv_apdu[56 * i + 4];
        wallet.cards_states = 15; //assuming if person is syncing then all cards have share
        memcpy(wallet.wallet_name, &recv_apdu[56 * i + 7], recv_apdu[56 * i + 6] /*16 always*/);
        memcpy(wallet.wallet_id, &recv_apdu[56 * i + 25], recv_apdu[56 * i + 24]);

        if (wallet.is_wallet_locked) {
            // Wallet is locked, get challenge from card for this wallet
            uint8_t target[SHA256_SIZE], random_number[POW_RAND_NUMBER_SIZE], max_tries = 8;

            while (max_tries > 0) {
                ISO7816 status_word = nfc_get_challenge(wallet.wallet_name, target, random_number);

                if (status_word == SW_NO_ERROR) {
                    memcpy(wallet.challenge.target, target, SHA256_SIZE);
                    memcpy(wallet.challenge.random_number, random_number, POW_RAND_NUMBER_SIZE);
                    memzero(wallet.challenge.nonce, POW_NONCE_SIZE);
                    wallet.challenge.card_locked = 15 ^ tap_card_data.acceptable_cards;
                    pow_get_approx_time_in_secs(target, &wallet.challenge.time_to_unlock_in_secs);
                    break;
                }

                max_tries--;
            }
        }

        uint32_t dummy;
        add_wallet_to_flash(&wallet, &dummy);
    }
}
#endif