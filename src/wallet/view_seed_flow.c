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

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief The function handles transitions of view seed flow for the passed
 * wallet id.
 *
 * @param wallet_id A pointer to a uint8_t array representing the wallet ID.
 *
 * @return true if view seed is successful, else false.
 */
static bool view_seed_handler(const uint8_t *wallet_id);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool view_seed_handler(const uint8_t *wallet_id) {
  CONFIDENTIAL char mnemonics[MAX_NUMBER_OF_MNEMONIC_WORDS]
                             [MAX_MNEMONIC_WORD_LENGTH] = {0};
  CONFIDENTIAL uint8_t no_of_mnemonics = 0;
  bool result = false;

  do {
    multi_instruction_init(
        ui_text_view_seed_messages, 3, DELAY_LONG_STRING, true);
    if (0 != get_state_on_confirm_scr(0, 1, 2)) {
      break;
    }

    no_of_mnemonics = reconstruct_mnemonics_flow(wallet_id, mnemonics);

    if (12 != no_of_mnemonics && 18 != no_of_mnemonics &&
        24 != no_of_mnemonics) {
      break;
    }

    message_scr_init(ui_text_seed_generated_successfully);
    if (0 != get_state_on_confirm_scr(0, 1, 2)) {
      break;
    }

    set_theme(LIGHT);
    list_init(mnemonics, no_of_mnemonics, ui_text_word_hash, true);
    if (0 == get_state_on_confirm_scr(0, 1, 2)) {
      result = true;
      break;
    }
  } while (0);

  reset_theme();
  memzero(mnemonics, sizeof(mnemonics));
  no_of_mnemonics = 0;
  return result;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void view_seed_flow(uint8_t *wallet_id) {
  ASSERT(NULL != wallet_id);

  if (false == view_seed_handler(wallet_id)) {
    mark_core_error_screen(ui_text_something_went_wrong);
  }

  return;
}