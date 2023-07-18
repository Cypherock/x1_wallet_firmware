/**
 * @file    view_seed_flow.c
 * @author  Cypherock X1 Team
 * @brief   Flow for view seed operation on an existing wallet
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
#include "constant_texts.h"
#include "core_error.h"
#include "reconstruct_wallet_flow.h"
#include "ui_delay.h"
#include "ui_list.h"
#include "ui_message.h"
#include "ui_multi_instruction.h"
#include "ui_state_machine.h"
/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef enum view_seed_states {
  VIEW_SEED_INSTRUCTIONS = 1,
  VIEW_SEED_MNEMONIC_RECONSTRUCTION,
  VIEW_SEED_RECONSTRUCT_SUCCESS_MESSAGE,
  VIEW_SEED_MNEMONIC_DISPLAY,
  VIEW_SEED_COMPLETED,
  VIEW_SEED_COMPLETED_WITH_ERRORS,
  VIEW_SEED_TIMED_OUT,
  VIEW_SEED_EARLY_EXIT,
  VIEW_SEED_EXIT,
} view_seed_states_e;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief The function handles different states and transitions of view seed
 * flow based on the current state.
 *
 * @param current_state The current state of the view seed process. It is of
 * type `view_seed_states_e`, which is an enumeration representing different
 * states of the process.
 * @param wallet_id A pointer to a uint8_t array representing the wallet ID.
 *
 * @return the next state of the view_seed_states_e enum type.
 */
view_seed_states_e view_seed_handler(view_seed_states_e current_state,
                                     const uint8_t *wallet_id);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
CONFIDENTIAL char mnemonics[MAX_NUMBER_OF_MNEMONIC_WORDS]
                           [MAX_MNEMONIC_WORD_LENGTH] = {0};
CONFIDENTIAL uint8_t no_of_mnemonics = 0;
/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
view_seed_states_e view_seed_handler(view_seed_states_e current_state,
                                     const uint8_t *wallet_id) {
  view_seed_states_e next_state = current_state;

  switch (current_state) {
    case VIEW_SEED_INSTRUCTIONS:
      multi_instruction_init(
          ui_text_view_seed_messages, 3, DELAY_LONG_STRING, true);
      next_state = get_state_on_confirm_scr(
          VIEW_SEED_MNEMONIC_RECONSTRUCTION, EXIT_FAILURE, VIEW_SEED_TIMED_OUT);
      break;

    case VIEW_SEED_MNEMONIC_RECONSTRUCTION:
      if (NULL == wallet_id) {
        next_state = VIEW_SEED_COMPLETED_WITH_ERRORS;
        break;
      }

      memzero(mnemonics, sizeof(mnemonics));
      no_of_mnemonics = reconstruct_mnemonics_flow(wallet_id, mnemonics);

      next_state = VIEW_SEED_COMPLETED_WITH_ERRORS;
      if (12 == no_of_mnemonics || 18 == no_of_mnemonics ||
          24 == no_of_mnemonics) {
        next_state = VIEW_SEED_RECONSTRUCT_SUCCESS_MESSAGE;
      }
      break;

    case VIEW_SEED_RECONSTRUCT_SUCCESS_MESSAGE:
      message_scr_init(ui_text_seed_generated_successfully);
      next_state = get_state_on_confirm_scr(
          VIEW_SEED_MNEMONIC_DISPLAY, VIEW_SEED_EXIT, VIEW_SEED_TIMED_OUT);
      break;

    case VIEW_SEED_MNEMONIC_DISPLAY:
      set_theme(LIGHT);
      list_init(mnemonics, no_of_mnemonics, ui_text_word_hash, true);
      next_state = get_state_on_confirm_scr(
          VIEW_SEED_COMPLETED, VIEW_SEED_EXIT, VIEW_SEED_TIMED_OUT);
      reset_theme();

      memzero(mnemonics, sizeof(mnemonics));
      break;

    case VIEW_SEED_COMPLETED_WITH_ERRORS:
      mark_core_error_screen(ui_text_something_went_wrong);
      next_state = VIEW_SEED_EARLY_EXIT;
      break;

    case VIEW_SEED_TIMED_OUT:
    case VIEW_SEED_COMPLETED:
    case VIEW_SEED_EXIT:
    case VIEW_SEED_EARLY_EXIT:
    default:
      break;
  }

  return next_state;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void view_seed_flow(uint8_t wallet_index) {
  if (0 == wallet_index || MAX_WALLETS_ALLOWED < wallet_index) {
    return;
  }

  const Flash_Wallet *temp_wallet = get_wallet_by_index(wallet_index);
  ASSERT(NULL != temp_wallet);

  view_seed_states_e flow_state = VIEW_SEED_INSTRUCTIONS;

  while (1) {
    flow_state = view_seed_handler(flow_state, temp_wallet->wallet_id);
    if (VIEW_SEED_COMPLETED <= flow_state) {
      break;
    }
  }

  memzero(mnemonics, sizeof(mnemonics));
  no_of_mnemonics = 0x0;
}