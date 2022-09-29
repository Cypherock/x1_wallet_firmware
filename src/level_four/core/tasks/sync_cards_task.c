/**
 * @file    sync_cards_task.c
 * @author  Cypherock X1 Team
 * @brief   Sync witht X1Cards.
 *          This file contains the implementation of the X1Cards sync task.
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
 * sync_cards_task.c
 *
 *  Created on: 19-Aug-2021
 *      Author: Aman Agarwal
 */

#include "bip39.h"
#include "shamir_wrapper.h"
#include "tasks_level_four.h"
#include "tasks_tap_cards.h"
#include "ui_input_text.h"
#include "ui_list.h"
#include "ui_message.h"
#include "ui_address.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "controller_main.h"
#include "flash_api.h"

extern char* ALPHABET;
extern char* ALPHA_NUMERIC;
extern char* NUMBERS;

extern uint8_t shamir_data[TOTAL_NUMBER_OF_SHARES][BLOCK_SIZE];
extern uint8_t arbitrary_data_share[TOTAL_NUMBER_OF_SHARES][MAX_ARBITRARY_DATA_SIZE];
extern uint8_t shamir_data_x_coords[TOTAL_NUMBER_OF_SHARES];
extern Wallet wallet;


void sync_cards_task(){
    switch(flow_level.level_three) {
    case SYNC_CARDS_START:{
        mark_event_over();
    } break;

    case SYNC_CARDS_CURRENT_WALLET_CONFIRM:{
        address_scr_init("Sync Wallet?", (char *)wallet.wallet_name, false);
    } break;
    case SYNC_CARDS_CHECK_WALLET_PIN:{
        mark_event_over();
    } break;
    case SYNC_CARDS_ENTER_PIN_FLOW:{
        if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
            flow_level.level_three = VIEW_SEED_DUMMY_TASK;
            break;
        }
        input_text_init(
            ALPHA_NUMERIC,
            ui_text_enter_pin,
            4,
            DATA_TYPE_PIN,
            8);
    } break;
    case SYNC_CARDS_TAP_TWO_CARDS_FLOW:{
        tap_threshold_cards_for_reconstruction();
    } break;
    case SYNC_CARDS_GENERATE_DEVICE_SHARE:{
        mark_event_over();
    } break;
    case SYNC_CARDS_CHECK_NEXT_WALLET: {
        mark_event_over();
    } break;
    case SYNC_CARDS_SUCCESS:{
        char display[35];
        if(flow_level.level_one == LEVEL_TWO_ADVANCED_SETTINGS)
            snprintf(display, sizeof(display), "%s", ui_text_syncing_complete);
        else{
            snprintf(display, sizeof(display), "Syncing %s complete", wallet.wallet_name);
        }
        delay_scr_init(display, DELAY_TIME);
    } break;
    default:{

    } break;
    }
}
