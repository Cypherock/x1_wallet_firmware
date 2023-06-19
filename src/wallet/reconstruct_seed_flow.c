/**
 * @file    reconstruct_seed_flow.c
 * @author  Cypherock X1 Team
 * @brief   Source file containing logic for seed reconstruction using X1 cards
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
#include "reconstruct_seed_flow.h"

#include "card_flow_reconstruct_wallet.h"
#include "constant_texts.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "ui_screens.h"
#include "ui_state_machine.h"
#include "wallet_list.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/
// TODO: Add these pointers in a common header file
extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;
extern char *PASSPHRASE;

// TODO: Remove usage of global variables
extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;
extern Wallet wallet;

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef enum {
  PASSPHRASE_INPUT = 1,
  PASSPHRASE_CONFIRM,
  PIN_INPUT,
  TAP_CARD_FLOW,
  RECONSTRUCT_SEED,
  COMPLETED,
  COMPLETED_WITH_ERRORS,
  TIMED_OUT,
  EARLY_EXIT,
  EXIT,
} reconstruct_state_e;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief This function handles different states of the reconstruct wallet seed
 * flow
 *
 * @param state The current state of the flow which needs to be executed
 * @param seed_out Pointer to buffer which needs to be populated with the wallet
 * seed
 * @return reconstruct_state_e The next state of the flow
 */
reconstruct_state_e reconstruct_seed_handler(reconstruct_state_e state,
                                             uint8_t *seed_out);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
reconstruct_state_e reconstruct_seed_handler(reconstruct_state_e state,
                                             uint8_t *seed_out) {
  reconstruct_state_e next_state = EXIT;
  switch (state) {
    case PASSPHRASE_INPUT: {
      if (!WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
        next_state = PIN_INPUT;
        break;
      }

      input_text_init(
          PASSPHRASE, ui_text_enter_passphrase, 0, DATA_TYPE_PASSPHRASE, 64);
      next_state =
          get_state_on_input_scr(PASSPHRASE_CONFIRM, EARLY_EXIT, TIMED_OUT);
      break;
    }

    case PASSPHRASE_CONFIRM: {
      char display[65];
      snprintf(
          display, sizeof(display), "%s", flow_level.screen_input.input_text);
      address_scr_init(ui_text_confirm_passphrase, display, false);
      memzero(display, sizeof(display));
      next_state = get_state_on_confirm_scr(
          PASSPHRASE_CONFIRM, PASSPHRASE_INPUT, TIMED_OUT);

      if (PASSPHRASE_CONFIRM == next_state) {
        snprintf(wallet_credential_data.passphrase,
                 sizeof(wallet_credential_data.passphrase),
                 "%s",
                 flow_level.screen_input.input_text);
      }

      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      break;
    }

    case PIN_INPUT: {
      if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
        next_state = TAP_CARD_FLOW;
        break;
      }

      input_text_init(ALPHA_NUMERIC, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      next_state = get_state_on_input_scr(PIN_INPUT, EARLY_EXIT, TIMED_OUT);

      if (PIN_INPUT == next_state) {
        sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                   strnlen(flow_level.screen_input.input_text,
                           sizeof(flow_level.screen_input.input_text)),
                   wallet_credential_data.password_single_hash);
        sha256_Raw(wallet_credential_data.password_single_hash,
                   SHA256_DIGEST_LENGTH,
                   wallet.password_double_hash);
        next_state = TAP_CARD_FLOW;
      }

      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      break;
    }

    case TAP_CARD_FLOW: {
      card_error_type_e card_status = card_flow_reconstruct_wallet(1);

      if (CARD_OPERATION_SUCCESS == card_status) {
        next_state = RECONSTRUCT_SEED;
      } else if (CARD_OPERATION_INCORRECT_PIN_ENTERED == card_status) {
        next_state = PIN_INPUT;
      } else {
        /* In case of other status code returned by the card operation:
         * CARD_OPERATION_PAIRING_REQUIRED,
         * CARD_OPERATION_LOCKED_WALLET,
         * CARD_OPERATION_P0_OCCURED,
         * CARD_OPERATION_ABORT_OPERATION
         * These error codes are non-recoverable from this flow. Error message
         * will be displayed to the user before the main menu
         */
        next_state = COMPLETED_WITH_ERRORS;
      }

      break;
    }

    case RECONSTRUCT_SEED: {
      instruction_scr_init("", NULL);
      instruction_scr_change_text(ui_text_processing, true);
      wallet_shamir_data.share_x_coords[1] = 5;
      get_flash_wallet_share_by_name((const char *)wallet.wallet_name,
                                     wallet_shamir_data.mnemonic_shares[1]);
      memcpy(wallet_shamir_data.share_encryption_data[1],
             wallet_shamir_data.share_encryption_data[0],
             NONCE_SIZE + WALLET_MAC_SIZE);

      uint8_t secret[BLOCK_SIZE];
      if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
        decrypt_shares();
      }

      recover_secret_from_shares(BLOCK_SIZE,
                                 MINIMUM_NO_OF_SHARES,
                                 wallet_shamir_data.mnemonic_shares,
                                 wallet_shamir_data.share_x_coords,
                                 secret);
      memzero(wallet_shamir_data.mnemonic_shares,
              sizeof(wallet_shamir_data.mnemonic_shares));
      mnemonic_clear();
      const char *mnemo =
          mnemonic_from_data(secret, wallet.number_of_mnemonics * 4 / 3);

      ASSERT(mnemo != NULL);
      mnemonic_to_seed(
          mnemo, wallet_credential_data.passphrase, seed_out, NULL);
      mnemonic_clear();
      next_state = COMPLETED;
      break;
    }

    case COMPLETED:
    case COMPLETED_WITH_ERRORS:
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
bool reconstruct_seed_flow(const uint8_t *wallet_id, uint8_t *seed_out) {
  if ((NULL == wallet_id) || (NULL == seed_out)) {
    return false;
  }

  // TODO: Consolidate in one function
  // Clear confidential data irrespective of the result of the flow
  memzero(&wallet, sizeof(wallet));
  memzero(&wallet_shamir_data, sizeof(wallet_shamir_data));
  memzero(&wallet_credential_data, sizeof(wallet_credential_data));

  // Select wallet based on wallet_id
  if (false == wallet_selector(wallet_id, &wallet)) {
    return false;
  }

  // Run flow till it reaches a completion state
  reconstruct_state_e current_state = PASSPHRASE_INPUT;
  while (1) {
    reconstruct_state_e next_state =
        reconstruct_seed_handler(current_state, seed_out);

    current_state = next_state;
    if (COMPLETED <= current_state) {
      break;
    }
  }

  // Clear confidential data irrespective of the result of the flow
  memzero(&wallet, sizeof(wallet));
  memzero(&wallet_shamir_data, sizeof(wallet_shamir_data));
  memzero(&wallet_credential_data, sizeof(wallet_credential_data));

  if (COMPLETED == current_state) {
    return true;
  }

  return false;
}
