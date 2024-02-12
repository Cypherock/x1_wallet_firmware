/**
 * @file    receive_transaction_tasks_solana.c
 * @author  Cypherock X1 Team
 * @brief   Receive transaction task for SOLANA.
 *          This file contains the implementation of the receive transaction
 *task for SOLANA.
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
#include "communication.h"
#include "constant_texts.h"
#include "controller_level_four.h"
#include "flash_api.h"
#include "tasks_level_four.h"
#include "tasks_tap_cards.h"
#include "ui_address.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_menu.h"
#include "utils.h"

extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;
extern char *PASSPHRASE;

extern Receive_Transaction_Data receive_transaction_data;

void receive_transaction_tasks_solana() {
  switch (flow_level.level_three) {
    case RECV_TXN_FIND_XPUB_SOLANA: {
      mark_event_over();
    } break;

    case RECV_TXN_ENTER_PASSPHRASE_SOLANA: {
      if (!WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
        flow_level.level_three = ADD_COINS_VERIFY;
        break;
      }
      input_text_init(PASSPHRASE,
                      26,
                      ui_text_enter_passphrase,
                      0,
                      DATA_TYPE_PASSPHRASE,
                      64);
    } break;

    case RECV_TXN_CONFIRM_PASSPHRASE_SOLANA: {
      char display[65] = {0};
      snprintf(
          display, sizeof(display), "%s", flow_level.screen_input.input_text);
      address_scr_init(ui_text_confirm_passphrase, display, false);
      memzero(display, sizeof(display));
    } break;

    case RECV_TXN_CHECK_PIN_SOLANA: {
      mark_event_over();
    } break;

    case RECV_TXN_ENTER_PIN_SOLANA: {
      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
    } break;

    case RECV_TXN_TAP_CARD_SOLANA: {
      retrieve_key_from_card();
    } break;

    case RECV_TXN_TAP_CARD_SEND_CMD_SOLANA: {
      mark_event_over();
    } break;

    case RECV_TXN_READ_DEVICE_SHARE_SOLANA: {
      mark_event_over();
    } break;

    case RECV_TXN_DERIVE_ADD_SCREEN_SOLANA: {
      instruction_scr_init(ui_text_processing, NULL);
      instruction_scr_change_text(ui_text_processing, true);
      BSP_DelayMs(DELAY_SHORT);
      mark_event_over();
    } break;

    case RECV_TXN_DERIVE_ADD_SOLANA: {
      mark_event_over();
    } break;

    case RECV_TXN_DISPLAY_ADDR_SOLANA: {
      instruction_scr_destructor();
      char display[70] = {0};
      snprintf(display,
               sizeof(display),
               "%s%s",
               ui_text_20_spaces,
               receive_transaction_data.solana_address);
      address_scr_init(ui_text_receive_on, display, true);
    } break;

    default:
      break;
  }

  return;
}
