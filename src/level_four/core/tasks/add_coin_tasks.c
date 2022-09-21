/**
 * @file    add_coin_tasks.c
 * @author  Cypherock X1 Team
 * @brief   Add coin task.
 *          This file contains the implementation of the add coin task.
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
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_menu.h"
#include "ui_message.h"
#include "ui_list.h"
#include "ui_address.h"
#include "controller_tap_cards.h"
#include "tasks_tap_cards.h"

extern char* ALPHABET;
extern char* ALPHA_NUMERIC;
extern char* NUMBERS;
extern char* PASSPHRASE;

extern lv_task_t* success_task;
extern lv_task_t* timeout_task;

extern Add_Coin_Data add_coin_data;


void add_coin_tasks()
{
    switch (flow_level.level_three) {

    case ADD_COINS_VERIFY: {
        char max_coin_supported[MAXIMUM_COIN_SUPPORTED][MAX_COIN_NAME_LENGTH];
        uint8_t coinIndex = 0;
        for (; coinIndex < add_coin_data.number_of_coins; coinIndex++) {
            snprintf(max_coin_supported[coinIndex], sizeof(max_coin_supported[coinIndex]), "%s", get_coin_name(add_coin_data.coin_indexes[coinIndex], add_coin_data.network_chain_ids[coinIndex]));
        }
        list_init(max_coin_supported, add_coin_data.number_of_coins, (add_coin_data.resync ? ui_text_resync_coins : ui_text_add_coins), true);
    } break;

    case ADD_COINS_ENTER_PASSPHRASE: {
        if (!WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
            flow_level.level_three = ADD_COINS_VERIFY;
            break;
        }
        input_text_init(
            PASSPHRASE,
            ui_text_enter_passphrase,
            0,
            DATA_TYPE_PASSPHRASE,
            64);
    } break;

    case ADD_COINS_CONFIRM_PASSPHRASE: {
        char display[65];
        snprintf(display, sizeof(display), ui_text_receive_on_address, flow_level.screen_input.input_text);
        address_scr_init(ui_text_confirm_passphrase, display, false);
        memzero(display, sizeof(display));
    } break;

    case ADD_COINS_CHECK_PIN: {
        mark_event_over();
    } break;

    case ADD_COINS_ENTER_PIN: {
        if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
            flow_level.level_three = ADD_COINS_CHECK_PIN;
            break;
        }
        input_text_init(
            ALPHA_NUMERIC,
            ui_text_enter_pin,
            4,
            DATA_TYPE_PIN,
            8);
    } break;

    case ADD_COINS_TAP_CARD: {
        tap_threshold_cards_for_reconstruction();
    } break;

    case ADD_COINS_TAP_CARD_SEND_CMD: {
        instruction_scr_init(ui_text_processing, NULL);
        BSP_DelayMs(DELAY_TIME);
        mark_event_over();
    } break;

    case ADD_COINS_READ_DEVICE_SHARE: {
        mark_event_over();
    } break;

    case ADD_COIN_GENERATING_XPUBS: {
        mark_event_over();
    } break;

    case ADD_COINS_FINAL_SCREEN:
        instruction_scr_destructor();
        delay_scr_init(ui_text_wait_while_balance_fetching, DELAY_TIME);
        CY_Reset_Not_Allow(true);
        break;
    
    default:
        break;
    }
}
