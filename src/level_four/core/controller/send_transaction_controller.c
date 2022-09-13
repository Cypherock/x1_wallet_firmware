/**
 * @file    send_transaction_controller.c
 * @author  Cypherock X1 Team
 * @brief   Send transaction next controller for BTC.
 *          Handles post event (only next events) operations for send transaction flow initiated by desktop app.
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
#include "ui_confirmation.h"
#include "ui_instruction.h"
#include "ui_message.h"
#include "controller_tap_cards.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "controller_main.h"

extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;

int input_index;

Send_Transaction_Data var_send_transaction_data = {
    .transaction_confirmation_list_index = 0,
    .unsigned_transaction = {
        .network_version = {0},
        .input_count = {0}, .input = NULL,
        .output_count = {0}, .output = NULL,
        .locktime = {0}, .sighash = {0}
    },
    .transaction_metadata = {
        .wallet_index = {0}, .purpose_index = {0},
        .coin_index = {0}, .account_index = {0},
        .input_count = {0}, .input = NULL,
        .output_count = {0}, .output = NULL,
        .change_count = {0}, .change = NULL,
        .transaction_fees = {0}, .decimal = {0},
        .token_name = NULL, .network_chain_id = 0
    },
    .signed_transaction = {
        .network_version = {0},
        .marker = {0}, .flag = {0},
        .input_count = {0}, .input = NULL,
        .output_count = {0}, .output = NULL,
        .witness = NULL, .locktime = {0},
    }
};
Send_Transaction_Cmd send_transaction_cmd = {
    .signed_txn_byte_array = NULL
};


extern lv_task_t* timeout_task;

void send_transaction_controller()
{
    switch (flow_level.level_three) {

    case SEND_TXN_VERIFY_COIN: {
        uint8_t arr[3] = {0x01, 0x1f, 0x40};
        transmit_data_to_app(SEND_TXN_REQ_UNSIGNED_TXN, arr, sizeof(arr));
        flow_level.level_three = SEND_TXN_UNSIGNED_TXN_WAIT_SCREEN;
        input_index = 0;
    } break;

    case SEND_TXN_UNSIGNED_TXN_WAIT_SCREEN: {
        uint8_t *data_array = NULL;
        uint16_t msg_size = 0;
        if (get_usb_msg_by_cmd_type(SEND_TXN_UNSIGNED_TXN, &data_array, &msg_size)) {
            byte_array_to_unsigned_txn(data_array, msg_size, &var_send_transaction_data.unsigned_transaction);
            clear_message_received_data();
            instruction_scr_destructor();
            lv_task_del(timeout_task);
            flow_level.level_three = SEND_TXN_UNSIGNED_TXN_RECEIVED;
            if (!btc_validate_unsigned_txn(&var_send_transaction_data.unsigned_transaction)) {
                comm_reject_request(SEND_TXN_REQ_UNSIGNED_TXN, 0);
                reset_flow_level();
                mark_error_screen(ui_text_wrong_btc_transaction);
                return;
            }
        }
    } break;

    case SEND_TXN_UNSIGNED_TXN_RECEIVED: {
        flow_level.level_three = SEND_TXN_VERIFY_UTXO_FETCH_RAW_TXN;
        transmit_one_byte(SEND_TXN_REQ_UNSIGNED_TXN, 0x02);
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS: {
        flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_AMOUNT;
    } break;

    case SEND_TXN_VERIFY_RECEIPT_AMOUNT: {
        var_send_transaction_data.transaction_confirmation_list_index++;
        if (var_send_transaction_data.transaction_confirmation_list_index > var_send_transaction_data.unsigned_transaction.output_count[0] - (var_send_transaction_data.transaction_metadata.change_count[0] + 1)) {
            flow_level.level_three = SEND_TXN_CHECK_RECEIPT_FEES_LIMIT;
        } else {
            flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS;
        }
    } break;

    case SEND_TXN_CHECK_RECEIPT_FEES_LIMIT: {
        flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_FEES;
    } break;

    case SEND_TXN_VERIFY_RECEIPT_FEES: {
        flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD;
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD: {
        memzero(wallet_credential_data.passphrase, sizeof(wallet_credential_data.passphrase));
        if (WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
            flow_level.level_three = SEND_TXN_ENTER_PASSPHRASE;
        } else {
            flow_level.level_three = SEND_TXN_CHECK_PIN;
        }
    } break;
    
    case SEND_TXN_ENTER_PASSPHRASE: {
         flow_level.level_three = SEND_TXN_CONFIRM_PASSPHRASE;
    } break;

    case SEND_TXN_CONFIRM_PASSPHRASE: {
        snprintf(wallet_credential_data.passphrase, sizeof(wallet_credential_data.passphrase), "%s", flow_level.screen_input.input_text);
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
        flow_level.level_three = SEND_TXN_CHECK_PIN;
    
    } break;

    case SEND_TXN_CHECK_PIN: {
        if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
            flow_level.level_three = SEND_TXN_ENTER_PIN;
        } else {
            flow_level.level_three = SEND_TXN_TAP_CARD;
        }
        
    } break;

    case SEND_TXN_ENTER_PIN: {
        sha256_Raw((uint8_t*)flow_level.screen_input.input_text, strlen(flow_level.screen_input.input_text), wallet_credential_data.password_single_hash);
        sha256_Raw(wallet_credential_data.password_single_hash, SHA256_DIGEST_LENGTH, wallet.password_double_hash);
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
        flow_level.level_three = SEND_TXN_TAP_CARD;
    } break;

    case SEND_TXN_TAP_CARD: {
        tap_card_data.desktop_control = true;
        tap_threshold_cards_for_reconstruction_flow_controller(1);
    } break;

    case SEND_TXN_TAP_CARD_SEND_CMD: {
        flow_level.level_three = SEND_TXN_READ_DEVICE_SHARE;
    } break;

    case SEND_TXN_READ_DEVICE_SHARE:
        wallet_shamir_data.share_x_coords[1] = 5;
        get_flash_wallet_share_by_name((const char *)wallet.wallet_name, wallet_shamir_data.mnemonic_shares[1]);
        memcpy(wallet_shamir_data.share_encryption_data[1], wallet_shamir_data.share_encryption_data[0], NONCE_SIZE+WALLET_MAC_SIZE);
        flow_level.level_three = SEND_TXN_SIGN_TXN;
        break;

    case SEND_TXN_SIGN_TXN: {
        uint8_t secret[BLOCK_SIZE];
        if (WALLET_IS_PIN_SET(wallet.wallet_info) && input_index == 0)
            decrypt_shares();
        recover_secret_from_shares(
            BLOCK_SIZE,
            MINIMUM_NO_OF_SHARES,
            wallet_shamir_data.mnemonic_shares,
            wallet_shamir_data.share_x_coords,
            secret);
        mnemonic_clear();
        const char* mnemo = mnemonic_from_data(secret, wallet.number_of_mnemonics * 4 / 3);
        if (input_index == 0 && !validate_change_address(&var_send_transaction_data.unsigned_transaction,
            &var_send_transaction_data.transaction_metadata, mnemo, wallet_credential_data.passphrase)) {
            instruction_scr_destructor();
            comm_reject_request(SEND_TXN_SENDING_SIGNED_TXN, 0x02);
            mark_error_screen(ui_text_btc_change_address_mismatch);
            reset_flow_level();
            return;
        }
        txn_preimage preimage;
        memzero(&preimage, sizeof(txn_preimage));
        send_transaction_cmd.signed_txn_byte_array = (uint8_t *) malloc(128 /* max size for script_sig */ * sizeof(uint8_t));
        ASSERT(send_transaction_cmd.signed_txn_byte_array != NULL);
        send_transaction_cmd.signed_txn_length = sig_from_unsigned_txn(&var_send_transaction_data.unsigned_transaction,
                                                      &var_send_transaction_data.transaction_metadata,
                                                      input_index, mnemo, wallet_credential_data.passphrase, &preimage,
                                                      send_transaction_cmd.signed_txn_byte_array);
        memzero(secret, sizeof(secret));
        mnemonic_clear();
        flow_level.level_three = SEND_TXN_WAITING_SCREEN;
    } break;

    case SEND_TXN_WAITING_SCREEN: {
        uint8_t *data_array = NULL;
        uint16_t msg_size = 0;
        if (!get_usb_msg_by_cmd_type(SEND_TXN_SENDING_SIGNED_TXN, &data_array, &msg_size))
            break;
        clear_message_received_data();
        transmit_data_to_app(SEND_TXN_SENDING_SIGNED_TXN, send_transaction_cmd.signed_txn_byte_array, send_transaction_cmd.signed_txn_length);
        if (++input_index < var_send_transaction_data.unsigned_transaction.input_count[0]) {
            flow_level.level_three = SEND_TXN_SIGN_TXN;
        } else {
            instruction_scr_destructor();
            lv_obj_clean(lv_scr_act());
            flow_level.level_three = SEND_TXN_FINAL_SCREEN;
            memzero(wallet_credential_data.passphrase, sizeof(wallet_credential_data.passphrase));
            memzero(wallet_shamir_data.mnemonic_shares, sizeof(wallet_shamir_data.mnemonic_shares));
            free(send_transaction_cmd.signed_txn_byte_array);
        }
    } break;

    case SEND_TXN_FINAL_SCREEN:
        reset_flow_level();
        break;

    case SEND_TXN_VERIFY_UTXO_FETCH_RAW_TXN: {
        flow_level.level_three = SEND_TXN_VERIFY_UTXO;
    } break;

    case SEND_TXN_VERIFY_UTXO: {
        uint8_t *data_array = NULL;
        uint16_t msg_size = 0;
        if (get_usb_msg_by_cmd_type(SEND_TXN_REQ_UNSIGNED_TXN, &data_array, &msg_size)) {
            lv_task_del(timeout_task);
            if (!btc_verify_utxo(data_array, msg_size,
                &var_send_transaction_data.unsigned_transaction.input[var_send_transaction_data.transaction_confirmation_list_index])) {
                LOG_ERROR("utxo %d invalid", var_send_transaction_data.transaction_confirmation_list_index);
                instruction_scr_destructor();
                mark_error_screen(ui_text_invalid_transaction);
                reset_flow_level();
                comm_reject_request(SEND_TXN_REQ_UNSIGNED_TXN, 0);
                clear_message_received_data();
                return;
            }
            clear_message_received_data();
            transmit_one_byte_confirm(SEND_TXN_REQ_UNSIGNED_TXN);
            var_send_transaction_data.transaction_confirmation_list_index++;
            if (var_send_transaction_data.transaction_confirmation_list_index >= var_send_transaction_data.unsigned_transaction.input_count[0]) {
                var_send_transaction_data.transaction_confirmation_list_index = 0;
                flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS;
            } else {
                flow_level.level_three = SEND_TXN_VERIFY_UTXO_FETCH_RAW_TXN;
            }
        }
    } break;

    default:
        break;
    }
}
