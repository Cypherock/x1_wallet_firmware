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
#include "ui_message.h"
#include "wallet_utilities.h"

extern Flash_Wallet wallet_for_flash;
extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;

void generate_wallet_controller() {
  switch (flow_level.level_three) {
    case GENERATE_WALLET_NAME_INPUT: {
      flow_level.level_three = GENERATE_WALLET_NAME_INPUT_CONFIRM;
    } break;

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
        flow_level.level_three = GENERATE_WALLET_PIN_INSTRUCTIONS_1;
      } else if (get_index_by_name(flow_level.screen_input.input_text, &temp) ==
                 INVALID_ARGUMENT) {
        mark_error_screen(ui_text_wallet_name_size_limit);
        flow_level.level_three = GENERATE_WALLET_NAME_INPUT;
      } else {
        LOG_ERROR("0xxx# wallet %d %d %d %s",
                  get_wallet_info(temp),
                  get_wallet_card_state(temp),
                  get_wallet_locked_status(temp),
                  get_wallet_name(temp));
        mark_error_screen(ui_text_wallet_name_exists);
        flow_level.level_three = GENERATE_WALLET_NAME_INPUT;
      }
    } break;

    case GENERATE_WALLET_PIN_INSTRUCTIONS_1: {
      flow_level.level_three = GENERATE_WALLET_PIN_INSTRUCTIONS_2;

    } break;

    case GENERATE_WALLET_PIN_INSTRUCTIONS_2: {
      flow_level.level_three = GENERATE_WALLET_SKIP_PIN;
    } break;

    case GENERATE_WALLET_SKIP_PIN: {
      flow_level.level_three = GENERATE_WALLET_PIN_INPUT;

      WALLET_SET_PIN(wallet_for_flash.wallet_info);
      WALLET_SET_PIN(wallet.wallet_info);
    } break;

    case GENERATE_WALLET_PIN_INPUT: {
      sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                 strnlen(flow_level.screen_input.input_text,
                         sizeof(flow_level.screen_input.input_text)),
                 wallet_credential_data.password_single_hash);
      sha256_Raw(wallet_credential_data.password_single_hash,
                 SHA256_DIGEST_LENGTH,
                 wallet.password_double_hash);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = GENERATE_WALLET_PIN_CONFIRM;
    } break;

    case GENERATE_WALLET_PIN_CONFIRM: {
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
        mark_error_screen(ui_text_pin_incorrect_re_enter);
        flow_level.level_three = GENERATE_WALLET_PIN_INPUT;
      }
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      memzero(temp, sizeof(temp));
    } break;

    case GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_1: {
      flow_level.level_three = GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_2;
    } break;

    case GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_2: {
      flow_level.level_three = GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_3;
    } break;

    case GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_3: {
      flow_level.level_three = GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_4;
    } break;

    case GENERATE_WALLET_PASSPHRASE_INSTRUCTIONS_4: {
      flow_level.level_three = GENERATE_WALLET_SKIP_PASSPHRASE;
    } break;

    case GENERATE_WALLET_SKIP_PASSPHRASE: {
      WALLET_SET_PASSPHRASE(wallet_for_flash.wallet_info);
      WALLET_SET_PASSPHRASE(wallet.wallet_info);
      flow_level.level_three = GENERATE_WALLET_PROCESSING;
    } break;

    case GENERATE_WALLET_PROCESSING: {
      flow_level.level_three = GENERATE_WALLET_SEED_GENERATE;
    } break;

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
    } break;

    case GENERATE_WALLET_SEED_GENERATED: {
      flow_level.level_three = GENERATE_WALLET_SAVE_WALLET_SHARE_TO_DEVICE;
    } break;

    case GENERATE_WALLET_SAVE_WALLET_SHARE_TO_DEVICE: {
      uint32_t index;
      wallet_for_flash.state = DEFAULT_VALUE_IN_FLASH;
      add_wallet_share_to_sec_flash(
          &wallet_for_flash, &index, wallet_shamir_data.mnemonic_shares[4]);
      flow_level.level_three = GENERATE_WALLET_TAP_CARD_FLOW;
    } break;

    case GENERATE_WALLET_TAP_CARD_FLOW: {
      card_flow_create_wallet();
    } break;

    case GENERATE_WALLET_VERIFY_SHARES:
      flow_level.level_three = verify_card_share_data() == 1
                                   ? GENERATE_WALLET_SUCCESS_MESSAGE
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

    case GENERATE_WALLET_SUCCESS_MESSAGE:
      reset_flow_level();
      break;

    case GENERATE_WALLET_FAILED_MESSAGE:
      flow_level.level_one = LEVEL_TWO_OLD_WALLET;
      flow_level.level_two = LEVEL_THREE_DELETE_WALLET;
      flow_level.level_three = 1;
      flow_level.level_four = 1;
      break;

    default:
      break;
  }
}
