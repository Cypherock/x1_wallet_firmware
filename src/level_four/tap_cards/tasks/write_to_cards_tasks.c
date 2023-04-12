/**
 * @file    write_to_cards_tasks.c
 * @author  Cypherock X1 Team
 * @brief   Write to cards.
 *          This file contains the task handler to write to cards.
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
#include "constant_texts.h"
#include "controller_main.h"
#include "flash_api.h"
#include "nfc.h"
#include "stdint.h"
#include "tasks.h"
#include "tasks_tap_cards.h"
#include "ui_instruction.h"
#include "ui_message.h"

extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;

void tap_cards_for_write_flow() {
  switch (flow_level.level_four) {
    case CARD_ONE_FRONTEND:
    case CARD_TWO_FRONTEND:
    case CARD_THREE_FRONTEND:
    case CARD_FOUR_FRONTEND: {
      char display[40];
      snprintf(display,
               sizeof(display),
               UI_TEXT_TAP_CARD,
               ((flow_level.level_four - 1) / 3) + 1);
      instruction_scr_init(ui_text_place_card_below, display);
      mark_event_over();
    } break;

    case CARD_ONE_READBACK:
    case CARD_TWO_READBACK:
    case CARD_THREE_READBACK:
    case CARD_FOUR_READBACK: {
      char display[40];
      instruction_scr_destructor();
      snprintf(display,
               sizeof(display),
               UI_TEXT_TAP_CARD,
               flow_level.level_four / 3);
      instruction_scr_init(ui_text_place_card_below, display);
      if (nfc_wait_for_card(DEFAULT_NFC_TG_INIT_TIME) != STM_SUCCESS)
        instruction_scr_change_text(ui_text_card_removed_fast, true);
    }
    case CARD_ONE_WRITE:
    case CARD_TWO_WRITE:
    case CARD_THREE_WRITE:
    case CARD_FOUR_WRITE:
      mark_event_over();
      break;

    default:
      LOG_CRITICAL("xx30");
      reset_flow_level();
      message_scr_init(ui_text_something_went_wrong);
      break;
  }
}
