/**
 * @file    pair_card_tasks.c
 * @author  Cypherock X1 Team
 * @brief   Pair card task.
 *          This file contains the pair card task.
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
#include "controller_main.h"
#include "events.h"
#include "nfc_events.h"
#include "tasks_tap_cards.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_events.h"
#include "ui_instruction.h"
#include "ui_skip_instruction.h"

void tap_card_pair_card_tasks() {
  char display[40];
  switch (flow_level.level_four) {
    case PAIR_CARD_TAP_A_CARD_DUMMY:
      confirm_scr_init(ui_text_continue_with_pairing);
      break;

    case PAIR_CARD_RED_FRONTEND:
    case PAIR_CARD_BLUE_FRONTEND:
    case PAIR_CARD_GREEN_FRONTEND:
    case PAIR_CARD_YELLOW_FRONTEND:
      if (get_keystore_used_status((flow_level.level_four - 1) >> 1) == 1) {
        mark_event_cancel();
      } else {
        snprintf(display,
                 sizeof(display),
                 UI_TEXT_PAIRING_TAP_CARD,
                 ((flow_level.level_four - 1) >> 1) + 1);
        skip_instruction_scr_init(display);

        nfc_enable_card_detect_event();

        evt_config_t evt_conf = {.abort_disabled = true,
                                 .evt_selection.bits.ui_events = true,
                                 .evt_selection.bits.nfc_events = true,
                                 .evt_selection.bits.usb_events = false,
                                 .timeout = 30000};
        evt_status_t evt_status = {0};
        get_events(evt_conf, &evt_status);

        nfc_disable_card_detect_event();
        if (evt_status.nfc_event.event_occured) {
          skip_instruction_scr_destructor();
          ui_reset_event();
          instruction_scr_init("Dummy", display);
          instruction_scr_change_text("...", true);
          mark_event_over();
        } else if (evt_status.ui_event.event_occured &&
                   evt_status.ui_event.event_type == UI_EVENT_REJECT) {
          mark_event_cancel();
        }
      }
      break;

    case PAIR_CARD_RED_BACKEND:
    case PAIR_CARD_BLUE_BACKEND:
    case PAIR_CARD_GREEN_BACKEND:
    case PAIR_CARD_YELLOW_BACKEND:
      snprintf(display,
               sizeof(display),
               UI_TEXT_PAIRING_TAP_CARD,
               flow_level.level_four >> 1);
      mark_event_over();
      break;

    case PAIR_CARD_SUCCESS_MESSAGE: {
      uint8_t pair_count = 0;
      if ((pair_count = get_keystore_used_count()) < 4) {
        char str[100] = "";
        snprintf(str,
                 sizeof(str),
                 "%d card pairing skipped, pair all cards for proper use",
                 4 - pair_count);
        delay_scr_init(str, DELAY_LONG_STRING);
      } else {
        delay_scr_init(ui_text_card_pairing_success, DELAY_TIME);
      }
    } break;

    default:
      break;
  }
}