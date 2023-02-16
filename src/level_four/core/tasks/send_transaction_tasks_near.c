/**
 * @file    send_transaction_tasks_near.c
 * @author  Cypherock X1 Team
 * @brief   Send transaction task for NEAR
 *          This file contains the implementation of the send transaction task for NEAR.
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
#include "controller_level_four.h"
#include "tasks_tap_cards.h"
#include "byte_utilities.h"
#include "ui_screens.h"

extern char* ALPHABET;
extern char* ALPHA_NUMERIC;
extern char* NUMBERS;
extern char* PASSPHRASE;

extern lv_task_t* success_task;
extern lv_task_t* timeout_task;

void send_transaction_tasks_near() {

    switch (flow_level.level_three) {

    case SEND_TXN_VERIFY_COIN_NEAR: {
        instruction_scr_init("", NULL);
        instruction_scr_change_text(ui_text_processing, true);
        BSP_DelayMs(DELAY_SHORT);
        mark_event_over();
    } break;

    case SEND_TXN_UNSIGNED_TXN_WAIT_SCREEN_NEAR: {
        mark_event_over();
    } break;

    case SEND_TXN_VERIFY_TXN_NONCE_NEAR: {
        char nonce_hex_str[NEAR_NONCE_SIZE_BYTES * 2 + 1] = { '\0' };
        uint8_t nonce_dec_str[NEAR_NONCE_SIZE_BYTES * 3] = { 0 };
        uint16_t nonce_dec_len = sizeof(nonce_dec_str), nonce_hex_len;
        int index, offset;
        nonce_hex_len = byte_array_to_hex_string(near_utxn.nonce, sizeof(near_utxn.nonce),
            nonce_hex_str, sizeof(nonce_hex_str));
        convertbase16tobase10(nonce_hex_len - 1, nonce_hex_str,
            nonce_dec_str, nonce_dec_len);
        // Loop till 2nd last index; to handle "0" nonce value by generalising for 1 digit nonce values
        for (index = 0; index < nonce_dec_len - 1; index++)
            if (nonce_dec_str[index] != 0) break;
        for (offset = index; index < nonce_dec_len; index++)
            nonce_dec_str[index - offset] = nonce_dec_str[index] + '0';
        ASSERT((index > offset) && ((index - offset) < nonce_dec_len));
        nonce_dec_str[index - offset] = '\0';
        ui_scrollable_page(ui_text_verify_nonce, (char*)nonce_dec_str, MENU_SCROLL_HORIZONTAL, false);
        // address_scr_init(ui_text_verify_nonce, (char*)nonce_dec_str, false);
    } break;

    case SEND_TXN_VERIFY_SENDER_ADDRESS_NEAR: {
        instruction_scr_destructor();
        char top_heading[45];
        char display[110];

        snprintf(top_heading, sizeof(top_heading), "%s", ui_text_verify_create_from);
        snprintf(display, sizeof(display), "%.*s", (int)near_utxn.signer_id_length, near_utxn.signer);
        ui_scrollable_page(top_heading, display, MENU_SCROLL_HORIZONTAL, false);
        // address_scr_init(top_heading, display, true);
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_NEAR: {
        instruction_scr_destructor();
        char top_heading[45];
        char display[110];

        if (near_utxn.actions_type == NEAR_ACTION_TRANSFER) {
            snprintf(top_heading, sizeof(top_heading), "%s", ui_text_verify_address);
            snprintf(display, sizeof(display), "%.*s", (int)near_utxn.receiver_id_length, near_utxn.receiver);
        }
        else if (near_utxn.actions_type == NEAR_ACTION_FUNCTION_CALL) {
            snprintf(top_heading, sizeof(top_heading), "%s",ui_text_verify_new_account_id);
            char account[NEAR_ACC_ID_MAX_LEN + 1] = { 0 };
            size_t account_length = near_get_new_account_id_from_fn_args((const char*)near_utxn.action.fn_call.args, near_utxn.action.fn_call.args_length, account);
            snprintf(display, sizeof(display), "%.*s", (int)account_length, account);
        }
        ui_scrollable_page(top_heading, display, MENU_SCROLL_HORIZONTAL, false);
        // address_scr_init(top_heading, display, true);
    } break;

    case SEND_TXN_CALCULATE_AMOUNT_NEAR: {
        instruction_scr_init("", NULL);
        instruction_scr_change_text(ui_text_processing, true);
        BSP_DelayMs(DELAY_SHORT);
        mark_event_over();
    }break;

    case SEND_TXN_VERIFY_RECEIPT_AMOUNT_NEAR: {
        char amount_string[40] = { '\0' }, amount_decimal_string[30] = { '\0' };
        char display[110] = { '\0' };
        memzero(amount_string, sizeof(amount_string));
        if (near_utxn.actions_type == NEAR_ACTION_TRANSFER) {
            byte_array_to_hex_string(near_utxn.action.transfer.amount, 16, amount_string, sizeof(amount_string));
        } else if (near_utxn.actions_type == NEAR_ACTION_FUNCTION_CALL) {
            byte_array_to_hex_string(near_utxn.action.fn_call.deposit, 16, amount_string, sizeof(amount_string));
        }else{
            //TODO: add error handling / handling for other actions
        }
        if(!convert_byte_array_to_decimal_string(32,var_send_transaction_data.transaction_metadata.eth_val_decimal[0], amount_string, amount_decimal_string, sizeof(amount_decimal_string))) break;
        instruction_scr_destructor();
        snprintf(display, sizeof(display), "%s\n%s", amount_decimal_string, get_coin_symbol(BYTE_ARRAY_TO_UINT32(var_send_transaction_data.transaction_metadata.coin_index),0));
        ui_scrollabe_page(ui_heading_verify_amount, display, MENU_SCROLL_HORIZONTAL, false);
        // confirm_scr_init(display);
    } break;

    case SEND_TXN_VERIFY_RECEIPT_FEES_NEAR: {
        char amount_string[40] = { '\0' }, amount_decimal_string[30] = { '\0' };
        char display[110] = { '\0' };
        byte_array_to_hex_string((const uint8_t*)var_send_transaction_data.transaction_metadata.transaction_fees, 8, amount_string, sizeof(amount_string));
        uint8_t decimal_val_s[32 * 3] = { 0 };
        if (sizeof(decimal_val_s) / sizeof(decimal_val_s[0]) > UINT8_MAX) {
            LOG_ERROR("0xxx#");
            break;
        }
        if(!convert_byte_array_to_decimal_string(16,var_send_transaction_data.transaction_metadata.eth_val_decimal[0]-4, amount_string, amount_decimal_string, sizeof(amount_decimal_string))) break;
        instruction_scr_destructor();
        if (near_utxn.actions_type == NEAR_ACTION_TRANSFER) {
            snprintf(display, sizeof(display), "%s\n%s", amount_decimal_string, get_coin_symbol(BYTE_ARRAY_TO_UINT32(var_send_transaction_data.transaction_metadata.coin_index),0));
        } else if (near_utxn.actions_type == NEAR_ACTION_FUNCTION_CALL) {
            snprintf(display, sizeof(display), "%s\n%s", "0.0012", get_coin_symbol(BYTE_ARRAY_TO_UINT32(var_send_transaction_data.transaction_metadata.coin_index),0));
        }else{
            //TODO: add error handling / handling for other actions
        }
        ui_scrollabe_page(ui_heading_verify_transaction_fee, display, MENU_SCROLL_HORIZONTAL, false);
        // confirm_scr_init(display);
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD_NEAR: {
        mark_event_over();
    } break;

    case SEND_TXN_ENTER_PASSPHRASE_NEAR: {
        if (!WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
            flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD_NEAR;
            break;
        }
        input_text_init(
            PASSPHRASE,
            ui_text_enter_passphrase,
            0,
            DATA_TYPE_PASSPHRASE,
            64);

    } break;

    case SEND_TXN_CONFIRM_PASSPHRASE_NEAR: {
        char display[65];
        snprintf(display, sizeof(display), "%s", flow_level.screen_input.input_text);
        ui_scrollable_page(ui_text_confirm_passphrase, display, MENU_SCROLL_HORIZONTAL, false);
        // address_scr_init(ui_text_confirm_passphrase, display, false);
        memzero(display, sizeof(display));
    } break;

    case SEND_TXN_CHECK_PIN_NEAR: {
        mark_event_over();
    } break;

    case SEND_TXN_ENTER_PIN_NEAR: {
        if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
            flow_level.level_three = SEND_TXN_CHECK_PIN_NEAR;
            break;
        }
        input_text_init(
            ALPHA_NUMERIC,
            ui_text_enter_pin,
            4,
            DATA_TYPE_PIN,
            8);

    } break;

    case SEND_TXN_TAP_CARD_NEAR: {
        tap_threshold_cards_for_reconstruction();
    } break;

    case SEND_TXN_TAP_CARD_SEND_CMD_NEAR: {
        instruction_scr_init("", NULL);
        instruction_scr_change_text(ui_text_processing, true);
        BSP_DelayMs(DELAY_SHORT);
        mark_event_over();
    } break;

    case SEND_TXN_READ_DEVICE_SHARE_NEAR: {
        mark_event_over();
    } break;

    case SEND_TXN_SIGN_TXN_NEAR: {
        instruction_scr_destructor();
        mark_event_over();
    } break;

    default:
        break;
    }

}