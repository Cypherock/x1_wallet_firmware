/**
 * @file    cyt_factory_reset.c
 * @author  Cypherock X1 Team
 * @brief   Factory reset task.
 *          This file contains the implementation of the factory reset task.
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
#include "cy_factory_reset.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_instruction.h"
#include "ui_multi_instruction.h"

void cyt_factory_reset() {
  switch (flow_level.level_three) {
    case FACTORY_RESET_INFO:
      multi_instruction_init(
          ui_text_factory_reset_confirm, 3, DELAY_LONG_STRING, true);
      break;

    case FACTORY_RESET_CONFIRM:
      confirm_scr_init(ui_text_confirm_factory_reset);
      break;

    case FACTORY_RESET_TAP_CARD1:
      instruction_scr_init("Dummy", ui_text_tap_1_2_cards);
      instruction_scr_change_text(ui_text_place_card_below, true);
      mark_event_over();
      break;

    case FACTORY_RESET_TAP_CARD2:
      instruction_scr_init("Dummy", ui_text_tap_2_2_cards);
      instruction_scr_change_text(ui_text_place_card_below, true);
      mark_event_over();
      break;

    case FACTORY_RESET_CHECK:
      instruction_scr_init(ui_text_processing, NULL);
      mark_event_over();
      break;

    case FACTORY_RESET_ERASING:
      BSP_DelayMs(DELAY_SHORT);
      mark_event_over();
      break;

    case FACTORY_RESET_DONE:
      instruction_scr_destructor();
      delay_scr_init(ui_text_erasing, DELAY_TIME);
      break;

    case FACTORY_RESET_CANCEL: {
      char msg[64] = "\0";
      const char *msg_list[3] = {
          msg, ui_text_reset_exit[1], ui_text_reset_exit[2]};
      instruction_scr_destructor();
      snprintf(msg,
               sizeof(msg),
               "Wallet '%s' not found on card(s)",
               get_wallet_name(factory_reset_mismatch_wallet_index));
      multi_instruction_init(msg_list, 3, DELAY_LONG_STRING, true);
    } break;

    default:
      mark_event_over();
  }
}