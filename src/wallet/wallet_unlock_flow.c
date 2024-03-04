/**
 * @file    wallet_unlock_flow.c
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
#include "wallet_unlock_flow.h"

#include "card_operations.h"
#include "card_unlock_wallet.h"
#include "constant_texts.h"
#include "core_error.h"
#include "flash_api.h"
#include "pow.h"
#include "pow_utilities.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "ui_screens.h"
#include "ui_state_machine.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/
// TODO: Add these pointers in a common header file
extern char *ALPHA_NUMERIC;

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
  WALLET_UNLOCK_FETCH_CHALLENGE = 1,
  WALLET_UNLOCK_SOLVE_CHALLENGE,
  WALLET_UNLOCK_PIN_INPUT,
  WALLET_UNLOCK_VERIFY_CHALLENGE,
  WALLET_UNLOCK_REFETCH_CHALLENGE,
  WALLET_UNLOCK_COMPLETED,
  WALLET_UNLOCK_COMPLETED_WITH_ERRORS,
  WALLET_UNLOCK_TIMED_OUT,
  WALLET_UNLOCK_EARLY_EXIT,
  WALLET_UNLOCK_EXIT,
} wallet_unlock_state_e;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
/**
 * The function checks if all elements in the given nonce array are equal to a
 * default value.
 *
 * @param nonce The parameter "nonce" is a pointer to an array of type uint8_t,
 * which represents a nonce value.
 *
 * @return a boolean value.
 */
static bool check_default_nonce(const uint8_t *nonce);

/**
 * The function `wallet_unlock_handler` handles the unlocking process of a
 * wallet, including fetching and solving challenges, inputting a PIN, and
 * verifying the challenge.
 *
 * @param state The current state of the wallet unlock process. It is of type
 * `wallet_unlock_state_e`.
 * @param flash_wallet The `flash_wallet` parameter is a pointer to a structure
 * of type `Flash_Wallet`. This structure contains information about the wallet,
 * such as the wallet name, whether it is locked or not, and the challenge to
 * unlock the wallet.
 *
 * @return the next state of the wallet unlock process, which is of type
 * `wallet_unlock_state_e`.
 */
static wallet_unlock_state_e wallet_unlock_handler(
    wallet_unlock_state_e state,
    const Flash_Wallet *flash_wallet);

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_default_nonce(const uint8_t *nonce) {
  for (size_t i = 0; i < POW_NONCE_SIZE; i++) {
    if (nonce[i] != DEFAULT_VALUE_IN_FLASH) {
      return false;
    }
  }

  return true;
}

