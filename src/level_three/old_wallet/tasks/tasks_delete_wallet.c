/**
 * @file    tasks_delete_wallet.c
 * @author  Cypherock X1 Team
 * @brief   Delete wallet task.
 *          Handles pre-processing & display updates for delete wallet tasks.
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
#include <stdint.h>
#include <stdio.h>

#include "constant_texts.h"
#include "shamir_wrapper.h"
#include "tasks.h"
#include "tasks_tap_cards.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_list.h"
#include "ui_message.h"
#include "wallet.h"

extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;

void delete_wallet_task() {
  switch (flow_level.level_three) {
    case DELETE_WALLET_DUMMY_TASK:
      confirm_scr_render(ui_text_need_all_x1cards_to_delete_wallet_entirely);
      break;
    case DELETE_WALLET_ENTER_PIN:
      if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
        flow_level.level_three = DELETE_WALLET_DUMMY_TASK;
        break;
      }
      input_text_init(ALPHA_NUMERIC, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      break;

    case DELETE_WALLET_TAP_CARDS:
      tap_cards_for_delete_flow();
      break;

    case DELETE_WALLET_FROM_DEVICE:
      mark_event_over();
      break;

    case DELETE_WALLET_SUCCESS:
      message_scr_init(ui_text_wallet_deleted_successfully);
      break;

    default:
      reset_flow_level();
      break;
  }
}
