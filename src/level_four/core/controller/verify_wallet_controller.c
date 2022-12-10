/**
 * @file    verify_wallet_controller.c
 * @author  Cypherock X1 Team
 * @brief   Verify wallet next controller.
 *          Handles post event (only next events) operations for verify wallet flow
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
#include "controller_tap_cards.h"
#include "sha2.h"
#include "flash_api.h"

extern Wallet_credential_data wallet_credential_data;

void verify_wallet_controller()
{
    switch (flow_level.level_three) {
    case VERIFY_WALLET_START:
        if (WALLET_IS_PIN_SET(wallet.wallet_info))
            flow_level.level_three = VERIFY_WALLET_PIN_INPUT;
        else
            flow_level.level_three = VERIFY_WALLET_TAP_CARDS_FLOW;
        break;

    case VERIFY_WALLET_PIN_INPUT:
        sha256_Raw((uint8_t*)flow_level.screen_input.input_text, strlen(flow_level.screen_input.input_text), wallet_credential_data.password_single_hash);
        sha256_Raw(wallet_credential_data.password_single_hash, SHA256_DIGEST_LENGTH, wallet.password_double_hash);
        memzero(flow_level.screen_input.input_text, sizeof(flow_level.screen_input.input_text));
        flow_level.level_three = VERIFY_WALLET_TAP_CARDS_FLOW;
        break;

    case VERIFY_WALLET_TAP_CARDS_FLOW:
        tap_cards_for_verification_flow_controller();
        break;

    case VERIFY_WALLET_READ_DEVICE_SHARE:
        wallet_shamir_data.share_x_coords[4] = 5;
        get_flash_wallet_share_by_name((const char *)wallet.wallet_name, wallet_shamir_data.mnemonic_shares[4]);
        memcpy(wallet_shamir_data.share_encryption_data[4], wallet_shamir_data.share_encryption_data[0], NONCE_SIZE+WALLET_MAC_SIZE);
        flow_level.level_three = VERIFY_WALLET_SHOW_MNEMONICS;
        break;

    case VERIFY_WALLET_SHOW_MNEMONICS: {
        uint8_t wallet_index;
        get_index_by_name((const char *)wallet.wallet_name, &wallet_index);
        set_wallet_state(wallet_index, VALID_WALLET);
        flow_level.level_three = VERIFY_WALLET_COMPLETE_INSTRUCTION;
    } break;

    case VERIFY_WALLET_COMPLETE_INSTRUCTION:
        flow_level.level_three = VERIFY_WALLET_SUCCESS;
        break;

    case VERIFY_WALLET_SUCCESS:
        reset_flow_level();
        break;

    case VERIFY_WALLET_DELETE:
        mark_error_screen(ui_text_wallet_verification_failed);

        /**
         * Our intention here is to delete wallet using delete_wallet_controller
         * which is called by flow_level.level_two = LEVEL_THREE_DELETE_WALLET
         */
        flow_level.level_three = DELETE_WALLET_DUMMY_TASK;
        flow_level.level_two = LEVEL_THREE_DELETE_WALLET;
        break;

    default:
        break;
    }
}
