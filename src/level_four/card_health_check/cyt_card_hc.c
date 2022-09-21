/**
 * @file    cyt_card_hc.c
 * @author  Cypherock X1 Team
 * @brief   Card health check task.
 *          This file contains the implementation of the card health check task.
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
#include "flash_config.h"
#include "cy_card_hc.h"
#include "ui_instruction.h"
#include "ui_delay.h"
#include "ui_list.h"
#include "ui_message.h"
#include "controller_tap_cards.h"
#include "app_error.h"
#include "ui_multi_instruction.h"

extern uint8_t *wallet_list[MAX_WALLETS_ALLOWED][2];
extern uint8_t wallet_count;
extern uint32_t card_fault_status;
void cyt_card_hc() {
#if X1WALLET_MAIN
    switch (flow_level.level_three) {
        case CARD_HC_START:
            mark_event_over();
            break;

        case CARD_HC_TAP_CARD:{
            instruction_scr_init(ui_text_place_card_below, ui_text_tap_a_card);
            instruction_scr_change_text(ui_text_card_health_check_start, true);
            mark_event_over();
        } break;

        case CARD_HC_DISPLAY_CARD_HEALTH: {
            char error_scr_list[3][MAX_NUM_OF_CHARS_IN_AN_INSTRUCTION]={"","",""};
            const char *pptr[3] = {error_scr_list[0], error_scr_list[1], error_scr_list[2]};
            uint8_t str_index=0;
            if(card_fault_status == 0){
                pptr[str_index++] = ui_text_card_seems_healthy;
                pptr[str_index++] = ui_text_click_to_view_wallets;
            }
            else{
                snprintf(error_scr_list[str_index++], MAX_NUM_OF_CHARS_IN_AN_INSTRUCTION, "%s: C%04lx", ui_text_card_health_check_error[0], card_fault_status);
                pptr[str_index++] = ui_text_card_health_check_error[1];
                if(wallet_count > MAX_WALLETS_ALLOWED)
                    pptr[str_index++] = ui_text_no_wallets_fetched;
                else
                    pptr[str_index++] = ui_text_click_to_view_wallets;
            }
            multi_instruction_init(pptr, str_index, DELAY_TIME, true);
        } break;

        case CARD_HC_DISPLAY_WALLETS:{
            if(wallet_count == 0){
                message_scr_init(ui_text_no_wallets_present);
            }
            else if(wallet_count <= MAX_WALLETS_ALLOWED){
                char choices[MAX_WALLETS_ALLOWED][16]={"","","",""}, heading[50];
                for(int i=0; i < wallet_count; i++){
                    strcpy(choices[i], (char*)wallet_list[i][0]);
                }
                snprintf(heading, sizeof(heading), ui_text_wallets_in_card, decode_card_number(tap_card_data.tapped_card));
                list_init(choices, wallet_count, heading, false);
            }
            else{
                mark_event_cancel();
            }
        } break;

        default:
            reset_flow_level();
    }
#endif
}