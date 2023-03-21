/**
 * @file    tasks_view_seeds.c
 * @author  Cypherock X1 Team
 * @brief   View seed task.
 *          This file contains the implementation of the view seed task.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
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
#include <string.h>
#include "bip39.h"
#include "constant_texts.h"
#include "controller_main.h"
#include "shamir_wrapper.h"
#include "tasks.h"
#include "tasks_tap_cards.h"
#include "ui_address.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_list.h"
#include "ui_message.h"
#include "ui_multi_instruction.h"
#include "wallet.h"

extern Wallet_shamir_data wallet_shamir_data;

extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;

void view_seed_task() {
  switch (flow_level.level_three) {
    case VIEW_SEED_DUMMY_TASK:
      multi_instruction_init(ui_text_view_seed_messages, 3, DELAY_LONG_STRING,
                             true);
      break;
    case VIEW_SEED_ENTER_PIN:
      if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
        flow_level.level_three = VIEW_SEED_DUMMY_TASK;
        break;
      }
      input_text_init(ALPHA_NUMERIC, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      break;

    case VIEW_SEED_TAP_CARDS_FLOW:
      tap_threshold_cards_for_reconstruction();
      break;

    case VIEW_SEED_SUCCESS:
      message_scr_init(ui_text_recovery_successfull);
      break;

    case VIEW_SEED_READ_DEVICE_SHARE:
      mark_event_over();
      break;

    case VIEW_SEED_DISPLAY: {
      uint8_t CONFIDENTIAL secret[MAX_ARBITRARY_DATA_SIZE];

      if (WALLET_IS_PIN_SET(wallet.wallet_info))
        decrypt_shares();
      if (WALLET_IS_ARBITRARY_DATA(wallet.wallet_info)) {
        recover_secret_from_shares(
            wallet.arbitrary_data_size,  // visualise this as horizontal length
            MINIMUM_NO_OF_SHARES,  //threshold. shares is a 2D array. visualise this as vertical height
            wallet_shamir_data.arbitrary_data_shares,
            wallet_shamir_data.share_x_coords, secret);
        secret[wallet.arbitrary_data_size] = '\0';

        address_scr_init(ui_text_confirm_data, (char *)secret, false);
      } else {
        recover_secret_from_shares(
            BLOCK_SIZE,  // visualise this as horizontal length
            MINIMUM_NO_OF_SHARES,  //threshold. shares is a 2D array. visualise this as vertical height
            wallet_shamir_data.mnemonic_shares,
            wallet_shamir_data.share_x_coords, secret);
        char words[MAX_NUMBER_OF_MNEMONIC_WORDS][MAX_MNEMONIC_WORD_LENGTH];
        mnemonic_clear();
        const char *mnemo = mnemonic_from_data(secret, BLOCK_SIZE);

        ASSERT(mnemo != NULL);
        __single_to_multi_line(mnemo,
                               strnlen(mnemo, MAX_NUMBER_OF_MNEMONIC_WORDS *
                                                  MAX_MNEMONIC_WORD_LENGTH),
                               words);
        mnemonic_clear();

        set_theme(LIGHT);
        list_init(words, wallet.number_of_mnemonics, ui_text_word_hash, true);
        reset_theme();
      }
      memzero(secret, sizeof(secret));
    } break;

    default:
      message_scr_init(ui_text_something_went_wrong);
      break;
  }
}
