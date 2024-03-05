/**
 * @file    wallet_locked_tasks.c
 * @author  Cypherock X1 Team
 * @brief   Wallet unlock task.
 *          This file contains the implementation of wallet unlock task.
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
#include "pow.h"
#include "pow_utilities.h"
#include "tasks_level_four.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_text_slideshow.h"

extern Flow_level flow_level;
extern Counter counter;
extern Wallet wallet;
extern char *ALPHA_NUMERIC;

void wallet_locked_task() {
  switch (flow_level.level_three) {
    case WALLET_LOCKED_MESSAGE: {
      char slideshow[2][MAX_NUM_OF_CHARS_IN_A_SLIDE];
      uint8_t wallet_index;

      get_index_by_name((const char *)wallet.wallet_name, &wallet_index);

      convert_secs_to_time(get_wallet_time_to_unlock(wallet_index),
                           (char *)wallet.wallet_name,
                           slideshow[0]);
      snprintf(slideshow[1],
               sizeof(slideshow[1]),
               "Multiple incorrect attempts may block %s",
               wallet.wallet_name);
      char *temp[3] = {
          slideshow[0], (char *)ui_text_do_not_detach_device, slideshow[1]};
      ui_text_slideshow_init((const char **)temp, 3, DELAY_TIME, false);
      start_proof_of_work_task((const char *)wallet.wallet_name);

    } break;

    case WALLET_LOCKED_ENTER_PIN:
      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      break;

    case WALLET_LOCKED_TAP_CARD_FRONTEND: {
      Flash_Wallet *flash_wallet;
      get_flash_wallet_by_name((const char *)wallet.wallet_name, &flash_wallet);

      char msg[32];
      snprintf(msg,
               sizeof(msg),
               UI_TEXT_TAP_CARD,
               decode_card_number(flash_wallet->challenge.card_locked));

      instruction_scr_init(msg, NULL);
      mark_event_over();
    } break;

    case WALLET_LOCKED_TAP_CARD_BACKEND:
      mark_event_over();
      break;

    case WALLET_LOCKED_SUCCESS: {
      char msg[50];
      snprintf(msg,
               sizeof(msg),
               "Correct PIN! %s is now unlocked",
               wallet.wallet_name);
      delay_scr_init(msg, DELAY_TIME);
    } break;

    default:
      break;
  }
}