/**
 * @author  Cypherock X1 Team
 * @brief   Source file containing logic for pin verification using X1 card
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
#include "verify_pin_flow.h"

#include <stdint.h>
#include <string.h>

#include "bip39.h"
#include "card_flow_reconstruct_wallet.h"
#include "common_error.h"
#include "constant_texts.h"
#include "controller_main.h"
#include "core_error.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "status_api.h"
#include "ui_input_text.h"
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

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef enum {
  PIN_INPUT,
  TAP_CARD_FLOW,
  COMPLETED, /**<This state is reached if the pin was verified successfully*/
  COMPLETED_WITH_ERRORS, /**<This state is reached if any card abort error is
                            received during the card operation*/
  ABORTED_DUE_TO_P0, /**<This state is reached if a P0 event is observed during
                        flow */
  EARLY_EXIT, /**<This state is reached if the user presses the cancel button on
                 PIN entry screen */
  EXIT,       /**<This state must not be returned */
} verification_state_e;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief This function handles different states of the get verified pin flow
 *
 * @param state The current state of the flow which needs to be executed
 * @param secret_out Pointer to buffer which needs to be populated with the pin
 * @param reject_cb Callback to execute if there is any rejection during PIN
 * input occurs, or a card abort error occurs
 * @return verification_state_e The next state of the flow
 */
static verification_state_e get_verified_pin_handler(verification_state_e state,
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
 */
static bool get_verified_pin(const uint8_t *wallet_id,
                             verification_state_e init_state,
                             uint8_t pin[MAX_PIN_SIZE],
                             rejection_cb *reject_cb);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static verification_state_e get_verified_pin_handler(verification_state_e state,
                                                     uint8_t *secret_out,
                                                     rejection_cb *reject_cb) {
  verification_state_e next_state = EXIT;
  switch (state) {
    case PIN_INPUT: {
      if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
        next_state = COMPLETED_WITH_ERRORS;
        break;
      }

      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      next_state =
          get_state_on_input_scr(PIN_INPUT, EARLY_EXIT, ABORTED_DUE_TO_P0);

      if (PIN_INPUT == next_state) {
        memzero(secret_out, MAX_PIN_SIZE);
        size_t len = strnlen(flow_level.screen_input.input_text, MAX_PIN_SIZE);
        memcpy(secret_out, flow_level.screen_input.input_text, len);
        sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                   len,
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
      uint32_t card_error_code = 0;
      card_error_type_e card_status =
          card_flow_reconstruct_wallet(1, &card_error_code);

      if (CARD_OPERATION_SUCCESS == card_status) {
        set_core_flow_status(COMMON_SEED_GENERATION_STATUS_PIN_CARD);
        next_state = COMPLETED;
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

static bool get_verified_pin(const uint8_t *wallet_id,
                             verification_state_e init_state,
                             uint8_t pin[MAX_PIN_SIZE],
                             rejection_cb *reject_cb) {
  // Select wallet based on wallet_id
  if (!get_wallet_data_by_id(wallet_id, &wallet, reject_cb)) {
    return false;
  }

  // Run flow till it reaches a completion state
  verification_state_e current_state = init_state;
  while (1) {
    verification_state_e next_state =
        get_verified_pin_handler(current_state, pin, reject_cb);

    current_state = next_state;
    if (COMPLETED <= current_state) {
      break;
    }
  }

  if (COMPLETED == current_state) {
  } else if (reject_cb && EARLY_EXIT == current_state) {
    // Inform the host of any rejection
    reject_cb(ERROR_COMMON_ERROR_USER_REJECTION_TAG,
              ERROR_USER_REJECTION_CONFIRMATION);
    return false;
  }
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
bool verify_pin(const uint8_t *wallet_id,
                uint8_t pin_out[MAX_PIN_SIZE],
                rejection_cb *reject_cb) {
  if ((NULL == wallet_id) || (NULL == pin_out)) {
    return false;
  }

  clear_wallet_data();
  mnemonic_clear();

  if (!get_verified_pin(wallet_id, PIN_INPUT, pin_out, reject_cb)) {
    return false;
  }

  mnemonic_clear();
  clear_wallet_data();
  return true;
}
