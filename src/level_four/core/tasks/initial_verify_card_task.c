/**
 * @file    initial_verify_card_task.c
 * @author  Cypherock X1 Team
 * @brief   Verify card task (initial).
 *          This file contains the task used to verify the card in the initial application.
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
#include "tasks_level_four.h"
#include "ui_instruction.h"
#include "ui_delay.h"
#include "ui_confirmation.h"
#include "communication.h"
#include "tasks.h"




extern uint8_t auth_card_number;

void initial_verify_card_task() {
#if X1WALLET_INITIAL
	switch(flow_level.level_three) {

    case VERIFY_CARD_START_MESSAGE: {
            char str[50], heading[50];
            snprintf(str, sizeof(str), UI_TEXT_PLACE_CARD_TILL_BEEP, 3);
            snprintf(heading, sizeof(heading), UI_TEXT_PLACE_CARD, auth_card_number);
            instruction_scr_init(str,heading);
            mark_event_over();
          } break;

    case VERIFY_CARD_ESTABLISH_CONNECTION_FRONTEND: {
            char str[50], heading[50];
            snprintf(str, sizeof(str), UI_TEXT_PLACE_CARD_TILL_BEEP, 3);
            snprintf(heading, sizeof(heading), UI_TEXT_PLACE_CARD,
                     auth_card_number);
            instruction_scr_destructor();
            instruction_scr_init("Dummy", heading);
            instruction_scr_change_text(str,true);
            BSP_DelayMs(500);
            mark_event_over();
          } break;

    case VERIFY_CARD_ESTABLISH_CONNECTION_BACKEND:
    case VERIFY_CARD_FETCH_RANDOM_NUMBER:
        mark_event_over();
        break;

    case VERIFY_CARD_SIGN_RANDOM_NUMBER_FRONTEND: {
      char str[50], heading[50];
      snprintf(str, sizeof(str), UI_TEXT_PLACE_CARD_TILL_BEEP, 2);
      snprintf(heading, sizeof(heading), UI_TEXT_PLACE_CARD,
               auth_card_number);
      instruction_scr_destructor();
      instruction_scr_init("Dummy", heading);
      instruction_scr_change_text(str, true);
      BSP_DelayMs(500);
      mark_event_over();
    } break;

    case VERIFY_CARD_SIGN_RANDOM_NUMBER_BACKEND:
    case VERIFY_CARD_AUTH_STATUS:
        mark_event_over();
        break;

    case VERIFY_CARD_PAIR_FRONTEND: {
      char str[50], heading[50];
      snprintf(str, sizeof(str), UI_TEXT_PLACE_CARD_TILL_BEEP, 1);
      snprintf(heading, sizeof(heading), UI_TEXT_PLACE_CARD,
               auth_card_number);

      instruction_scr_destructor();
      instruction_scr_init("Dummy", heading);
      instruction_scr_change_text(str, true);
      BSP_DelayMs(500);
      mark_event_over();
    } break;

    case VERIFY_CARD_PAIR_BACKEND:
    case VERIFY_CARD_FINAL_MESSAGE:
        mark_event_over();
        break;

    case VERIFY_CARD_FAILED:
        instruction_scr_destructor();
        delay_scr_init(ui_text_card_authentication_failed, DELAY_TIME);
        break;

    default:
        break;
	}
#endif
}
