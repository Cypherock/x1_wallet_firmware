/**
 * @file    card_action_controllers.c
 * @author  Cypherock X1 Team
 * @brief   Handle reading logic of wallets from card.
 *          This file contains the functions to retrieve wallet from cards.
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
#include "controller_tap_cards.h"
#include "flash_api.h"
#include "nfc.h"
#include "shamir_wrapper.h"
#include "tasks.h"
#include "ui_instruction.h"
#include "ui_message.h"
#include "wallet_utilities.h"
#include "wallet.h"

static void handle_retrieve_wallet_success(uint8_t xcor);

void readback_share_from_card(uint8_t xcor)
{
    tap_card_data.retries = 5;
    while (1) {
        tap_card_data.acceptable_cards = encode_card_number(xcor + 1);
        memcpy(tap_card_data.family_id, get_family_id(), FAMILY_ID_SIZE);
        tap_card_data.lvl3_retry_point = flow_level.level_three;
        tap_card_data.lvl4_retry_point = flow_level.level_four;
        if (xcor == 0) tap_card_data.tapped_card = 0;
        if (!tap_card_applet_connection())
            return;
        tap_card_data.status = nfc_retrieve_wallet(&wallet);

        if (tap_card_data.status == SW_NO_ERROR) {
            buzzer_start(BUZZER_DURATION);
            instruction_scr_change_text(ui_text_remove_card_prompt, true);
            if(xcor != 3){
                nfc_detect_card_removal();
            }
            handle_retrieve_wallet_success(xcor);
            instruction_scr_destructor();
            break;
        } else if (tap_card_handle_applet_errors()) {
            break;
        }
    }
}

static void handle_retrieve_wallet_success(uint8_t xcor)
{
    if (WALLET_IS_ARBITRARY_DATA(wallet.wallet_info))
        memcpy(((uint8_t *) wallet_shamir_data.arbitrary_data_shares) + xcor * wallet.arbitrary_data_size, wallet.arbitrary_data_share, wallet.arbitrary_data_size);
    else
        memcpy(wallet_shamir_data.mnemonic_shares[xcor], wallet.wallet_share_with_mac_and_nonce, BLOCK_SIZE);
    memcpy(wallet_shamir_data.share_encryption_data[xcor], wallet.wallet_share_with_mac_and_nonce + BLOCK_SIZE, NONCE_SIZE + WALLET_MAC_SIZE);
    memzero(wallet.arbitrary_data_share, sizeof(wallet.arbitrary_data_share));
    memzero(wallet.wallet_share_with_mac_and_nonce, sizeof(wallet.wallet_share_with_mac_and_nonce));

    wallet_shamir_data.share_x_coords[xcor] = wallet.xcor;
    if (xcor < 3) {
        flow_level.level_four++;
    } else {
        flow_level.level_three++;
        flow_level.level_four = 1;
    }
}

int verify_card_share_data() {
    uint8_t CONFIDENTIAL secret[MAX_ARBITRARY_DATA_SIZE];
    uint8_t status = 0;
    uint8_t wallet_id[WALLET_ID_SIZE] = {0};

    wallet_shamir_data.share_x_coords[4] = 5;
    get_flash_wallet_share_by_name((const char *)wallet.wallet_name, wallet_shamir_data.mnemonic_shares[4]);

    if (WALLET_IS_PIN_SET(wallet.wallet_info))
        decrypt_shares();
    if (WALLET_IS_ARBITRARY_DATA(wallet.wallet_info)) {
        status = generate_data_5C2(
            wallet.arbitrary_data_size,
            wallet_shamir_data.arbitrary_data_shares,
            wallet_shamir_data.share_x_coords,
            secret);
        memzero(wallet_shamir_data.arbitrary_data_shares, sizeof(wallet_shamir_data.arbitrary_data_shares));
    } else {
        status = generate_shares_5C2(
            wallet_shamir_data.mnemonic_shares,
            wallet_shamir_data.share_x_coords,
            secret);
        memzero(wallet_shamir_data.mnemonic_shares, sizeof(wallet_shamir_data.mnemonic_shares));
    }

    if (status == 1) {
        // verify wallet id only if secret successfully regenerated
        mnemonic_clear();
        const char* mnemo = mnemonic_from_data(secret, wallet.number_of_mnemonics * 4 / 3);
        ASSERT(mnemo != NULL);
        calculate_wallet_id(wallet_id, mnemo);
        status = memcmp(wallet.wallet_id, wallet_id, WALLET_ID_SIZE);
        LOG_INFO("xxx36: %d", status);
        status = (status == 0) ? 1 : 0;
        mnemonic_clear();
    }

    // both checks complete; accordingly update on flash
    uint8_t wallet_index;
    get_index_by_name((const char *)wallet.wallet_name, &wallet_index);
    set_wallet_state(wallet_index, status == 1 ? VALID_WALLET : INVALID_WALLET);

    memzero(secret, sizeof(secret));
    return status;
}
