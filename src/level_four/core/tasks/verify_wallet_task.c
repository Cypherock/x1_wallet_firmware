/**
 * @file    verify_wallet_task.c
 * @author  Cypherock X1 Team
 * @brief   Verify wallet task.
 *          This file contains the implementation of the verify wallet task.
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
#include "bip39.h"
#include "controller_main.h"
#include "flash_api.h"
#include "shamir_wrapper.h"
#include "tasks_level_four.h"
#include "tasks_tap_cards.h"
#include "ui_address.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_list.h"
#include "ui_message.h"
#include "ui_multi_instruction.h"

extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;

extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;

void verify_wallet_tasks() {
  switch (flow_level.level_three) {
    case VERIFY_WALLET_START:
      mark_event_over();
      break;

    case VERIFY_WALLET_PIN_INPUT:
      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      break;

    case VERIFY_WALLET_TAP_CARDS_FLOW:
      tap_cards_for_verification_flow();
      break;

    case VERIFY_WALLET_DATA: {
      instruction_scr_init(ui_text_processing, "");
      instruction_scr_change_text(ui_text_processing, true);
      BSP_DelayMs(DELAY_SHORT);
      mark_event_over();
    } break;

    case VERIFY_WALLET_SUCCESS: {
      instruction_scr_destructor();
      const char *messages[6] = {
          ui_text_verification_is_now_complete_messages[0],
          ui_text_verification_is_now_complete_messages[1],
          ui_text_verification_is_now_complete_messages[2],
          ui_text_verification_is_now_complete_messages[4],
          ui_text_verification_is_now_complete_messages[5],
          NULL};
      uint8_t count = 5;

      if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
        messages[3] = ui_text_verification_is_now_complete_messages[3];
        messages[4] = ui_text_verification_is_now_complete_messages[4];
        messages[5] = ui_text_verification_is_now_complete_messages[5];
        count = 6;
      }

      multi_instruction_init(messages, count, DELAY_LONG_STRING, true);
    } break;

    case VERIFY_WALLET_DELETE:
      instruction_scr_destructor();
      address_scr_init(ui_text_verification_cancelled,
                       (char *)ui_text_delete_this_wallet,
                       false);
      break;

    default:
      break;
  }
}
