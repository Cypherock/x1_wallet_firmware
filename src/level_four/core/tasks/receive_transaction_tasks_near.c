/**
 * @file    receive_transaction_tasks_near.c
 * @author  Cypherock X1 Team
 * @brief   Receive transaction for NEAR
 *          This file contains the implementation of the receive transaction for
 *NEAR.
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
#include "near_context.h"
#include "tasks_level_four.h"
#include "tasks_tap_cards.h"
#include "ui_address.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_menu.h"

extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;
extern char *PASSPHRASE;

extern lv_task_t *timeout_task;

extern Receive_Transaction_Data receive_transaction_data;

void receive_transaction_tasks_near() {
  switch (flow_level.level_three) {
    case RECV_TXN_FIND_XPUB_NEAR: {
      mark_event_over();
    } break;

    case RECV_TXN_ENTER_PASSPHRASE_NEAR: {
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

    case RECV_TXN_CONFIRM_PASSPHRASE_NEAR: {
      char display[65];
      snprintf(
          display, sizeof(display), "%s", flow_level.screen_input.input_text);
      address_scr_init(ui_text_confirm_passphrase, display, false);
      memzero(display, sizeof(display));
    } break;

    case RECV_TXN_CHECK_PIN_NEAR: {
      mark_event_over();
    } break;

    case RECV_TXN_ENTER_PIN_NEAR: {
      // TODO: Redundant check
      if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
        flow_level.level_three = RECV_TXN_CHECK_PIN_NEAR;
        break;
      }
      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
    } break;

    case RECV_TXN_TAP_CARD_NEAR: {
      // TODO: Shorten func name
      tap_threshold_cards_for_reconstruction();
    } break;

    case RECV_TXN_TAP_CARD_SEND_CMD_NEAR: {
      mark_event_over();
    } break;

    case RECV_TXN_READ_DEVICE_SHARE_NEAR: {
      mark_event_over();
    } break;

    case RECV_TXN_DERIVE_ADD_SCREEN_NEAR: {
      instruction_scr_init("", NULL);
      instruction_scr_change_text(ui_text_processing, true);
      BSP_DelayMs(DELAY_SHORT);
      mark_event_over();
    } break;

    case RECV_TXN_DERIVE_ADD_NEAR: {
      mark_event_over();
    } break;

    case RECV_TXN_WAIT_FOR_LINK_NEAR: {
      instruction_scr_change_text(ui_text_waiting_for_desktop, true);
      mark_event_over();
    } break;

    case RECV_TXN_DISPLAY_ACC_NEAR: {
      char display[110];
      snprintf(display,
               sizeof(display),
               "%s%s%s",
               ui_text_20_spaces,
               ui_text_20_spaces,
               receive_transaction_data.near_registered_account);
      address_scr_init(ui_text_new_account_id, display, true);
    } break;

    case RECV_TXN_DISPLAY_ADDR_NEAR: {
      char display[110];
      char heading[20] = {0};
      snprintf(heading, sizeof(heading), "%s", ui_text_receive_on);
      char address_s[2 * sizeof(receive_transaction_data.near_pubkey) + 1] =
          "ed25519:";
      if (receive_transaction_data.near_account_type == 1) {
        if (receive_transaction_data.near_acc_found) {
          memcpy(address_s,
                 receive_transaction_data.near_registered_account,
                 sizeof(receive_transaction_data.near_registered_account));
        } else {
          size_t sz = sizeof(address_s) - 8;
          b58enc(address_s + 8, &sz, receive_transaction_data.near_pubkey, 32);
          snprintf(heading, sizeof(heading), "%s", ui_text_new_public_key);
        }
      } else {
        byte_array_to_hex_string(receive_transaction_data.near_pubkey,
                                 sizeof(receive_transaction_data.near_pubkey),
                                 address_s,
                                 sizeof(address_s));
      }
      snprintf(display,
               sizeof(display),
               "%s%s%s",
               ui_text_20_spaces,
               ui_text_20_spaces,
               address_s);

      address_scr_init(heading, display, true);

    } break;

    case RECV_TXN_WAIT_FOR_REPLACE_NEAR_SCREEN: {
      instruction_scr_init(ui_text_waiting_for_desktop, NULL);
      mark_event_over();
    } break;

    case RECV_TXN_WAIT_FOR_REPLACE_NEAR: {
      mark_event_over();
    } break;

    case RECV_TXN_SELECT_REPLACE_ACC_NEAR: {
      char *acc_id[NEAR_REGISTERED_ACCOUNT_COUNT] = {0};

      near_deserialize_account_ids(coin_specific_data.coin_data,
                                   NEAR_COIN_DATA_MAX_LEN,
                                   acc_id,
                                   receive_transaction_data.near_acc_count);

      menu_init((const char **)acc_id,
                NEAR_REGISTERED_ACCOUNT_COUNT,
                ui_text_replace_account,
                true);

    } break;

    case RECV_TXN_VERIFY_SAVE_ACC_NEAR: {
      receive_transaction_data.near_acc_index =
          flow_level.screen_input.list_choice - 1;

      char *acc_id[NEAR_REGISTERED_ACCOUNT_COUNT] = {0};

      near_deserialize_account_ids(coin_specific_data.coin_data,
                                   NEAR_COIN_DATA_MAX_LEN,
                                   acc_id,
                                   receive_transaction_data.near_acc_count);

      address_scr_init(ui_text_confirm_account,
                       acc_id[receive_transaction_data.near_acc_index],
                       false);

    } break;

    case RECV_TXN_FINAL_SCREEN_NEAR: {
      // this exists to handle multiple exit points in near's receive flow
      mark_event_over();
    } break;

    default:
      break;
  }
  return;
}
