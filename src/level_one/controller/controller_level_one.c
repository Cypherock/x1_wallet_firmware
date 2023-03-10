/**
 * @file    controller_level_one.c
 * @author  Cypherock X1 Team
 * @brief   Level one next controller (main).
 *          Handles post event (only next events) operations for level one
 *          tasks of the main application.
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
#include "controller_level_one.h"
#include "communication.h"
#include "controller_level_two.h"
#include "application_startup.h"
#include "ui_instruction.h"
#include "flash_api.h"
#include "controller_tap_cards.h"
#include "apdu.h"
#include <inttypes.h>
#include <string.h>

extern lv_task_t* listener_task;

extern uint8_t device_auth_flag;
extern bool main_app_ready;

void level_one_controller()
{
#if X1WALLET_MAIN
    if (flow_level.show_error_screen) {
        flow_level.show_error_screen = false;
        return;
    }

    if (flow_level.show_desktop_start_screen) {
        flow_level.show_desktop_start_screen = false;
        return;
    }

    if (get_card_data_health() == DATA_HEALTH_CORRUPT) {
        reset_card_data_health();
        return;
    }

    if (!main_app_ready) {
        device_auth_flag = 0;
        return;
    }

    if (device_auth_flag) {
        device_auth();
        return;
    }

    if (counter.level > LEVEL_ONE) {
        level_two_controller();
        return;
    }

    uint16_t screen_choice = flow_level.screen_input.list_choice;

    uint16_t wallet_count = get_wallet_count();
    {
    	uint8_t valid_wallets=0;
		for (int walletIndex=0; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
			wallet_state _wallet_state = get_wallet_state(walletIndex);
			if (_wallet_state == VALID_WALLET ||
				_wallet_state == INVALID_WALLET ||
				_wallet_state == UNVERIFIED_VALID_WALLET ||
	            _wallet_state == VALID_WALLET_WITHOUT_DEVICE_SHARE) {
				valid_wallets++;
			}
		}
	    if(valid_wallets != wallet_count){
	    	wallet_count = valid_wallets;
	    }
    }

    if (screen_choice > wallet_count) { //New wallet is selected
        flow_level.level_one = screen_choice - wallet_count + 1;    // sets to LEVEL_TWO_NEW_WALLET or LEVEL_TWO_ADVANCED_SETTINGS
        if(wallet_count == MAX_WALLETS_ALLOWED){
            flow_level.level_one += 1;
        }
        if((flow_level.level_one == LEVEL_TWO_NEW_WALLET) && (get_keystore_used_count() != MAX_KEYSTORE_ENTRY)){
            tap_card_take_to_pairing();
            mark_error_screen(ui_text_error_pair_all_cards);
            return;
        }

    } else { //Old wallet is selected
        uint8_t index;
        if (get_ith_valid_wallet_index(screen_choice - 1, &index) != SUCCESS_){
            LOG_ERROR("0xx# - %d", (screen_choice - 1));
            for (int walletIndex = 0; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
                LOG_ERROR("wallet %d %d %d %s",
                    get_wallet_info(walletIndex),
                    get_wallet_card_state(walletIndex),
                    get_wallet_locked_status(walletIndex),
                    get_wallet_name(walletIndex));
            }
            cy_exit_flow();
        }
        // further down the flow, this gets overwritten by the wallet_id fetched from cards
        memcpy(wallet.wallet_id, get_wallet_id(index), WALLET_ID_SIZE);
        memcpy(
            wallet.wallet_name,
            get_wallet_name(index), NAME_SIZE);
        wallet.wallet_info = get_wallet_info(index);
        flow_level.level_one = LEVEL_TWO_OLD_WALLET;
    }
    lv_task_set_prio(listener_task, LV_TASK_PRIO_OFF); // Task will now not run
    mark_device_state(CY_TRIGGER_SOURCE | CY_APP_WAIT_USER_INPUT, 0xFF);
    increase_level_counter();
    clear_list_choice();
#endif
}
