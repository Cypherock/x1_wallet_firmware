/**
 * @file    controller_arbitrary_data.c
 * @author  Cypherock X1 Team
 * @brief   Arbitrary data next controller.
 *          Handles post event (only next events) operations for arbitrary data
 *flow.
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
#include "card_action_controllers.h"
#include "card_flow_create_wallet.h"
#include "constant_texts.h"
#include "controller_add_wallet.h"
#include "controller_main.h"
#include "controller_tap_cards.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "tasks.h"
#include "ui_message.h"
#include "wallet_utilities.h"

extern Wallet_credential_data wallet_credential_data;
extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;

extern Flash_Wallet wallet_for_flash;
extern char arbitrary_data[4096 / 8 + 1];

#ifdef USE_ARBITRARY_DATA
static void restore_wallet_enter_mnemonics_flow_controller() {
  if (flow_level.level_four <= wallet.number_of_mnemonics) {
    snprintf(
        wallet_credential_data.mnemonics[flow_level.level_four - 1],
        sizeof(wallet_credential_data.mnemonics[flow_level.level_four - 1]),
        "%s",
        mnemonic_get_word(flow_level.screen_input.list_choice));
    flow_level.level_four++;
  } else {
    flow_level.level_three++;
  }
}
#endif

void arbitrary_data_controller() {
  switch (flow_level.level_three) {
    case ARBITRARY_DATA_NAME_INPUT: {
      flow_level.level_three = ARBITRARY_DATA_NAME_CONFIRM;
    } break;
    case ARBITRARY_DATA_NAME_CONFIRM: {
      uint8_t dummy;
      if (get_index_by_name(flow_level.screen_input.input_text, &dummy) ==
          SUCCESS_) {
        // wallet already exists
        mark_error_screen(ui_text_wallet_name_exists);
      } else {
        flow_level.level_three = RESTORE_WALLET_PIN_INSTRUCTIONS_1;
        snprintf((char *)wallet_for_flash.wallet_name,
                 sizeof(wallet_for_flash.wallet_name),
                 "%s",
                 flow_level.screen_input.input_text);
        snprintf((char *)wallet.wallet_name,
                 sizeof(wallet.wallet_name),
                 "%s",
                 flow_level.screen_input.input_text);
      }
    } break;
    case RESTORE_WALLET_PIN_INSTRUCTIONS_1: {
      flow_level.level_three = RESTORE_WALLET_PIN_INSTRUCTIONS_2;
    } break;

    case RESTORE_WALLET_PIN_INSTRUCTIONS_2: {
      flow_level.level_three = ARBITRARY_DATA_SKIP_PIN;
    } break;

    case ARBITRARY_DATA_SKIP_PIN: {
      flow_level.level_three = ARBITRARY_DATA_PIN_INPUT;
      WALLET_SET_PIN(wallet_for_flash.wallet_info);
      WALLET_SET_PIN(wallet.wallet_info);
    } break;

    case ARBITRARY_DATA_PIN_INPUT: {
      sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                 strnlen(flow_level.screen_input.input_text,
                         sizeof(flow_level.screen_input.input_text)),
                 wallet.password_double_hash);
      sha256_Raw(wallet.password_double_hash,
                 SHA256_DIGEST_LENGTH,
                 wallet.password_double_hash);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = ARBITRARY_DATA_PIN_CONFIRM;
    } break;

    case ARBITRARY_DATA_PIN_CONFIRM: {
      uint8_t CONFIDENTIAL temp[SHA256_DIGEST_LENGTH] = {0};

      ASSERT(temp != NULL);
      sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                 strnlen(flow_level.screen_input.input_text,
                         sizeof(flow_level.screen_input.input_text)),
                 temp);
      sha256_Raw(temp, SHA256_DIGEST_LENGTH, temp);
      if (memcmp(wallet.password_double_hash, temp, SHA256_DIGEST_LENGTH) ==
          0) {
        flow_level.level_three = ARBITRARY_DATA_ENTER_DATA_INSTRUCTION;
      } else {
        mark_error_screen(ui_text_pin_incorrect_re_enter);
        flow_level.level_three = ARBITRARY_DATA_PIN_INPUT;
      }
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      memzero(temp, sizeof(temp));
    } break;

    case ARBITRARY_DATA_ENTER_DATA_INSTRUCTION: {
      flow_level.level_three = ARBITRARY_DATA_ENTER_DATA;

    } break;

    case ARBITRARY_DATA_ENTER_DATA: {
      flow_level.level_three = ARBITRARY_DATA_CONFIRM_DATA;
    } break;

    case ARBITRARY_DATA_CONFIRM_DATA: {
      flow_level.level_three = ARBITRARY_DATA_CREATING_WAIT_SCREEN;
    } break;

    case ARBITRARY_DATA_CREATING_WAIT_SCREEN: {
      flow_level.level_three = ARBITRARY_DATA_CREATE;
    } break;

    case ARBITRARY_DATA_CREATE: {
      uint8_t wallet_index, temp_wallet_id[WALLET_ID_SIZE];

      sha256_Raw((const uint8_t *)arbitrary_data,
                 strnlen(arbitrary_data, sizeof(arbitrary_data)),
                 temp_wallet_id);
      sha256_Raw(temp_wallet_id, SHA256_DIGEST_LENGTH, temp_wallet_id);

      if ((get_first_matching_index_by_id(temp_wallet_id, &wallet_index) ==
           DOESNT_EXIST)) {
        memcpy(wallet_for_flash.wallet_id, temp_wallet_id, WALLET_ID_SIZE);
        memcpy(wallet.wallet_id, wallet_for_flash.wallet_id, WALLET_ID_SIZE);
        wallet.arbitrary_data_size =
            strnlen(arbitrary_data, sizeof(arbitrary_data));
        WALLET_SET_ARBITRARY_DATA(wallet.wallet_info);
        WALLET_SET_ARBITRARY_DATA(wallet_for_flash.wallet_info);

        convert_to_shares(wallet.arbitrary_data_size,
                          (uint8_t *)arbitrary_data,
                          wallet.total_number_of_shares,
                          wallet.minimum_number_of_shares,
                          wallet_shamir_data.arbitrary_data_shares);
        memzero(arbitrary_data, sizeof(arbitrary_data));
        flow_level.level_three = ARBITRARY_DATA_TAP_CARDS;
        flow_level.level_four = 1;
        flow_level.level_five = 1;
      } else {
        mark_error_screen(ui_text_wallet_with_same_mnemo_exists);
        flow_level.level_three = ARBITRARY_DATA_ENTER_DATA_INSTRUCTION;
        flow_level.level_four = 1;
        flow_level.level_five = 1;
      }
    } break;

    case ARBITRARY_DATA_TAP_CARDS:
      card_flow_create_wallet();
      break;

    case ARBITRARY_DATA_VERIFY_SHARES:
      flow_level.level_three = verify_card_share_data() == 1
                                   ? ARBITRARY_DATA_SUCCESS_MESSAGE
                                   : ARBITRARY_DATA_FAILED_MESSAGE;
      memzero(wallet.password_double_hash, sizeof(wallet.password_double_hash));
      memzero(wallet.wallet_share_with_mac_and_nonce,
              sizeof(wallet.wallet_share_with_mac_and_nonce));
      memzero(wallet.arbitrary_data_share, sizeof(wallet.arbitrary_data_share));
      memzero(wallet.checksum, sizeof(wallet.checksum));
      memzero(wallet.key, sizeof(wallet.key));
      memzero(wallet.beneficiary_key, sizeof(wallet.beneficiary_key));
      memzero(wallet.iv_for_beneficiary_key,
              sizeof(wallet.iv_for_beneficiary_key));
      break;

    case ARBITRARY_DATA_SUCCESS_MESSAGE:
      reset_flow_level();
      break;

    case ARBITRARY_DATA_FAILED_MESSAGE:
      flow_level.level_one = LEVEL_TWO_OLD_WALLET;
      flow_level.level_three = 1;
      flow_level.level_four = 1;
      break;

    default:
      message_scr_init(ui_text_something_went_wrong);
      reset_flow_level();
      break;
  }
  return;
}
