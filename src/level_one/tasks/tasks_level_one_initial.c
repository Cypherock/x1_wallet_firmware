/**
 * @file    tasks_level_one_initial.c
 * @author  Cypherock X1 Team
 * @brief   Level one task (initial).
 *          Handles display updates for level one tasks of the initial application.
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
#include "flash_struct.h"
#include "tasks_level_one.h"
#include "tasks_level_two.h"
#include "tasks_tap_cards.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_instruction.h"
#include "ui_menu.h"
#include "ui_message.h"
#include "ui_multi_instruction.h"
#include "ui_scroll_page.h"

extern lv_task_t *listener_task;

void level_one_tasks_initial() {
#if X1WALLET_INITIAL
  // It is safe to allow reset as all the flows are desktop triggered.
  CY_Reset_Not_Allow(false);
  // Since all desktop flows are handled via advanced settings, we can safely say that the app is busy.
  mark_device_state(flow_level.level_one == LEVEL_TWO_ADVANCED_SETTINGS
                        ? (CY_TRIGGER_SOURCE | CY_APP_IDLE)
                        : (CY_APP_IDLE_TASK | CY_APP_IDLE),
                    0xFF);
  if (flow_level.show_error_screen) {
    message_scr_init(flow_level.error_screen_text);
    return;
  }

  if (flow_level.show_desktop_start_screen) {
    if (flow_level.level_two == LEVEL_THREE_RESET_DEVICE_CONFIRM) {
        message_scr_init(flow_level.confirmation_screen_text);
    } else {
        ui_scrollable_page(ui_heading_confirm_action, flow_level.confirmation_screen_text,
                           MENU_SCROLL_HORIZONTAL, false);
    }
    return;
  }

  if (counter.level > LEVEL_ONE) {
    level_two_tasks();
    return;
  }

  lv_task_set_prio(listener_task, LV_TASK_PRIO_MID);

  switch (flow_level.level_one) {
    case 1: {
      delay_scr_init("WELCOME", DELAY_TIME);
    } break;

    case 2: {
      multi_instruction_init(ui_text_startup_instruction_screen_2, 5U,
                             DELAY_LONG_STRING, true);
    } break;

    case 3: {
      delay_scr_init(ui_text_tap_a_card_instruction1, DELAY_LONG_STRING);
    } break;

    case 4: {
      flow_level.level_three = TAP_ONE_CARD_TAP_A_CARD_FRONTEND;
      tasks_read_card_id();
      instruction_scr_destructor();
      instruction_scr_init(ui_text_tap_a_card_instruction2, NULL);
    } break;

    case 5: {
      flow_level.level_three = TAP_ONE_CARD_TAP_A_CARD_BACKEND;
      mark_event_over();
    } break;

    case 6: {
      multi_instruction_init(ui_text_startup_instruction_screen_4, 2U,
                             DELAY_TIME, false);
    } break;

    case 7: {
      instruction_scr_init(ui_text_check_cysync_app, NULL);
    } break;

    case 8: {
      instruction_scr_init(ui_text_device_verification_success, NULL);
    } break;

    case 9: {
      instruction_scr_init(ui_text_device_verification_failure, NULL);
    } break;

    case 10: {
      instruction_scr_init(ui_text_provision_success, NULL);
    } break;

    case 11: {
      instruction_scr_init(ui_text_provision_fail, NULL);
    } break;

    default:
      break;
  }
#endif
}
