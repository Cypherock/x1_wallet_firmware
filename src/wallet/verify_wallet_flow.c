/**
 * @file    verify_wallet_flow.c
 * @author  Cypherock X1 Team
 * @brief   Source file for the wallet verification flow
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
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

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "verify_wallet_flow.h"

#include "card_flow_create_wallet.h"
#include "card_flow_verify_wallet.h"
#include "constant_texts.h"
#include "core_error.h"
#include "crypto_random.h"
#include "flash_api.h"
#include "flash_if.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "ui_state_machine.h"
#include "wallet_utilities.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/
// TODO: Remove usage of global variables
extern Flash_Wallet wallet_for_flash;
extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;

// TODO: Add these pointers in a common header file
extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef enum {
  PIN_INPUT,
  TAP_CARD_FLOW,
  VERIFY_SHARES,
  COMPLETED,
  COMPLETED_WITH_ERRORS,
  TIMED_OUT,
  EARLY_EXIT,
  EXIT,
} verify_wallet_state_e;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief State handler for the verify wallet flow on the X1 vault
 *
 * @param current_state The current state of the flow which needs to be executed
 * @return verify_wallet_state_e The next state of the flow, based on processing
 * the current state handler
 */
verify_wallet_state_e verify_wallet_state_handler(
    verify_wallet_state_e current_state);
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
verify_wallet_state_e verify_wallet_state_handler(
    verify_wallet_state_e current_state) {
  verify_wallet_state_e next_state = EXIT;

  switch (current_state) {
    case PIN_INPUT: {
      if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
        next_state = TAP_CARD_FLOW;
        break;
      }

      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      next_state = get_state_on_input_scr(PIN_INPUT, EARLY_EXIT, TIMED_OUT);

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
      next_state = TAP_CARD_FLOW;
      break;
    }

    case TAP_CARD_FLOW: {
      if (true == card_flow_verify_wallet()) {
        next_state = VERIFY_SHARES;
      } else {
        next_state = EXIT;
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
/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void verify_wallet_flow(const Flash_Wallet *flash_wallet) {
  verify_wallet_state_e current_state = PIN_INPUT;
  clear_wallet_data();

  // Populate global wallet object with wallet data
  memcpy(wallet.wallet_id, flash_wallet->wallet_id, WALLET_ID_SIZE);
  memcpy(wallet.wallet_name, flash_wallet->wallet_name, NAME_SIZE);
  wallet.wallet_info = flash_wallet->wallet_info;

  while (1) {
    verify_wallet_state_e next_state =
        verify_wallet_state_handler(current_state);

    if (TIMED_OUT <= next_state) {
      break;
    }

    current_state = next_state;
  }

  clear_wallet_data();
}