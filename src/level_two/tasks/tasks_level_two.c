/**
 * @file    tasks_level_two.c
 * @author  Cypherock X1 Team
 * @brief   Level two task.
 *          Handles pre-processing & display updates for level two tasks.
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
#include "tasks_level_two.h"

#include "communication.h"
#include "constant_texts.h"
#include "flash_api.h"
#include "tasks.h"
#include "tasks_add_wallet.h"
#include "tasks_advanced_settings.h"
#include "tasks_old_wallet.h"
#include "ui_confirmation.h"
#include "ui_menu.h"
#include "ui_message.h"

extern lv_task_t *listener_task;

void level_two_tasks() {
  if (flow_level.show_error_screen) {
    message_scr_init(flow_level.error_screen_text);
    return;
  }
#if X1WALLET_MAIN
  // create menu options
  char *options_arr[MAX_LEN_OF_MENU_OPTIONS];
#endif

  switch (flow_level.level_one) {
#if X1WALLET_MAIN
    case LEVEL_TWO_OLD_WALLET: {
      if (counter.level > LEVEL_TWO) {
        level_three_old_wallet_tasks();
        return;
      }

      uint8_t wallet_index,
          status = get_index_by_name((const char *)wallet.wallet_name,
                                     &wallet_index);
      if (status != SUCCESS_) {
        LOG_ERROR("get %s fail %d", wallet.wallet_name, status);
        cy_exit_flow();
        return;
      }
      LOG_INFO("wallet %d %d %d %d %s",
               get_wallet_info(wallet_index),
               get_wallet_card_state(wallet_index),
               get_wallet_locked_status(wallet_index),
               get_wallet_state(wallet_index),
               get_wallet_name(wallet_index));

      if (get_wallet_state(wallet_index) == INVALID_WALLET) {
        mark_list_choice(LEVEL_THREE_DELETE_WALLET);
        mark_event_over();
        mark_error_screen(ui_text_creation_failed_delete_wallet);
        return;
      }

      if (is_wallet_locked(wallet_index)) {
        mark_list_choice(LEVEL_THREE_WALLET_LOCKED);
        mark_event_over();
        snprintf(flow_level.confirmation_screen_text,
                 sizeof(flow_level.confirmation_screen_text),
                 "%s",
                 ui_text_wallet_lock_continue_to_unlock);
        flow_level.show_desktop_start_screen = true;
        return;
      }

      if (is_wallet_partial(wallet_index)) {
        mark_list_choice(LEVEL_THREE_DELETE_WALLET);
        mark_event_over();
        mark_error_screen(ui_text_wallet_partial_continue_to_delete);
        return;
      }

      if (is_wallet_unverified(wallet_index)) {
        mark_list_choice(LEVEL_THREE_VERIFY_WALLET);
        mark_event_over();
        mark_error_screen(ui_text_wallet_not_verified_continue_to_verify);
        return;
      }

      if (is_wallet_share_not_present(wallet_index)) {
        mark_list_choice(LEVEL_THREE_SYNC_WALLET);
        mark_event_over();
        mark_error_screen(
            ui_text_wallet_out_of_sync_continue_to_sync_with_x1cards);
        return;
      }

      // options after selecting a wallet
      uint8_t oldWalletIndex = 0;
      for (; oldWalletIndex < NUMBER_OF_OPTIONS_OLD_WALLET; oldWalletIndex++) {
        options_arr[oldWalletIndex] =
            (char *)ui_text_options_old_wallet[oldWalletIndex];
      }
      if (WALLET_IS_ARBITRARY_DATA(wallet.wallet_info))
        options_arr[0] = (char *)ui_text_view_data;

      char arr[NAME_SIZE];
      snprintf(arr, sizeof(arr), "%s", wallet.wallet_name);
      menu_init(
          (const char **)options_arr, NUMBER_OF_OPTIONS_OLD_WALLET, arr, true);
    } break;

    case LEVEL_TWO_NEW_WALLET:

      if (counter.level > LEVEL_TWO) {
        if (flow_level.level_two == LEVEL_THREE_GENERATE_WALLET) {
          tasks_add_new_wallet();
        } else {
          tasks_restore_wallet();
        }
        return;
      }

      for (uint8_t newWalletIndex = 0;
           newWalletIndex < NUMBER_OF_OPTIONS_NEW_WALLET;
           newWalletIndex++) {
        options_arr[newWalletIndex] =
            (char *)ui_text_options_new_wallet[newWalletIndex + 1];
      }

      menu_init((const char **)options_arr,
                NUMBER_OF_OPTIONS_NEW_WALLET,
                ui_text_options_new_wallet[0],
                true);
      break;
#endif

    case LEVEL_TWO_ADVANCED_SETTINGS:

      if (counter.level > LEVEL_TWO) {
        level_three_advanced_settings_tasks();
        return;
      }
#if X1WALLET_MAIN
      uint8_t advancedSettingsIndex = 0, optionsIndex = 1;
      for (; advancedSettingsIndex < NUMBER_OF_OPTIONS_ADVANCED_OPTIONS;
           advancedSettingsIndex++) {
        if (optionsIndex == LEVEL_THREE_TOGGLE_LOGGING)
          options_arr[advancedSettingsIndex] =
              (char *)(is_logging_enabled()
                           ? ui_text_options_logging_export[0]
                           : ui_text_options_logging_export[1]);
        else if (optionsIndex == LEVEL_THREE_TOGGLE_PASSPHRASE)
          options_arr[advancedSettingsIndex] =
              (char *)(is_passphrase_enabled() ? ui_text_options_passphrase[0]
                                               : ui_text_options_passphrase[1]);
        else
          options_arr[advancedSettingsIndex] =
              (char *)ui_text_options_advanced_settings[optionsIndex];
        optionsIndex++;
      }

      menu_init((const char **)options_arr,
                NUMBER_OF_OPTIONS_ADVANCED_OPTIONS,
                ui_text_options_advanced_settings[0],
                true);
#endif
      break;

    default:
      message_scr_init(ui_text_something_went_wrong);
      break;
  }
}