static wallet_unlock_state_e wallet_unlock_handler(
    wallet_unlock_state_e state,
    const Flash_Wallet *flash_wallet) {
  wallet_unlock_state_e next_state = WALLET_UNLOCK_EXIT;
  switch (state) {
    case WALLET_UNLOCK_FETCH_CHALLENGE:
      if (false == check_default_nonce(flash_wallet->challenge.nonce)) {
        next_state = WALLET_UNLOCK_SOLVE_CHALLENGE;
        break;
      }

      char heading[50] = "";
      card_operation_frontend_t frontend = {.heading = heading,
                                            .msg = ui_text_place_card_below};
      snprintf(heading,
               sizeof(heading),
               UI_TEXT_TAP_CARD,
               decode_card_number(flash_wallet->challenge.card_locked));
      instruction_scr_init(ui_text_place_card_below, heading);

    case WALLET_UNLOCK_REFETCH_CHALLENGE: {
      card_error_type_e error =
          card_fetch_challenge(flash_wallet->wallet_name, &frontend);

      switch (error) {
        case CARD_OPERATION_SUCCESS:
          if (false == flash_wallet->is_wallet_locked) {
            mark_core_error_screen(ui_text_wallet_already_unlocked, true);
            next_state = WALLET_UNLOCK_COMPLETED;
            break;
          } else {
            next_state = WALLET_UNLOCK_SOLVE_CHALLENGE;
          }
          break;

        default:
          next_state = WALLET_UNLOCK_EXIT;
          break;
      }

      if (WALLET_UNLOCK_REFETCH_CHALLENGE == state) {
        message_scr_init(ui_text_wallet_lock_continue_to_unlock);
        if (0 != get_state_on_confirm_scr(0, 1, 2)) {
          next_state = WALLET_UNLOCK_EXIT;
        }
      }

    } break;

    case WALLET_UNLOCK_SOLVE_CHALLENGE: {
      char slideshow[2][MAX_NUM_OF_CHARS_IN_A_SLIDE];
      convert_secs_to_time(flash_wallet->challenge.time_to_unlock_in_secs,
                           (char *)flash_wallet->wallet_name,
                           slideshow[0]);
      snprintf(slideshow[1],
               sizeof(slideshow[1]),
               UI_TEXT_UNLOCK_WARNING,
               wallet.wallet_name);
      char *temp[3] = {
          slideshow[0], (char *)ui_text_do_not_detach_device, slideshow[1]};
      ui_text_slideshow_init((const char **)temp, 3, DELAY_TIME, false);
      start_proof_of_work_task((const char *)flash_wallet->wallet_name);

      while (1) {
        if (true == proof_of_work_task()) {
          break;
        }
      }
      instruction_scr_init(ui_text_processing, NULL);
      next_state = WALLET_UNLOCK_PIN_INPUT;
    } break;

    case WALLET_UNLOCK_PIN_INPUT: {
      // TODO: Use common function for pin input in all flows
      if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
        next_state = WALLET_UNLOCK_VERIFY_CHALLENGE;
        break;
      }

      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      next_state = get_state_on_input_scr(WALLET_UNLOCK_PIN_INPUT,
                                          WALLET_UNLOCK_EARLY_EXIT,
                                          WALLET_UNLOCK_TIMED_OUT);

      if (WALLET_UNLOCK_PIN_INPUT == next_state) {
        sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                   strnlen(flow_level.screen_input.input_text,
                           sizeof(flow_level.screen_input.input_text)),
                   wallet_credential_data.password_single_hash);
        sha256_Raw(wallet_credential_data.password_single_hash,
                   SHA256_DIGEST_LENGTH,
                   wallet.password_double_hash);
        next_state = WALLET_UNLOCK_VERIFY_CHALLENGE;
      }

      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      break;
    }

    case WALLET_UNLOCK_VERIFY_CHALLENGE: {
      card_error_type_e card_status = card_unlock_wallet(&wallet);

      switch (card_status) {
        case CARD_OPERATION_SUCCESS: {
          char msg[50];
          snprintf(
              msg, sizeof(msg), UI_TEXT_WALLET_UNLOCKED, wallet.wallet_name);
          delay_scr_init(msg, DELAY_TIME);
          next_state = WALLET_UNLOCK_COMPLETED;
        } break;

        case CARD_OPERATION_LOCKED_WALLET:
          clear_core_error_screen();
          next_state = WALLET_UNLOCK_REFETCH_CHALLENGE;
          break;

        default:
          next_state = WALLET_UNLOCK_COMPLETED_WITH_ERRORS;
          break;
      }

      break;
    }

    // TODO: Manage states better to indicate error when failure occurs
    case WALLET_UNLOCK_COMPLETED:
    case WALLET_UNLOCK_COMPLETED_WITH_ERRORS:
    case WALLET_UNLOCK_TIMED_OUT:
    case WALLET_UNLOCK_EARLY_EXIT:
    case WALLET_UNLOCK_EXIT:
    default:
      break;
  }

  return next_state;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void wallet_unlock_flow(const Flash_Wallet *flash_wallet) {
  if ((NULL == flash_wallet) || (false == flash_wallet->is_wallet_locked)) {
    return 0;
  }

  wallet_unlock_state_e state = WALLET_UNLOCK_FETCH_CHALLENGE;
  clear_wallet_data();

  // Populate wallet data for card operations
  memcpy(wallet.wallet_id, flash_wallet->wallet_id, WALLET_ID_SIZE);
  memcpy(wallet.wallet_name, flash_wallet->wallet_name, NAME_SIZE);
  wallet.wallet_info = flash_wallet->wallet_info;

  while (1) {
    state = wallet_unlock_handler(state, flash_wallet);

    if (WALLET_UNLOCK_COMPLETED <= state) {
      break;
    }
  }

  clear_wallet_data();
}