/**
 * @file    sync_cards_controller.c
 * @author  Cypherock X1 Team
 * @brief   Sync with X1 Card next controller.
 *          Handles post event (only next events) operations for sync with X1 Cards flow.
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
/*
 * sync_cards_controller.c
 *
 *  Created on: 19-Aug-2021
 *      Author: Aman Agarwal
 */

#include "controller_level_four.h"
#include "controller_tap_cards.h"
#include "sha2.h"
#include "flash_api.h"
#include "shamir_wrapper.h"

extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;
extern Flash_Wallet wallet_for_flash;
static uint32_t valid_wallet_index = 0;

void sync_cards_controller(void)
{

    switch(flow_level.level_three) {
    case SYNC_CARDS_START:{
        valid_wallet_index++;
        flow_level.level_three = SYNC_CARDS_CURRENT_WALLET_CONFIRM;
    } break;

    case SYNC_CARDS_CURRENT_WALLET_CONFIRM:{
        flow_level.level_three = SYNC_CARDS_CHECK_WALLET_PIN;
    } break;

    case SYNC_CARDS_CHECK_WALLET_PIN:{
        if(WALLET_IS_PIN_SET(wallet.wallet_info)){
            flow_level.level_three = SYNC_CARDS_ENTER_PIN_FLOW;
        } else{
            flow_level.level_three = SYNC_CARDS_TAP_TWO_CARDS_FLOW;
        }
    } break;

    case SYNC_CARDS_ENTER_PIN_FLOW:{
        sha256_Raw((uint8_t*)flow_level.screen_input.input_text, strlen(flow_level.screen_input.input_text), wallet_credential_data.password_single_hash);
        sha256_Raw(wallet_credential_data.password_single_hash, SHA256_DIGEST_LENGTH, wallet.password_double_hash);
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
        flow_level.level_three = SYNC_CARDS_TAP_TWO_CARDS_FLOW;
    } break;

    case SYNC_CARDS_TAP_TWO_CARDS_FLOW:{
        tap_card_data.desktop_control = false;
        tap_threshold_cards_for_reconstruction_flow_controller(2);
        if (counter.level == LEVEL_ONE ||
            (flow_level.level_one == LEVEL_TWO_OLD_WALLET && flow_level.level_two == LEVEL_THREE_WALLET_LOCKED)) {
            // if wallet is locked, then we need to go to next wallet
            counter.level = LEVEL_THREE;
            flow_level.level_one = LEVEL_TWO_ADVANCED_SETTINGS;
            flow_level.level_two = LEVEL_THREE_SYNC_WALLET_FLOW;
            flow_level.level_three = SYNC_CARDS_CHECK_NEXT_WALLET;
        }

    } break;

    case SYNC_CARDS_GENERATE_DEVICE_SHARE:{
        uint8_t temp_password_hash[SHA256_DIGEST_LENGTH];
        if (WALLET_IS_PIN_SET(wallet.wallet_info)){
            memcpy(temp_password_hash, wallet_credential_data.password_single_hash, SHA256_DIGEST_LENGTH);
            decrypt_shares();
        }
        recover_share_from_shares(
                BLOCK_SIZE,
                MINIMUM_NO_OF_SHARES,
				wallet_shamir_data.mnemonic_shares,
				wallet_shamir_data.share_x_coords,
				wallet_shamir_data.mnemonic_shares[4],
                5);

        if(WALLET_IS_PIN_SET(wallet.wallet_info)){
            memcpy(wallet_shamir_data.share_encryption_data[4], wallet_shamir_data.share_encryption_data[0], NONCE_SIZE+WALLET_MAC_SIZE);
            memcpy(wallet_credential_data.password_single_hash, temp_password_hash, SHA256_DIGEST_LENGTH);
            memzero(temp_password_hash, SHA256_DIGEST_LENGTH);
            encrypt_shares();
        }

        uint32_t wallet_index;
        Flash_Wallet *flash_wallet;
        get_index_by_name((const char *)wallet.wallet_name, (uint8_t *)(&wallet_index));
        get_flash_wallet_by_name((const char *)wallet.wallet_name, &flash_wallet);
        memcpy(&wallet_for_flash, flash_wallet, sizeof(Flash_Wallet));
        put_wallet_share_sec_flash(wallet_index, wallet_shamir_data.mnemonic_shares[4]);

        if(flow_level.level_one == LEVEL_TWO_ADVANCED_SETTINGS){
            if(valid_wallet_index)
                valid_wallet_index--;
            flow_level.level_three = SYNC_CARDS_CHECK_NEXT_WALLET;
        }else{
            flow_level.level_three = SYNC_CARDS_SUCCESS;
        }
    } break;

    case SYNC_CARDS_CHECK_NEXT_WALLET: {
        uint8_t index = -1;
        flow_level.level_three = SYNC_CARDS_SUCCESS;
        if (get_ith_wallet_without_share(valid_wallet_index, &index) == SUCCESS_){
            memcpy(
                wallet.wallet_name,
                get_wallet_name(index), NAME_SIZE);
            wallet.wallet_info = get_wallet_info(index);
            flow_level.level_three = SYNC_CARDS_START;
        }
    } break;

    case SYNC_CARDS_SUCCESS:{
        valid_wallet_index=0;
        reset_flow_level();
    } break;

    default:{

    } break;
    }
}
