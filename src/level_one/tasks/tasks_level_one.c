/**
 * @file    tasks_level_one.c
 * @author  Cypherock X1 Team
 * @brief   Level one task (main).
 *          Handles display updates for level one tasks of the main application.
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

#include "tasks_level_one.h"
#include "flash_api.h"
#include "tasks_level_two.h"
#include "ui_confirmation.h"
#include "ui_menu.h"
#include "ui_message.h"
#include "apdu.h"

extern lv_task_t* listener_task;

extern uint8_t device_auth_flag;

void level_one_tasks()
{
#if X1WALLET_MAIN
    if (flow_level.show_error_screen) {
        mark_device_state(CY_TRIGGER_SOURCE | CY_APP_WAIT_USER_INPUT, 0xFF);
        message_scr_init(flow_level.error_screen_text);
        return;
    }

    if (get_card_data_health() == DATA_HEALTH_CORRUPT) {
        mark_device_state(CY_TRIGGER_SOURCE | CY_APP_WAIT_USER_INPUT, 0xFF);
        message_scr_init(ui_text_unreliable_cards);
        return;
    }

    if (flow_level.show_desktop_start_screen) {
        mark_device_state(CY_TRIGGER_SOURCE | CY_APP_WAIT_USER_INPUT, 0);
        confirm_scr_init(flow_level.confirmation_screen_text);
        return;
    }

    if (device_auth_flag) {
        mark_event_over();
        return;
    }

    if (counter.level > LEVEL_ONE) {
        level_two_tasks();
        return;
    }

    uint8_t number_of_options = get_wallet_count();
    LOG_INFO("wallet count %d", number_of_options);

    // create list for chooses
    char* choices[MAX_LEN_OF_MENU_OPTIONS];

    uint8_t mainMenuIndex = 0;
    uint8_t walletIndex = 0;

    for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
        if (get_wallet_state(walletIndex) == VALID_WALLET
            || get_wallet_state(walletIndex) == UNVERIFIED_VALID_WALLET
            || get_wallet_state(walletIndex) == VALID_WALLET_WITHOUT_DEVICE_SHARE) {
            choices[mainMenuIndex] = (char*)get_wallet_name(walletIndex);
            mainMenuIndex++;
        }
    }

    if(mainMenuIndex != number_of_options){
    	number_of_options = mainMenuIndex;
    }

    number_of_options += NUMBER_OF_OPTIONS_MAIN_MENU;

    uint8_t walletMenuOptionIndex = 0;
    
    //initialise walletMenuOptionIndex to 0 if wallet limit is exceeded
    if(number_of_options > 5){
        walletMenuOptionIndex = 2;
    }else{
        walletMenuOptionIndex = 1;
    }

    //check if wallet limit is exceeded or not
    if(number_of_options > 5){
        number_of_options = 5;
    }
    // fill other options
    for (; walletMenuOptionIndex <= NUMBER_OF_OPTIONS_MAIN_MENU; walletMenuOptionIndex++) {
        choices[mainMenuIndex] = (char*)ui_text_options_main_menu[walletMenuOptionIndex];
        mainMenuIndex++;
    }
    menu_init((const char **) choices, number_of_options, ui_text_options_main_menu[0], false);
    lv_task_set_prio(listener_task, LV_TASK_PRIO_MID);
    CY_Reset_Not_Allow(false);
    mark_device_state(CY_APP_IDLE_TASK | CY_APP_IDLE, 0xFF);
#endif
}
