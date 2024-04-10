/**
 * @file    reconstruct_seed.c
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
#include "reconstruct_wallet_flow.h"

#include "bip39.h"
#include "card_flow_reconstruct_wallet.h"
#include "common_error.h"
#include "constant_texts.h"
#include "core_error.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "status_api.h"
#include "ui_screens.h"
#include "ui_state_machine.h"
#include "wallet_list.h"
#include "wallet_utilities.h"

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
  COMPLETED,             /**<This state is reached if the entropy is generated
                            successfully*/
  COMPLETED_WITH_ERRORS, /**<This state is reached if any card abort error is
                            received during the card operation*/
  ABORTED_DUE_TO_P0, /**<This state is reached if a P0 event is observed during
                        flow */
  EARLY_EXIT, /**<This state is reached if the user presses the cancel button on
                 PIN or Passphrase entry screen */
  EXIT,       /**<This state must not be returned */
} reconstruct_state_e;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief This function handles different states of the reconstruct wallet flow
 *
 * @param state The current state of the flow which needs to be executed
 * @param secret_out Pointer to buffer which needs to be populated with the
 * wallet secret
 * @param reject_cb Callback to execute if there is any rejection during PIN or
 * passphrase input occurs, or a card abort error occurs
 * @return reconstruct_state_e The next state of the flow
 */
static reconstruct_state_e reconstruct_wallet_handler(reconstruct_state_e state,
                                                      uint8_t *secret_out,
                                                      rejection_cb *reject_cb);

/**
 * @brief This function takes wallet ID, an initial state, and a rejection
 * callback function as input, and reconstructs a wallet by running a series of
 * handlers until it reaches a completion state, returning the generated
 * mnemonics if successful.
 *
 * @param wallet_id A pointer to the wallet ID, which is an array of uint8_t
 * (unsigned 8-bit integers).
 * @param init_state The initial state of the wallet reconstruction process. It
 * determines where the reconstruction process should start from.
 * @param reject_cb Callback to execute if there is any rejection during PIN or
 * passphrase input occurs, or a card abort error occurs
 *
 * @return a pointer to a constant character mnemonics string (const char *).
 */
static const char *reconstruct_wallet(const uint8_t *wallet_id,
                                      reconstruct_state_e init_state,
                                      rejection_cb *reject_cb);

/**
 * @brief The function generates mnemonics from a secret and verifies a wallet
 * ID using those mnemonics.
 *
 * @param secret A pointer to an array of uint8_t, which represents the secret
 * data used to generate the mnemonics.
 * @param wallet_id A pointer to an array uint8_t with wallet id, wallet_id  is
 * compared against the wallet id generated from mnemonics, if same wallet id is
 * generated, then wallet is verified.
 *
 * @return a pointer to a constant character mnemonics string (const char *).
 */
static const char *generate_mnemonics_and_verify_wallet(
    const uint8_t *secret,
    const uint8_t *wallet_id);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static reconstruct_state_e reconstruct_wallet_handler(reconstruct_state_e state,
                                                      uint8_t *secret_out,
                                                      rejection_cb *reject_cb) {
  reconstruct_state_e next_state = EXIT;
  switch (state) {
    case PASSPHRASE_INPUT: {
      set_core_flow_status(COMMON_SEED_GENERATION_STATUS_INIT);
      if (!WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
        next_state = PIN_INPUT;
        break;
      }

      input_text_init(PASSPHRASE,
                      26,
                      ui_text_enter_passphrase,
                      0,
                      DATA_TYPE_PASSPHRASE,
                      64);
      next_state = get_state_on_input_scr(
          PASSPHRASE_CONFIRM, EARLY_EXIT, ABORTED_DUE_TO_P0);
      break;
    }

    case PASSPHRASE_CONFIRM: {
      char display[65];
      snprintf(
          display, sizeof(display), "%s", flow_level.screen_input.input_text);
      ui_scrollable_page(
          ui_text_confirm_passphrase, display, MENU_SCROLL_HORIZONTAL, false);
      memzero(display, sizeof(display));
      next_state = get_state_on_confirm_scr(
          PASSPHRASE_CONFIRM, PASSPHRASE_INPUT, ABORTED_DUE_TO_P0);

      if (PASSPHRASE_CONFIRM == next_state) {
        snprintf(wallet_credential_data.passphrase,
                 sizeof(wallet_credential_data.passphrase),
                 "%s",
                 flow_level.screen_input.input_text);
        set_core_flow_status(COMMON_SEED_GENERATION_STATUS_PASSPHRASE);
        next_state = PIN_INPUT;
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

      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      next_state =
          get_state_on_input_scr(PIN_INPUT, EARLY_EXIT, ABORTED_DUE_TO_P0);

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
      uint32_t card_error_code;
      card_error_type_e card_status =
          card_flow_reconstruct_wallet(1, &card_error_code);

      if (CARD_OPERATION_SUCCESS == card_status) {
        set_core_flow_status(COMMON_SEED_GENERATION_STATUS_PIN_CARD);
        next_state = RECONSTRUCT_SEED;
      } else if (CARD_OPERATION_INCORRECT_PIN_ENTERED == card_status) {
        next_state = PIN_INPUT;
      } else if (CARD_OPERATION_P0_OCCURED == card_status) {
        next_state = ABORTED_DUE_TO_P0;
      } else {
        /* In case of other status code returned by the card operation:
         * CARD_OPERATION_LOCKED_WALLET,
         * CARD_OPERATION_ABORT_OPERATION
         *
         * we need to inform the host on the type of card error due to which
         * operation was aborted, as these are non-recoverable.
         */
        if (reject_cb) {
          reject_cb(ERROR_COMMON_ERROR_CARD_ERROR_TAG,
                    get_card_error_from_nfc_status(card_error_code));
        }
        next_state = COMPLETED_WITH_ERRORS;
      }

      break;
    }

    case RECONSTRUCT_SEED: {
      instruction_scr_init(ui_text_processing, NULL);
      wallet_shamir_data.share_x_coords[1] = 5;
      get_flash_wallet_share_by_name((const char *)wallet.wallet_name,
                                     wallet_shamir_data.mnemonic_shares[1]);
      memcpy(wallet_shamir_data.share_encryption_data[1],
             wallet_shamir_data.share_encryption_data[0],
             PADDED_NONCE_SIZE + WALLET_MAC_SIZE);

      if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
        decrypt_shares();
      }

      recover_secret_from_shares(BLOCK_SIZE,
                                 MINIMUM_NO_OF_SHARES,
                                 wallet_shamir_data.mnemonic_shares,
                                 wallet_shamir_data.share_x_coords,
                                 secret_out);

      memzero(wallet_shamir_data.mnemonic_shares,
              sizeof(wallet_shamir_data.mnemonic_shares));
      next_state = COMPLETED;
      break;
    }

    // TODO: Manage states better to indicate error when failure occurs
    case COMPLETED:
    case COMPLETED_WITH_ERRORS:
    case ABORTED_DUE_TO_P0:
    case EARLY_EXIT:
    case EXIT:
    default:
      break;
  }

  return next_state;
}

