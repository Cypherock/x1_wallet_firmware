/**
 * @file    controller_level_two.c
 * @author  Cypherock X1 Team
 * @brief   Level two next controller.
 *          Handles post event (only next events) operations for level two
 *tasks.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
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
#include "controller_level_two.h"

#include "communication.h"
#include "constant_texts.h"
#include "controller_add_wallet.h"
#include "controller_advanced_settings.h"
#include "controller_old_wallet.h"
#include "flash_api.h"
#include "tasks.h"

extern lv_task_t *listener_task;

void level_two_controller() {
  if (flow_level.show_error_screen) {
    flow_level.show_error_screen = false;
    return;
  }

  switch (flow_level.level_one) {
#if X1WALLET_MAIN
    case LEVEL_TWO_OLD_WALLET: {
      if (counter.level > LEVEL_TWO) {
        level_three_old_wallet_controller();
        return;
      }

      flow_level.level_two = flow_level.screen_input.list_choice;
      counter.level =
          LEVEL_THREE;    // increase level counter from two to three
      lv_task_set_prio(listener_task, LV_TASK_PRIO_OFF);
      mark_device_state(CY_TRIGGER_SOURCE | CY_APP_BUSY, 0xFF);
    } break;

    case LEVEL_TWO_NEW_WALLET: {
      if (counter.level > LEVEL_TWO) {
        if (flow_level.level_two == LEVEL_THREE_GENERATE_WALLET) {
          generate_wallet_controller();
        } else {
          restore_wallet_controller();
        }
        return;
      }

      if (get_wallet_count() == MAX_WALLETS_ALLOWED) {
        mark_error_screen(ui_text_already_have_maxi_wallets);
        decrease_level_counter();
        break;
      }

      flow_level.level_two = flow_level.screen_input.list_choice;
      // level_four variable to be used as progress tracker for wallet
      // generation

      if (flow_level.level_two == LEVEL_THREE_GENERATE_WALLET) {
        flow_level.level_three = GENERATE_WALLET_NAME_INPUT;
      } else {
        flow_level.level_three = RESTORE_WALLET_NAME_INPUT;
      }
      increase_level_counter();
    } break;
#endif

    case LEVEL_TWO_ADVANCED_SETTINGS: {
      if (counter.level > LEVEL_TWO) {
        level_three_advanced_settings_controller();
        return;
      }
      flow_level.level_two = flow_level.screen_input.list_choice;
      increase_level_counter();
    } break;

    default:
      break;
  }
}