/**
 * @file    send_transaction_tasks.c
 * @author  Cypherock X1 Team
 * @brief   Send transaction task for BTC.
 *          This file contains the implementation of the send transaction task for BTC.
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
#include "btc.h"
#include "constant_texts.h"
#include "controller_level_four.h"
#include "tasks_level_four.h"
#include "ui_address.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_message.h"
#include "tasks_tap_cards.h"
#include "segwit_addr.h"

extern char* ALPHABET;
extern char* ALPHA_NUMERIC;
extern char* NUMBERS;
extern char* PASSPHRASE;

extern lv_task_t* timeout_task;

extern int input_index;

void send_transaction_tasks()
{
    switch (flow_level.level_three) {

    case SEND_TXN_VERIFY_COIN: {
        delay_scr_init(ui_text_processing, DELAY_SHORT);
        mark_event_over();
    } break;

    case SEND_TXN_UNSIGNED_TXN_WAIT_SCREEN: {
        mark_event_over();
    } break;

    case SEND_TXN_UNSIGNED_TXN_RECEIVED: {
        mark_event_over();
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS: {
        instruction_scr_destructor();
        char address[64];
        char *hrp;
        uint8_t addr_version;

        if (BYTE_ARRAY_TO_UINT32(var_send_transaction_data.transaction_metadata.coin_index) == BITCOIN)
            hrp = "bc";
        else
            hrp = "tb";
        get_version(0x8000002CU, BYTE_ARRAY_TO_UINT32(var_send_transaction_data.transaction_metadata.coin_index), &addr_version, NULL);
        if (var_send_transaction_data.unsigned_transaction.output[var_send_transaction_data.transaction_confirmation_list_index].script_public_key[0] == 0)
            segwit_addr_encode(address, hrp, 0x00, &var_send_transaction_data.unsigned_transaction.output[var_send_transaction_data.transaction_confirmation_list_index].script_public_key[2],
                               var_send_transaction_data.unsigned_transaction.output[var_send_transaction_data.transaction_confirmation_list_index].script_public_key[1]);
        else
            get_address(addr_version, var_send_transaction_data.unsigned_transaction.output[var_send_transaction_data.transaction_confirmation_list_index].script_public_key, address);
        char top_heading[225];
        char display[70];

        snprintf(top_heading, sizeof(top_heading), ui_text_output_send_to_address, var_send_transaction_data.transaction_confirmation_list_index + 1);
        snprintf(display, sizeof(display), "%s%s", ui_text_20_spaces, address);
        address_scr_init(top_heading, display, true);
        address_scr_focus_next();
    } break;

    case SEND_TXN_VERIFY_RECEIPT_AMOUNT: {
        instruction_scr_destructor();
        uint64_t value;
        memcpy(&value, var_send_transaction_data.unsigned_transaction.output[var_send_transaction_data.transaction_confirmation_list_index].value, 8);
        log_hex_array("value", (uint8_t*) &value, sizeof(value));
        double valueToDisplay = 1.0 * value / (SATOSHI_PER_BTC);
        char display[225] = {0};
        uint8_t precision = get_floating_precision(value, SATOSHI_PER_BTC);
        snprintf(display, sizeof(display), ui_text_output_send_value_double, var_send_transaction_data.transaction_confirmation_list_index + 1, precision, valueToDisplay, get_coin_symbol(BYTE_ARRAY_TO_UINT32(var_send_transaction_data.transaction_metadata.coin_index), var_send_transaction_data.transaction_metadata.network_chain_id));
        confirm_scr_init(display);
    } break;

    case SEND_TXN_CHECK_RECEIPT_FEES_LIMIT: {
        if (btc_get_txn_fee(&var_send_transaction_data.unsigned_transaction) > get_transaction_fee_threshold(&var_send_transaction_data.unsigned_transaction, BYTE_ARRAY_TO_UINT32(var_send_transaction_data.transaction_metadata.coin_index))) {
            confirm_scr_init(ui_text_warning_transaction_fee_too_high);
            confirm_scr_focus_cancel();
        } else {
        	mark_event_over();
        }
    } break;

    case SEND_TXN_VERIFY_RECEIPT_FEES: {
        instruction_scr_destructor();
        uint64_t txn_fees = btc_get_txn_fee(&var_send_transaction_data.unsigned_transaction);
        double txn_fees_in_btc = 1.0 * txn_fees / (SATOSHI_PER_BTC);
        char display[225] = {0};
        uint8_t precision = get_floating_precision(txn_fees, SATOSHI_PER_BTC);
        snprintf(display, sizeof(display), ui_text_send_transaction_fee_double,
            precision, txn_fees_in_btc, get_coin_symbol(BYTE_ARRAY_TO_UINT32(var_send_transaction_data.transaction_metadata.coin_index), var_send_transaction_data.transaction_metadata.network_chain_id));
        confirm_scr_init(display);
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD: {
        mark_event_over();
    } break;

    case SEND_TXN_ENTER_PASSPHRASE: {
        if (!WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
            flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD;
            break;
        }
        input_text_init(
            PASSPHRASE,
            ui_text_enter_passphrase,
            0,
            DATA_TYPE_PASSPHRASE,
            64);

    } break;
    
    case SEND_TXN_CONFIRM_PASSPHRASE: {
        char display[65];
        snprintf(display, sizeof(display), ui_text_receive_on_address, flow_level.screen_input.input_text);
        address_scr_init(ui_text_confirm_passphrase, display, false);
        memzero(display, sizeof(display));
    } break;

    case SEND_TXN_CHECK_PIN: {
        mark_event_over();
    } break;

    case SEND_TXN_ENTER_PIN: {
        if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
            flow_level.level_three = SEND_TXN_CHECK_PIN;
            break;
        }
        input_text_init(
            ALPHA_NUMERIC,
            ui_text_enter_pin,
            4,
            DATA_TYPE_PIN,
            8);

    } break;

    case SEND_TXN_TAP_CARD: {
        tap_threshold_cards_for_reconstruction();
    } break;

    case SEND_TXN_TAP_CARD_SEND_CMD: {
        delay_scr_init(ui_text_processing, DELAY_SHORT);
        mark_event_over();
    } break;

    case SEND_TXN_READ_DEVICE_SHARE:
    case SEND_TXN_SIGN_TXN:
    case SEND_TXN_WAITING_SCREEN:
        mark_event_over();
        break;

    case SEND_TXN_FINAL_SCREEN:
        delay_scr_init(ui_text_exported_signed_transaction_to_desktop, DELAY_TIME);
        CY_Reset_Not_Allow(true);
        break;

    case SEND_TXN_VERIFY_UTXO_FETCH_RAW_TXN: {
        timeout_task = lv_task_create(_timeout_listener, 100000, LV_TASK_PRIO_HIGH, NULL);
        lv_task_once(timeout_task);

        mark_event_over();
    } break;

    case SEND_TXN_VERIFY_UTXO: {
        mark_event_over();
    } break;

    default:
        break;
    }
}