static const char *generate_mnemonics_and_verify_wallet(
    const uint8_t *secret,
    const uint8_t *wallet_id) {
  const char *mnemonics =
      mnemonic_from_data(secret, wallet.number_of_mnemonics * 4 / 3);
  ASSERT(mnemonics != NULL);

  if (!verify_wallet_id(wallet_id, mnemonics)) {
    mark_core_error_screen(ui_text_wallet_verification_failed_in_reconstruction,
                           false);
    mnemonics = NULL;
  }
  return mnemonics;
}

static const char *reconstruct_wallet(const uint8_t *wallet_id,
                                      reconstruct_state_e init_state,
                                      rejection_cb *reject_cb) {
  uint8_t secret[BLOCK_SIZE] = {0};
  const char *mnemonics = NULL;

  // Select wallet based on wallet_id
  if (!get_wallet_data_by_id(wallet_id, &wallet, reject_cb)) {
    return NULL;
  }

  // Run flow till it reaches a completion state
  reconstruct_state_e current_state = init_state;
  while (1) {
    reconstruct_state_e next_state =
        reconstruct_wallet_handler(current_state, secret, reject_cb);

    current_state = next_state;
    if (COMPLETED <= current_state) {
      break;
    }
  }

  if (COMPLETED == current_state) {
    mnemonics = generate_mnemonics_and_verify_wallet(secret, wallet_id);
    if (NULL == mnemonics) {
      if (reject_cb) {
        reject_cb(ERROR_COMMON_ERROR_CARD_ERROR_TAG,
                  ERROR_CARD_ERROR_SW_RECORD_NOT_FOUND);
      }
      current_state = COMPLETED_WITH_ERRORS;
    }
  } else if (reject_cb && EARLY_EXIT == current_state) {
    // Inform the host of any rejection
    reject_cb(ERROR_COMMON_ERROR_USER_REJECTION_TAG,
              ERROR_USER_REJECTION_CONFIRMATION);
  }

  memzero(secret, sizeof(secret));
  return mnemonics;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
bool reconstruct_seed(const uint8_t *wallet_id,
                      uint8_t *seed_out,
                      rejection_cb *reject_cb) {
  if ((NULL == wallet_id) || (NULL == seed_out)) {
    return false;
  }

  uint8_t result = false;

  clear_wallet_data();
  mnemonic_clear();

  const char *mnemonics =
      reconstruct_wallet(wallet_id, PASSPHRASE_INPUT, reject_cb);

  if (NULL != mnemonics) {
    mnemonic_to_seed(
        mnemonics, wallet_credential_data.passphrase, seed_out, NULL);
    result = true;
  }

  mnemonic_clear();
  clear_wallet_data();
  return result;
}

uint8_t reconstruct_mnemonics(const uint8_t *wallet_id,
                              char mnemonic_list[MAX_NUMBER_OF_MNEMONIC_WORDS]
                                                [MAX_MNEMONIC_WORD_LENGTH]) {
  if ((NULL == wallet_id) || (NULL == mnemonic_list)) {
    return 0;
  }

  uint8_t result = 0;

  clear_wallet_data();
  mnemonic_clear();

  const char *mnemonics = reconstruct_wallet(wallet_id, PIN_INPUT, NULL);

  if (NULL != mnemonics) {
    uint16_t len = strnlen(
        mnemonics, MAX_NUMBER_OF_MNEMONIC_WORDS * MAX_MNEMONIC_WORD_LENGTH);
    __single_to_multi_line(mnemonics, len, mnemonic_list);
    result = wallet.number_of_mnemonics;
  }

  mnemonic_clear();
  clear_wallet_data();
  return result;
}