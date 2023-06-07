/**
 * @file    controller_new_wallet.c
 * @author  Cypherock X1 Team
 * @brief   New wallet next controller.
 *          Handles post event (only next events) operations for new wallet
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
#include <inttypes.h>
#include <stdlib.h>

#include "bip39.h"
#include "card_action_controllers.h"
#include "card_flow_create_wallet.h"
#include "constant_texts.h"
#include "controller_add_wallet.h"
#include "controller_main.h"
#include "controller_tap_cards.h"
#include "crypto_random.h"
#include "flash_api.h"
#include "flash_if.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "tasks.h"
#include "tasks_add_wallet.h"
#include "tasks_tap_cards.h"
#include "ui_address.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_list.h"
#include "ui_message.h"
#include "ui_multi_instruction.h"
#include "wallet_utilities.h"
extern Flash_Wallet wallet_for_flash;
extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;

extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;

extern Wallet_credential_data wallet_credential_data;
#include "events.h"

uint32_t next_state(uint32_t next, uint32_t prev) {
  evt_status_t status = get_events(EVENT_CONFIG_UI, MAX_INACTIVITY_TIMEOUT);

  if (true == status.p0_event.flag) {
    return GENERATE_WALLET_FAILED_MESSAGE;
  } else {
    if (UI_EVENT_REJECT == status.ui_event.event_type) {
      return prev;
    } else {
      return next;
    }
  }
  return GENERATE_WALLET_FAILED_MESSAGE;
}

uint32_t generate_wallet_controller_new(uint32_t current_state) {
  uint32_t state;

  switch (current_state) {
    case GENERATE_WALLET_NAME_INPUT: {
      input_text_init(
          ALPHABET, ui_text_enter_wallet_name, 2, DATA_TYPE_TEXT, 15);
      state = next_state(GENERATE_WALLET_NAME_INPUT_CONFIRM,
                         GENERATE_WALLET_FAILED_MESSAGE);
      break;
    }

    case GENERATE_WALLET_NAME_INPUT_CONFIRM: {
      uint8_t temp;
      if (get_index_by_name(flow_level.screen_input.input_text, &temp) ==
          DOESNT_EXIST) {
        memset((void *)&wallet_for_flash, 0, sizeof(wallet_for_flash));
        memset((void *)&wallet, 0, sizeof(wallet));
        set_wallet_init();
        snprintf((char *)wallet_for_flash.wallet_name,
                 sizeof(wallet_for_flash.wallet_name),
                 "%s",
                 flow_level.screen_input.input_text);
        snprintf((char *)wallet.wallet_name,
                 sizeof(wallet.wallet_name),
                 "%s",
                 flow_level.screen_input.input_text);
        char display[65];
        snprintf(
            display, sizeof(display), "%s", flow_level.screen_input.input_text);
        address_scr_init(ui_text_confirm_wallet_name, display, false);
        state = next_state(GENERATE_WALLET_PIN_INSTRUCTIONS_1,
                           GENERATE_WALLET_NAME_INPUT);
      } else if (get_index_by_name(flow_level.screen_input.input_text, &temp) ==
                 INVALID_ARGUMENT) {
        message_scr_init(ui_text_wallet_name_size_limit);
        state = next_state(GENERATE_WALLET_NAME_INPUT,
                           GENERATE_WALLET_FAILED_MESSAGE);
      } else {
        LOG_ERROR("0xxx# wallet %d %d %d %s",
                  get_wallet_info(temp),
                  get_wallet_card_state(temp),
                  get_wallet_locked_status(temp),
                  get_wallet_name(temp));
        message_scr_init(ui_text_wallet_name_exists);
        state = next_state(GENERATE_WALLET_NAME_INPUT,
                           GENERATE_WALLET_FAILED_MESSAGE);
      }

      break;
    }

    case GENERATE_WALLET_PIN_INSTRUCTIONS_1: {
      WALLET_UNSET_PIN(wallet_for_flash.wallet_info);
      WALLET_UNSET_PIN(wallet.wallet_info);

      char display[75];
      if (strnlen(flow_level.screen_input.input_text,
                  sizeof(flow_level.screen_input.input_text)) <= 15)
        snprintf(
            display, sizeof(display), UI_TEXT_PIN_INS1, wallet.wallet_name);
      else
        snprintf(display, sizeof(display), UI_TEXT_PIN_INS1, "this wallet");
      delay_scr_init(display, DELAY_TIME);
      delay_scr_init(ui_wallet_pin_instruction_2, DELAY_LONG_STRING);
      confirm_scr_init(ui_text_do_you_want_to_set_pin);
      confirm_scr_focus_cancel();
      state = next_state(GENERATE_WALLET_PIN_INPUT,
                         GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_1);
      break;
    }

    case GENERATE_WALLET_SKIP_PIN: {
      flow_level.level_three = GENERATE_WALLET_PIN_INPUT;

      WALLET_SET_PIN(wallet_for_flash.wallet_info);
      WALLET_SET_PIN(wallet.wallet_info);
      break;
    }

    case GENERATE_WALLET_PIN_INPUT: {
      input_text_init(ALPHA_NUMERIC, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      state = next_state(GENERATE_WALLET_PIN_CONFIRM,
                         GENERATE_WALLET_FAILED_MESSAGE);

      if (GENERATE_WALLET_PIN_CONFIRM == state) {
        sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                   strnlen(flow_level.screen_input.input_text,
                           sizeof(flow_level.screen_input.input_text)),
                   wallet_credential_data.password_single_hash);
        sha256_Raw(wallet_credential_data.password_single_hash,
                   SHA256_DIGEST_LENGTH,
                   wallet.password_double_hash);
        memzero(flow_level.screen_input.input_text,
                sizeof(flow_level.screen_input.input_text));
      }
      break;
    }

    case GENERATE_WALLET_PIN_CONFIRM: {
      input_text_init(ALPHA_NUMERIC, ui_text_confirm_pin, 4, DATA_TYPE_PIN, 8);
      state = next_state(GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_1,
                         GENERATE_WALLET_FAILED_MESSAGE);

      if (GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_1 == state) {
        uint8_t CONFIDENTIAL temp[SHA256_DIGEST_LENGTH] = {0};
        sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                   strnlen(flow_level.screen_input.input_text,
                           sizeof(flow_level.screen_input.input_text)),
                   temp);
        sha256_Raw(temp, SHA256_DIGEST_LENGTH, temp);
        if (memcmp(wallet.password_double_hash, temp, SHA256_DIGEST_LENGTH) ==
            0) {
          if (is_passphrase_enabled())
            flow_level.level_three = GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_1;
          else
            flow_level.level_three = GENERATE_WALLET_PROCESSING;
        } else {
          memzero(flow_level.screen_input.input_text,
                  sizeof(flow_level.screen_input.input_text));
          memzero(temp, sizeof(temp));
          message_scr_init(ui_text_pin_incorrect_re_enter);
          state = next_state(GENERATE_WALLET_PIN_INPUT,
                             GENERATE_WALLET_FAILED_MESSAGE);
        }
      }

      break;
    }

    case GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_1: {
      WALLET_UNSET_PASSPHRASE(wallet_for_flash.wallet_info);
      WALLET_UNSET_PASSPHRASE(wallet.wallet_info);

      if (!is_passphrase_enabled()) {
        state = GENERATE_WALLET_PROCESSING;
        break;
      }

      char display[65];
      if (strnlen(flow_level.screen_input.input_text,
                  sizeof(flow_level.screen_input.input_text)) <= 15)
        snprintf(display,
                 sizeof(display),
                 UI_TEXT_PASSPHRASE_INS1,
                 wallet.wallet_name);
      else
        snprintf(
            display, sizeof(display), UI_TEXT_PASSPHRASE_INS1, "this wallet");
      delay_scr_init(display, DELAY_TIME);
      delay_scr_init(ui_wallet_passphrase_instruction_2, DELAY_TIME);
      delay_scr_init(ui_wallet_passphrase_instruction_3, DELAY_TIME);
      delay_scr_init(ui_wallet_passphrase_instruction_4, DELAY_TIME);
      confirm_scr_init(ui_text_use_passphrase_question);
      confirm_scr_focus_cancel();
      state = next_state(GENERATE_WALLET_SKIP_PASSPHRASE,
                         GENERATE_WALLET_PROCESSING);
      break;
    }

    case GENERATE_WALLET_SKIP_PASSPHRASE: {
      WALLET_SET_PASSPHRASE(wallet_for_flash.wallet_info);
      WALLET_SET_PASSPHRASE(wallet.wallet_info);
      state = GENERATE_WALLET_PROCESSING;
      break;
    }

    case GENERATE_WALLET_PROCESSING: {
      instruction_scr_init(ui_text_processing, NULL);
      state = GENERATE_WALLET_SEED_GENERATE;
      break;
    }

    case GENERATE_WALLET_SEED_GENERATE: {
      // TODO: Use different variable for secret values instead of
      // wallet.wallet_share_with_mac_and_nonce
      random_generate(wallet.wallet_share_with_mac_and_nonce,
                      wallet.number_of_mnemonics * 4 / 3);
      mnemonic_clear();
      const char *mnemo =
          mnemonic_from_data(wallet.wallet_share_with_mac_and_nonce,
                             wallet.number_of_mnemonics * 4 / 3);

      ASSERT(mnemo != NULL);

      calculate_wallet_id(wallet_for_flash.wallet_id, mnemo);
      memcpy(wallet.wallet_id, wallet_for_flash.wallet_id, WALLET_ID_SIZE);
      convert_to_shares(32,
                        wallet.wallet_share_with_mac_and_nonce /*secret*/,
                        wallet.total_number_of_shares,
                        wallet.minimum_number_of_shares,
                        wallet_shamir_data.mnemonic_shares);
      if (WALLET_IS_PIN_SET(wallet.wallet_info))
        encrypt_shares();
      derive_beneficiary_key(
          wallet.beneficiary_key, wallet.iv_for_beneficiary_key, mnemo);
      derive_wallet_key(wallet.key, mnemo);
      mnemonic_clear();
      memzero(wallet.wallet_share_with_mac_and_nonce,
              sizeof(wallet.wallet_share_with_mac_and_nonce));
      flow_level.level_three = GENERATE_WALLET_SEED_GENERATED;
      instruction_scr_destructor();
      message_scr_init(ui_text_seed_generated_successfully);
      state = next_state(GENERATE_WALLET_SAVE_WALLET_SHARE_TO_DEVICE,
                         GENERATE_WALLET_FAILED_MESSAGE);
      break;
    }

    case GENERATE_WALLET_SAVE_WALLET_SHARE_TO_DEVICE: {
      uint32_t index;
      wallet_for_flash.state = DEFAULT_VALUE_IN_FLASH;
      add_wallet_share_to_sec_flash(
          &wallet_for_flash, &index, wallet_shamir_data.mnemonic_shares[4]);
      state = GENERATE_WALLET_TAP_CARD_FLOW;
      break;
    }

    case GENERATE_WALLET_TAP_CARD_FLOW: {
      card_flow_create_wallet();
    } break;

    case GENERATE_WALLET_VERIFY_SHARES: {
      instruction_scr_init(ui_text_processing, "");
      state = verify_card_share_data() == 1 ? GENERATE_WALLET_SUCCESS_MESSAGE
                                            : GENERATE_WALLET_FAILED_MESSAGE;
      memzero(wallet.password_double_hash, sizeof(wallet.password_double_hash));
      memzero(wallet_credential_data.password_single_hash,
              sizeof(wallet_credential_data.password_single_hash));
      memzero(wallet.wallet_share_with_mac_and_nonce,
              sizeof(wallet.wallet_share_with_mac_and_nonce));
      memzero(wallet.arbitrary_data_share, sizeof(wallet.arbitrary_data_share));
      memzero(wallet.checksum, sizeof(wallet.checksum));
      memzero(wallet.key, sizeof(wallet.key));
      memzero(wallet.beneficiary_key, sizeof(wallet.beneficiary_key));
      memzero(wallet.iv_for_beneficiary_key,
              sizeof(wallet.iv_for_beneficiary_key));
      break;
    }

    case GENERATE_WALLET_SUCCESS_MESSAGE: {
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
      break;
    }

    case GENERATE_WALLET_FAILED_MESSAGE: {
      message_scr_init(ui_text_creation_failed_delete_wallet);
      break;
    }

    default:
      break;
  }
  return state;
}

void create_wallet(bool new_wallet) {
  // TODO: use new_wallet to decide how mnemonics would be generated
  uint32_t state = GENERATE_WALLET_NAME_INPUT;

  while (1) {
    uint32_t next_state = generate_wallet_controller_new(state);

    if ((GENERATE_WALLET_SUCCESS_MESSAGE == next_state) ||
        (GENERATE_WALLET_FAILED_MESSAGE == next_state)) {
      return;
    }

    state = next_state;
  }

  return;
}