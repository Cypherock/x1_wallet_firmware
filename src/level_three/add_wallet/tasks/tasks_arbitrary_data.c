/**
 * @file    tasks_arbitrary_data.c
 * @author  Cypherock X1 Team
 * @brief   Arbitrary data tasks.
 *          This file contains the implementation of the tasks that are used to
 *create arbitrary data.
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
#include "shamir_wrapper.h"
#include "stdint.h"
#include "tasks.h"
#include "tasks_add_wallet.h"
#include "tasks_tap_cards.h"
#include "ui_address.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_input_mnemonics.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_list.h"
#include "ui_menu.h"
#include "ui_message.h"
#include "ui_multi_instruction.h"
#include "ui_text_slideshow.h"
#include "wallet.h"

extern char arbitrary_data[4096 / 8 + 1];

extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;
extern char *PASSPHRASE;

#ifdef USE_ARBITRARY_DATA
static void restore_wallet_enter_mnemonics_flow() {
  if (flow_level.level_four <= wallet.number_of_mnemonics) {
    char heading[20];
    snprintf(
        heading, sizeof(heading), UI_TEXT_ENTER_WORD, flow_level.level_four);
    ui_mnem_init(heading);
  } else {
    // todo check if mnemonics is correct
    mark_event_over();
  }
}
#endif

void tasks_arbitrary_data() {
  if (flow_level.show_error_screen) {
    message_scr_init(flow_level.error_screen_text);
    return;
  }

  switch (flow_level.level_three) {
    case ARBITRARY_DATA_NAME_INPUT: {
      input_text_init(WALLET_NAME_CHARSET,
                      0,
                      ui_text_enter_wallet_name,
                      2,
                      DATA_TYPE_TEXT,
                      15);
    } break;

    case ARBITRARY_DATA_NAME_CONFIRM: {
      char display[65];
      snprintf(
          display, sizeof(display), "%s", flow_level.screen_input.input_text);
      address_scr_init(ui_text_confirm_wallet_name, display, false);
    } break;

    case RESTORE_WALLET_PIN_INSTRUCTIONS_1: {
      char display[65];
      if (strnlen(flow_level.screen_input.input_text,
                  sizeof(flow_level.screen_input.input_text)) <= 15)
        snprintf(
            display, sizeof(display), UI_TEXT_PIN_INS1, wallet.wallet_name);
      else
        snprintf(display, sizeof(display), UI_TEXT_PIN_INS1, "this wallet");
      delay_scr_init(display, DELAY_TIME);
    } break;

    case RESTORE_WALLET_PIN_INSTRUCTIONS_2: {
      delay_scr_init(ui_wallet_pin_instruction_2, DELAY_TIME);
    } break;

    case ARBITRARY_DATA_SKIP_PIN: {
      confirm_scr_init(ui_text_do_you_want_to_set_pin);
      confirm_scr_focus_cancel();
    } break;

    case ARBITRARY_DATA_PIN_INPUT: {
      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
    } break;

    case ARBITRARY_DATA_PIN_CONFIRM: {
      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_confirm_pin, 4, DATA_TYPE_PIN, 8);
    } break;

    case ARBITRARY_DATA_ENTER_DATA_INSTRUCTION: {
      message_scr_init(ui_text_now_enter_your_data);

    } break;

    case ARBITRARY_DATA_ENTER_DATA: {
      input_text_init(
          PASSPHRASE, 26, ui_text_enter_data, 0, DATA_TYPE_PASSPHRASE, 255);
    } break;

    case ARBITRARY_DATA_CONFIRM_DATA: {
      snprintf(arbitrary_data,
               sizeof(arbitrary_data),
               "%s",
               flow_level.screen_input.input_text);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      address_scr_init(ui_text_confirm_data, arbitrary_data, false);
    } break;

    case ARBITRARY_DATA_CREATING_WAIT_SCREEN: {
      instruction_scr_init(ui_text_processing, NULL);
      mark_event_over();
    } break;

    case ARBITRARY_DATA_CREATE: {
      instruction_scr_destructor();
      mark_event_over();
    } break;

    case ARBITRARY_DATA_TAP_CARDS: {
      tap_cards_for_write_flow();
    } break;

    case ARBITRARY_DATA_VERIFY_SHARES:
      instruction_scr_init(ui_text_processing, "");
      instruction_scr_change_text(ui_text_processing, true);
      BSP_DelayMs(DELAY_SHORT);
      mark_event_over();
      break;

    case ARBITRARY_DATA_SUCCESS_MESSAGE: {
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

    case ARBITRARY_DATA_FAILED_MESSAGE: {
      instruction_scr_destructor();
      message_scr_init(ui_text_creation_failed_delete_wallet);
    } break;

    default: {
      message_scr_init(ui_text_something_went_wrong);
    } break;
  }
  return;
}
