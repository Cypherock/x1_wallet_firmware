/**
 * @file    restore_seed_phrase_flow.c
 * @author  Cypherock X1 Team
 * @brief   Restore wallet next controller.
 *          Handles post event (only next events) operations for restore wallet
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
#include "card_flow_create_wallet.h"
#include "constant_texts.h"
#include "core_error.h"
#include "flash_if.h"
#include "options.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "ui_core_confirm.h"
#include "ui_input_mnemonics.h"
#include "ui_screens.h"
#include "ui_state_machine.h"
#include "wallet_utilities.h"

extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;

extern Flash_Wallet wallet_for_flash;
extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;

typedef enum {
  NAME_INPUT = 1,
  NAME_INPUT_CONFIRM,
  PIN_INSTRUCTIONS,
  PIN_SELECT,
  PIN_INPUT,
  PIN_CONFIRM,
  PASSPHRASE_INSTRUCTIONS,
  PASSPHRASE_ENABLE,
  INPUT_NUMBER_OF_MNEMONICS,
  INPUT_SEED,
  VALIDATE_SEED,
  VERIFY_SEED,
  SAVE_WALLET_SHARE_TO_DEVICE,
  TAP_CARD_FLOW,
  VERIFY_SHARES,
  COMPLETED,
  COMPLETED_WITH_ERRORS,
  TIMED_OUT,
  EARLY_EXIT,
  EXIT,
} restore_wallet_state_e;

#ifndef SKIP_ENTER_MNEMONICS_DEBUG
/**
 * @brief Helper function to input mnemonics from user for restoring wallet
 *
 * @param number_of_mnemonics Number of mnemonics to be entered
 * @param wallet_credential_data Reference to the wallet_credential_data
 * structure where the mnemonics will be populated
 * @return restore_wallet_state_e The next state of the execution flow
 */
static restore_wallet_state_e enter_mnemonics(
    uint16_t number_of_mnemonics,
    Wallet_credential_data *wallet_credential_data) {
  char heading[20];
  uint16_t word = 0;

  while (word < number_of_mnemonics) {
    snprintf(heading, sizeof(heading), UI_TEXT_ENTER_WORD, word + 1);
    ui_mnem_init(heading);

    uint16_t choice = 0xFFFF;
    if (TIMED_OUT ==
        get_state_on_list_scr(INPUT_SEED, INPUT_SEED, TIMED_OUT, &choice)) {
      return TIMED_OUT;
    }

    // Either valid list choice was selected or the back button was pressed
    if (0xFFFF == choice) {
      // This case arises when the user presses back button on the mnem ui

      // If user presses back button on the first word, take them to the
      // previous step.
      if (0 == word) {
        return INPUT_NUMBER_OF_MNEMONICS;
      } else {
        word -= 1;
      }
    } else {
      // This case arises when the user makes a valid mnem selection
      // Clear the slot and then write the mnemonic
      memzero(wallet_credential_data->mnemonics[word],
              sizeof(wallet_credential_data->mnemonics[word]));
      snprintf(wallet_credential_data->mnemonics[word],
               sizeof(wallet_credential_data->mnemonics[word]),
               "%s",
               mnemonic_get_word(choice));
      word += 1;
    }
  }

  // Control reaches here only if all mnemonic words were entered
  return VALIDATE_SEED;
}
#endif

/**
 * @brief State handler for the restore wallet flow on the X1 vault
 *
 * @param current_state The current state of the flow which needs to be executed
 * @return restore_wallet_state_e The next state of the flow, based on
 * processing the current state handler
 */
