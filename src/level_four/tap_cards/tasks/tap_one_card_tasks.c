/**
 * @file    tap_one_card_tasks.c
 * @author  Cypherock X1 Team
 * @brief   Tap one card task.
 *          This file contains the implementation of the tap one card tasks.
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
#include "flash_if.h"
#include "flash_api.h"
#include "nfc.h"
#include "tasks.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_message.h"
#include "utils.h"
#include "pow_utilities.h"
#include "buzzer.h"
#include "tasks_tap_cards.h"


extern char* ALPHABET;
extern char* ALPHA_NUMERIC;
extern char* NUMBERS;
extern char* HEX;
char card_id_fetched[2 * CARD_ID_SIZE + 1];
char card_version[2 * CARD_VERSION_SIZE + 1];


extern bool no_wallet_on_cards;
void tap_a_card_and_sync_task()
{
    switch (flow_level.level_three) { // revert back from level_three to level_four if broken
    case TAP_ONE_CARD_TAP_A_CARD_FRONTEND:
        instruction_scr_init(ui_text_tap_a_card);
        mark_event_over();
        break;
    case TAP_ONE_CARD_TAP_A_CARD_BACKEND:
        mark_event_over();
        break;
    case TAP_ONE_CARD_SUCCESS_MESSAGE:
        if(no_wallet_on_cards == true){
            reset_flow_level();
            flow_level.show_error_screen = true;
            snprintf(flow_level.error_screen_text, 90, "%s", ui_text_wallet_not_found_on_cycard);
        }
        else{
            delay_scr_init(ui_text_sync_wallets_next_steps, DELAY_TIME);
        }

        break;
    default:
        break;
    }
}

static void get_card_version(char * arr, char message[22]){
    snprintf(message, 22, "CyCard version\n ");
    message[15] = arr[0],message[16] = '.',message[17] = arr[1] , message[18] = '.' ;
    if(arr[2] != '0')
        message[19] = arr[2] , message[20] = arr[3];
    else
        message[19] = arr[3],message[20] = 0;

}

void tasks_read_card_id()
{
    switch (flow_level.level_three) { // revert back from level_three to level_four if broken
    case TAP_ONE_CARD_TAP_A_CARD_FRONTEND:
        instruction_scr_init(ui_text_tap_a_card);
        mark_event_over();
        break;
    case TAP_ONE_CARD_TAP_A_CARD_BACKEND:
        mark_event_over();
        break;
    case TAP_ONE_CARD_SUCCESS_MESSAGE:{
        char message[22];
        get_card_version(card_version, message);
        message_scr_init((const char *)message);
        break;
    
    default:
        break;
    }
    }
}

void tasks_update_card_id()
{
    switch (flow_level.level_three) {
    case 1: {
        input_text_init(
            HEX,
            ui_text_family_id_hex,
            10,
            DATA_TYPE_TEXT,
            8);
    }

    break;

    case 2: {
        instruction_scr_init(ui_text_tap_a_card);
        mark_event_over();
    } break;

    case 3: {
        mark_event_over();
    } break;

    case 4:
        message_scr_init(ui_text_successfull);
        break;
    
    default:
        break;
    }
}
