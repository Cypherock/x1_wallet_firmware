/**
 * @file    reconstruct_from_threshold_cards_controller.c
 * @author  Cypherock X1 Team
 * @brief   Reconstruct from threshold.
 *          This file contains the implementation of the functions that
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
#include "ui_message.h"
#include "controller_tap_cards.h"

extern int error_occurred_remaining_tries;
extern Wallet_shamir_data wallet_shamir_data;

static uint8_t remaining_cards;

static void _tap_card_backend(uint8_t xcor);
static void _handle_retrieve_wallet_success(uint8_t xcor);

void tap_threshold_cards_for_reconstruction_flow_controller(uint8_t threshold)
{
    switch (flow_level.level_four) {
    case TAP_CARD_ONE_FRONTEND:
        tap_card_data.tapped_card = 0;
        tap_card_data.active_cmd_type = CARD_ERROR_FACED;
        flow_level.level_four = TAP_CARD_ONE_BACKEND;
        break;

    case TAP_CARD_ONE_BACKEND:
        remaining_cards = 15;
        _tap_card_backend(0);
        buzzer_start(BUZZER_DURATION);
        instruction_scr_destructor();
        if(threshold == 1 && flow_level.level_four == TAP_CARD_TWO_FRONTEND){
            flow_level.level_three++;
            flow_level.level_four = 1;
        }
        break;

    case TAP_CARD_TWO_FRONTEND:
        flow_level.level_four = TAP_CARD_TWO_BACKEND;
        break;

    case TAP_CARD_TWO_BACKEND:
        _tap_card_backend(1);
        break;

    default:
        message_scr_init(ui_text_something_went_wrong);
        break;
    }
}

static void _tap_card_backend(uint8_t xcor)
{
    tap_card_data.retries = 5;

    while (!CY_Read_Reset_Flow()) {
        tap_card_data.acceptable_cards = remaining_cards;
        memcpy(tap_card_data.family_id, get_family_id(), FAMILY_ID_SIZE);
        tap_card_data.lvl3_retry_point = flow_level.level_three;
        tap_card_data.lvl4_retry_point = flow_level.level_four - 1;
        if (xcor == 0) tap_card_data.tapped_card = 0;
        if (!tap_card_applet_connection())
            return;
        tap_card_data.status = nfc_retrieve_wallet(&wallet);

        if (tap_card_data.status == SW_NO_ERROR) {
            remaining_cards = tap_card_data.acceptable_cards;
            if (tap_card_data.desktop_control)
                transmit_one_byte_confirm(USER_ENTERED_PIN);
            _handle_retrieve_wallet_success(xcor);
            buzzer_start(BUZZER_DURATION);
            instruction_scr_destructor();
            break;
        } else if (tap_card_handle_applet_errors()) {
            break;
        }
    }
}

static void _handle_retrieve_wallet_success(uint8_t xcor)
{
    if (WALLET_IS_ARBITRARY_DATA(wallet.wallet_info))
        memcpy(((uint8_t *) wallet_shamir_data.arbitrary_data_shares) + xcor * wallet.arbitrary_data_size, wallet.arbitrary_data_share, wallet.arbitrary_data_size);
    else
        memcpy(wallet_shamir_data.mnemonic_shares[xcor], wallet.wallet_share_with_mac_and_nonce, BLOCK_SIZE);
    memcpy(wallet_shamir_data.share_encryption_data[xcor], wallet.wallet_share_with_mac_and_nonce + BLOCK_SIZE, NONCE_SIZE + WALLET_MAC_SIZE);
    memzero(wallet.arbitrary_data_share, sizeof(wallet.arbitrary_data_share));
    memzero(wallet.wallet_share_with_mac_and_nonce, sizeof(wallet.wallet_share_with_mac_and_nonce));

    wallet_shamir_data.share_x_coords[xcor] = wallet.xcor;

    if(xcor < 1){
        flow_level.level_four++;
    }
    else{
        flow_level.level_three++;
        flow_level.level_four = 1;
    }
}
