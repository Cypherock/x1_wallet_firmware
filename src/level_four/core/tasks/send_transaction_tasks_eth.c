/**
 * @file    send_transaction_tasks_eth.c
 * @author  Cypherock X1 Team
 * @brief   Send transaction for ETH.
 *          This file contains functions to send transaction for ETH.
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
#include "eth.h"
#include "constant_texts.h"
#include "controller_level_four.h"
#include "tasks_level_four.h"
#include "ui_address.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_message.h"
#include "math.h"
#include "tasks_tap_cards.h"
#include "utils.h"
#include "contracts.h"

extern char* ALPHABET;
extern char* ALPHA_NUMERIC;
extern char* NUMBERS;
extern char* PASSPHRASE;

extern lv_task_t* timeout_task;


void send_transaction_tasks_eth()
{

    switch (flow_level.level_three) {

    case SEND_TXN_VERIFY_COIN_ETH: {
        instruction_scr_init(ui_text_processing, NULL);
        mark_event_over();
    } break;

    case SEND_TXN_UNSIGNED_TXN_WAIT_SCREEN_ETH: {
        mark_event_over();
    } break;

    case SEND_TXN_UNSIGNED_TXN_RECEIVED_ETH: {
        if (!eth_unsigned_txn_ptr.contract_verified)
            delay_scr_init(ui_text_unverified_contract, DELAY_TIME);
        else
            mark_event_over();
    } break;

    case SEND_TXN_VERIFY_CONTRACT_ADDRESS: {
        char address[43];
        address[0] = '0';
        address[1] = 'x';
        char top_heading[55];
        char display[70];

        instruction_scr_destructor();
        byte_array_to_hex_string(eth_unsigned_txn_ptr.to_address,
                                 ETHEREUM_ADDRESS_LENGTH, address + 2, sizeof(address) - 2);
        snprintf(top_heading, sizeof(top_heading), "%s", ui_text_verify_contract);
        snprintf(display, sizeof(display), "%s%s", ui_text_20_spaces, address);
        address_scr_init(top_heading, display, true);
    } break;

    case SEND_TXN_VERIFY_TXN_NONCE_ETH: {
        char nonce_hex_str[ETH_NONCE_SIZE_BYTES * 2 + 1] = {'\0'};
        uint8_t nonce_dec_str[ETH_NONCE_SIZE_BYTES * 3] = {0};
        uint16_t nonce_dec_len = sizeof(nonce_dec_str), nonce_hex_len;
        int index, offset;
        nonce_hex_len = byte_array_to_hex_string(eth_unsigned_txn_ptr.nonce, eth_unsigned_txn_ptr.nonce_size[0],
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
        instruction_scr_destructor();
        address_scr_init("Verify nonce", (char *) nonce_dec_str, false);
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_ETH: {
        instruction_scr_destructor();
        char address[43];
        address[0] = '0';
        address[1] = 'x';
        char top_heading[225];
        uint8_t address_bytes[20];
        char display[70];

        instruction_scr_destructor();
        eth_get_to_address(&eth_unsigned_txn_ptr, address_bytes);
        byte_array_to_hex_string(address_bytes, sizeof(address_bytes), address + 2, sizeof(address) - 2);
        snprintf(top_heading, sizeof(top_heading), "%s", ui_text_verify_address);
        snprintf(display, sizeof(display), "%s%s", ui_text_20_spaces, address);
        address_scr_init(top_heading, display, true);
    } break;

    case SEND_TXN_CALCULATE_AMOUNT_ETH: {
        instruction_scr_init(ui_text_fetching_recipient_amount, NULL);
        mark_event_over();
    }break;

    case SEND_TXN_VERIFY_RECEIPT_AMOUNT_ETH: {
        char amount_string[65] = {'\0'}, amount_decimal_string[30] = {'\0'};
        char display[110] = {'\0'};
        memzero(amount_string, sizeof(amount_string));
        uint8_t len = 0, i = 0, j = 0;

        char token[9];
        snprintf(token, sizeof(token), "%s", var_send_transaction_data.transaction_metadata.token_name);
        
        len = eth_get_value(&eth_unsigned_txn_ptr, amount_string);
        uint8_t decimal_val_s[ETH_VALUE_SIZE_BYTES * 3] = {0};
        if (sizeof(decimal_val_s)/sizeof(decimal_val_s[0]) > UINT8_MAX){
          LOG_ERROR("0xxx#");
          break;
        }
        const uint8_t dec_val_len = sizeof(decimal_val_s)/sizeof(decimal_val_s[0]); //logbase10(2pow256) roughly equals 78
        convertbase16tobase10(len, amount_string, decimal_val_s, dec_val_len);
        bool pre_dec_digit = false, post_dec_digit = false;
        uint8_t offset = 0;
        log_hex_array("eth value: ", (uint8_t*)amount_string, len);
        uint8_t point_index = dec_val_len - var_send_transaction_data.transaction_metadata.decimal[0];
        i = 0;
        j = dec_val_len - 1;

        while(i <= j){
            if (i == point_index && post_dec_digit){
                if(!pre_dec_digit){
                    offset += snprintf(amount_decimal_string + offset, sizeof(amount_decimal_string) - offset, "0");
                }
                offset += snprintf(amount_decimal_string + offset, sizeof(amount_decimal_string) - offset, ".");
            }
            if (j >= point_index){
                if (!decimal_val_s[j] && !post_dec_digit){
                    j--;
                }
                else if(decimal_val_s[j]){
                    post_dec_digit = true;
                }
            }
            if(decimal_val_s[i] || i == point_index){
                pre_dec_digit = true;
            }
            if(pre_dec_digit || decimal_val_s[i]){
                //attach non zero leading value detected or decimal digits till j(should be the
                //last non zero decimal digit index).
                offset += snprintf(amount_decimal_string + offset, sizeof(amount_decimal_string) - offset, "%d", decimal_val_s[i]);
            }
            i++;
        }
        if(!post_dec_digit && !pre_dec_digit){
            snprintf(amount_decimal_string, sizeof(amount_decimal_string) - 1, "0.0");
        }

        instruction_scr_destructor();
        snprintf(display, sizeof(display), ui_text_verify_amount, amount_decimal_string, var_send_transaction_data.transaction_metadata.token_name);
        confirm_scr_init(display);
    } break;

    case SEND_TXN_VERIFY_RECEIPT_FEES_ETH: {
        instruction_scr_destructor();
        uint8_t point_index;
        char gas_eth_dec_str[30] = {'\0'};
        uint64_t txn_fee = bendian_byte_to_dec(eth_unsigned_txn_ptr.gas_price, eth_unsigned_txn_ptr.gas_price_size[0]) / 1000000000;
        txn_fee *= bendian_byte_to_dec(eth_unsigned_txn_ptr.gas_limit, eth_unsigned_txn_ptr.gas_limit_size[0]);

        point_index = snprintf(gas_eth_dec_str + 1, sizeof(gas_eth_dec_str) - 1, "%09llu", txn_fee);
        ASSERT(point_index >= 0 && point_index < sizeof(gas_eth_dec_str));
        gas_eth_dec_str[0] = point_index > ETH_GWEI_INDEX ? ' ' : '0';
        gas_eth_dec_str[point_index + 1] = '\0';
        point_index++;
        for (int i = 0; i < ETH_GWEI_INDEX; i++, point_index--) {
            gas_eth_dec_str[point_index] = gas_eth_dec_str[point_index - 1];
        }
        gas_eth_dec_str[point_index] = '.';

        char display[125];
        snprintf(display, sizeof(display), ui_text_send_transaction_fee, gas_eth_dec_str, "ETH");
        confirm_scr_init(display);
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD_ETH: {
        mark_event_over();
    } break;

    case SEND_TXN_ENTER_PASSPHRASE_ETH: {
        if (!WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
            flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD_ETH;
            break;
        }
        input_text_init(
            PASSPHRASE,
            ui_text_enter_passphrase,
            0,
            DATA_TYPE_PASSPHRASE,
            64);

    } break;

    case SEND_TXN_CONFIRM_PASSPHRASE_ETH: {
        char display[65];
        snprintf(display, sizeof(display), ui_text_receive_on_address, flow_level.screen_input.input_text);
        address_scr_init(ui_text_confirm_passphrase, display, false);
        memzero(display, sizeof(display));
    } break;

    case SEND_TXN_CHECK_PIN_ETH: {
        mark_event_over();
    } break;

    case SEND_TXN_ENTER_PIN_ETH: {
        if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
            flow_level.level_three = SEND_TXN_CHECK_PIN_ETH;
            break;
        }
        input_text_init(
            ALPHA_NUMERIC,
            ui_text_enter_pin,
            4,
            DATA_TYPE_PIN,
            8);

    } break;

    case SEND_TXN_TAP_CARD_ETH: {
        tap_threshold_cards_for_reconstruction();
    } break;

    case SEND_TXN_TAP_CARD_SEND_CMD_ETH: {
        instruction_scr_init(ui_text_signing_transaction, NULL);
        mark_event_over();
    } break;

    case SEND_TXN_READ_DEVICE_SHARE_ETH: {
        mark_event_over();
    } break;

    case SEND_TXN_SIGN_TXN_ETH: {
        mark_event_over();
    } break;

    case SEND_TXN_WAITING_SCREEN_ETH: {
        mark_event_over();
    } break;

    case SEND_TXN_FINAL_SCREEN_ETH:
        delay_scr_init(ui_text_exported_signed_transaction_to_desktop, DELAY_TIME);
        CY_Reset_Not_Allow(true);
        break;
    
    default:
        break;
    }

}