restore_wallet_state_e restore_wallet_state_handler(
    restore_wallet_state_e current_state) {
  restore_wallet_state_e next_state = EXIT;

  switch (current_state) {
    case NAME_INPUT: {
      input_text_init(WALLET_NAME_CHARSET,
                      0,
                      ui_text_enter_wallet_name,
                      2,
                      DATA_TYPE_TEXT,
                      15);
      next_state = get_state_on_input_scr(NAME_INPUT, EARLY_EXIT, TIMED_OUT);

      if (NAME_INPUT != next_state) {
        break;
      }

      // Validate wallet name against previously created wallets
      uint8_t temp;
      int status = get_index_by_name(flow_level.screen_input.input_text, &temp);
      if (INVALID_ARGUMENT == status) {
        message_scr_init(ui_text_wallet_name_size_limit);
        next_state = get_state_on_confirm_scr(NAME_INPUT, TIMED_OUT, TIMED_OUT);
      } else if (SUCCESS_ == status) {
        LOG_ERROR("0xxx# wallet %d %d %d %s",
                  get_wallet_info(temp),
                  get_wallet_card_state(temp),
                  get_wallet_locked_status(temp),
                  get_wallet_name(temp));
        message_scr_init(ui_text_wallet_name_exists);
        next_state = get_state_on_confirm_scr(NAME_INPUT, TIMED_OUT, TIMED_OUT);
      } else {
        next_state = NAME_INPUT_CONFIRM;
      }

      break;
    }

    case NAME_INPUT_CONFIRM: {
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
      next_state =
          get_state_on_confirm_scr(PIN_INSTRUCTIONS, NAME_INPUT, TIMED_OUT);

      break;
    }

    case PIN_INSTRUCTIONS: {
      char display[75];
      if (strnlen(flow_level.screen_input.input_text,
                  sizeof(flow_level.screen_input.input_text)) <= 15)
        snprintf(
            display, sizeof(display), UI_TEXT_PIN_INS1, wallet.wallet_name);
      else
        snprintf(display, sizeof(display), UI_TEXT_PIN_INS1, "this wallet");

      delay_scr_init(display, DELAY_TIME);
      delay_scr_init(ui_wallet_pin_instruction_2, DELAY_LONG_STRING);
      next_state = PIN_SELECT;
      break;
    }

    case PIN_SELECT: {
      WALLET_UNSET_PIN(wallet_for_flash.wallet_info);
      WALLET_UNSET_PIN(wallet.wallet_info);

      confirm_scr_init(ui_text_do_you_want_to_set_pin);
      confirm_scr_focus_cancel();
      next_state = get_state_on_confirm_scr(
          PIN_INPUT, PASSPHRASE_INSTRUCTIONS, TIMED_OUT);
      break;
    }

    case PIN_INPUT: {
      WALLET_SET_PIN(wallet_for_flash.wallet_info);
      WALLET_SET_PIN(wallet.wallet_info);

      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      next_state = get_state_on_input_scr(PIN_INPUT, PIN_SELECT, TIMED_OUT);

      if (PIN_INPUT != next_state) {
        break;
      }

      sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                 strnlen(flow_level.screen_input.input_text,
                         sizeof(flow_level.screen_input.input_text)),
                 wallet_credential_data.password_single_hash);
      sha256_Raw(wallet_credential_data.password_single_hash,
                 SHA256_DIGEST_LENGTH,
                 wallet.password_double_hash);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      next_state = PIN_CONFIRM;
      break;
    }

    case PIN_CONFIRM: {
      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_confirm_pin, 4, DATA_TYPE_PIN, 8);
      next_state = get_state_on_input_scr(PIN_CONFIRM, PIN_SELECT, TIMED_OUT);

      if (PIN_CONFIRM != next_state) {
        break;
      }

      uint8_t CONFIDENTIAL temp[SHA256_DIGEST_LENGTH] = {0};
      sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                 strnlen(flow_level.screen_input.input_text,
                         sizeof(flow_level.screen_input.input_text)),
                 temp);
      sha256_Raw(temp, SHA256_DIGEST_LENGTH, temp);
      if (memcmp(wallet.password_double_hash, temp, SHA256_DIGEST_LENGTH) ==
          0) {
        next_state = PASSPHRASE_INSTRUCTIONS;
      } else {
        message_scr_init(ui_text_pin_incorrect_re_enter);
        next_state =
            get_state_on_confirm_scr(PIN_CONFIRM, TIMED_OUT, TIMED_OUT);
      }

      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      memzero(temp, sizeof(temp));
      break;
    }

    case PASSPHRASE_INSTRUCTIONS: {
      WALLET_UNSET_PASSPHRASE(wallet_for_flash.wallet_info);
      WALLET_UNSET_PASSPHRASE(wallet.wallet_info);

      if (!is_passphrase_enabled()) {
        next_state = INPUT_NUMBER_OF_MNEMONICS;
        break;
      }

      char display[65];
      snprintf(display,
               sizeof(display),
               UI_TEXT_PASSPHRASE_INS1,
               wallet.wallet_name);

      delay_scr_init(display, DELAY_TIME);
      delay_scr_init(ui_wallet_passphrase_instruction_2, DELAY_TIME);
      delay_scr_init(ui_wallet_passphrase_instruction_3, DELAY_TIME);
      delay_scr_init(ui_wallet_passphrase_instruction_4, DELAY_TIME);
      confirm_scr_init(ui_text_use_passphrase_question);
      confirm_scr_focus_cancel();
      next_state = get_state_on_confirm_scr(
          PASSPHRASE_ENABLE, INPUT_NUMBER_OF_MNEMONICS, TIMED_OUT);
      break;
    }

    case PASSPHRASE_ENABLE: {
      WALLET_SET_PASSPHRASE(wallet_for_flash.wallet_info);
      WALLET_SET_PASSPHRASE(wallet.wallet_info);
      next_state = INPUT_NUMBER_OF_MNEMONICS;
      break;
    }

    case INPUT_NUMBER_OF_MNEMONICS: {
      menu_init(ui_text_mnemonics_number_options,
                NUMBER_OF_OPTIONS_MNEMONIC_INPUT,
                ui_text_number_of_words,
                true);
      uint16_t choice = 0;
      next_state = get_state_on_list_scr(
          INPUT_NUMBER_OF_MNEMONICS, EARLY_EXIT, TIMED_OUT, &choice);

      if (INPUT_NUMBER_OF_MNEMONICS != next_state) {
        break;
      }

      next_state = INPUT_SEED;

      // overwrite wallet.number_of_mnemonics based on menu input choice
      if (1 == choice) {
        wallet.number_of_mnemonics = 12;
      } else if (2 == choice) {
        wallet.number_of_mnemonics = 18;
      } else if (3 == choice) {
        wallet.number_of_mnemonics = 24;
      } else {
        next_state = EARLY_EXIT;
      }

      break;
    }

    case INPUT_SEED: {
#ifndef SKIP_ENTER_MNEMONICS_DEBUG
      next_state =
          enter_mnemonics(wallet.number_of_mnemonics, &wallet_credential_data);
#else
      __single_to_multi_line("--ENTER-MNEMONIC-HERE-FOR-TESTING--",
                             36,
                             wallet_credential_data.mnemonics);
      next_state = VALIDATE_SEED;
#endif
      break;
    }

    case VALIDATE_SEED: {
      instruction_scr_init(ui_text_processing, NULL);
      char single_line_mnemonics[MAX_NUMBER_OF_MNEMONIC_WORDS *
                                 MAX_MNEMONIC_WORD_LENGTH];

#ifdef SKIP_ENTER_MNEMONICS_DEBUG
      snprintf(
          single_line_mnemonics, 36, "--ENTER-MNEMONIC-HERE-FOR-TESTING--");
      __single_to_multi_line(
          single_line_mnemonics, 160, wallet_credential_data.mnemonics);
#endif

      uint8_t wallet_index, temp_wallet_id[WALLET_ID_SIZE];

      __multi_to_single_line(wallet_credential_data.mnemonics,
                             wallet.number_of_mnemonics,
                             single_line_mnemonics);
      mnemonic_clear();

      // Check if mnemonics are valid by verifying checksum
      int result = mnemonic_check(single_line_mnemonics);
      mnemonic_clear();

      if (!result) {
        mark_core_error_screen(ui_text_incorrect_mnemonics, false);
        next_state = EARLY_EXIT;
        break;
      }

      // Check if the seed phrase matches an already existing wallet
      calculate_wallet_id(temp_wallet_id, single_line_mnemonics);
      if (get_first_matching_index_by_id(temp_wallet_id, &wallet_index) ==
          SUCCESS_) {
        mark_core_error_screen(ui_text_wallet_with_same_mnemo_exists, false);
        next_state = EARLY_EXIT;
        break;
      }

      // The seed phrase is valid, copy data into internal structs
      memcpy(wallet_for_flash.wallet_id, temp_wallet_id, WALLET_ID_SIZE);
      memcpy(wallet.wallet_id, wallet_for_flash.wallet_id, WALLET_ID_SIZE);
      uint8_t secret[BLOCK_SIZE + 1];
      memzero(secret, BLOCK_SIZE + 1);
      mnemonic_clear();
      mnemonic_to_entropy(single_line_mnemonics, secret);
      mnemonic_clear();
      convert_to_shares(BLOCK_SIZE,
                        secret,
                        wallet.total_number_of_shares,
                        wallet.minimum_number_of_shares,
                        wallet_shamir_data.mnemonic_shares);
      memzero(secret, sizeof(secret));

      derive_wallet_nonce(wallet_shamir_data.share_encryption_data);

      if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
        encrypt_shares();
      }
      derive_beneficiary_key(wallet.beneficiary_key,
                             wallet.iv_for_beneficiary_key,
                             single_line_mnemonics);
      derive_wallet_key(wallet.key, single_line_mnemonics);
      memzero(single_line_mnemonics, sizeof(single_line_mnemonics));

      next_state = VERIFY_SEED;
      break;
    }

    case VERIFY_SEED: {
      delay_scr_init(ui_text_verify_entered_words, DELAY_TIME);
      set_theme(LIGHT);
      list_init(wallet_credential_data.mnemonics,
                wallet.number_of_mnemonics,
                ui_text_verify_word_hash,
                true);
      next_state = get_state_on_confirm_scr(
          SAVE_WALLET_SHARE_TO_DEVICE, INPUT_SEED, TIMED_OUT);
      memzero(wallet_credential_data.mnemonics,
              sizeof(wallet_credential_data.mnemonics));
      reset_theme();
      break;
    }

    case SAVE_WALLET_SHARE_TO_DEVICE: {
      uint32_t index;
      wallet_for_flash.state = DEFAULT_VALUE_IN_FLASH;
      add_wallet_share_to_sec_flash(
          &wallet_for_flash,
          &index,
          wallet_shamir_data.mnemonic_shares[4],
          wallet_shamir_data.share_encryption_data[4]);
      next_state = TAP_CARD_FLOW;
      break;
    }

    case TAP_CARD_FLOW: {
      if (true == card_flow_create_wallet()) {
        next_state = VERIFY_SHARES;
      } else {
        next_state = COMPLETED_WITH_ERRORS;
      }
      break;
    }

    case VERIFY_SHARES: {
      instruction_scr_init(ui_text_processing, NULL);
      next_state =
          verify_card_share_data() == 1 ? COMPLETED : COMPLETED_WITH_ERRORS;
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

    case COMPLETED: {
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
      next_state = get_state_on_confirm_scr(EXIT, EXIT, EXIT);
      break;
    }

    case COMPLETED_WITH_ERRORS: {
      next_state = EXIT;
      break;
    }

    case TIMED_OUT:
    case EARLY_EXIT:
    case EXIT:
    default:
      break;
  }

  return next_state;
}

Flash_Wallet *restore_seed_phrase_flow() {
  restore_wallet_state_e current_state = NAME_INPUT;

  // Ensure that atleast 4 cards are paired
  if (get_keystore_used_count() < MAX_KEYSTORE_ENTRY) {
    mark_core_error_screen(ui_text_error_pair_all_cards, false);
    return NULL;
  }

  // Confirm that all 4 cards are needed
  if (!core_scroll_page(
          NULL, ui_text_need_all_x1cards_to_create_wallet, NULL)) {
    return NULL;
  }

  clear_wallet_data();

  while (1) {
    restore_wallet_state_e next_state =
        restore_wallet_state_handler(current_state);

    if (TIMED_OUT <= next_state) {
      break;
    }

    current_state = next_state;
  }

  Flash_Wallet *flash_wallet = NULL;
  get_flash_wallet_by_name((const char *)wallet.wallet_name, &flash_wallet);

  clear_wallet_data();
  return flash_wallet;
}