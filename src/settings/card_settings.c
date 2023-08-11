/**
 * @file    card_settings.c
 * @author  Cypherock X1 Team
 * @brief   Source file with helper functions related to X1 vault and X1 card
 *          mutual configuration
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
#include "card_flow_pairing.h"
#include "card_operations.h"
#include "constant_texts.h"
#include "flash_api.h"
#include "settings_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "ui_state_machine.h"
#include "utils.h"

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

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void pair_x1_cards(void) {
  if (MAX_KEYSTORE_ENTRY != get_keystore_used_count() &&
      !core_confirmation(ui_text_pair_card_confirm, NULL)) {
    return;
  }

  uint8_t new_cards_paired = 0;

  // In case any error occurred return early
  if (false == card_flow_pairing(&new_cards_paired)) {
    return;
  }

  if (MAX_KEYSTORE_ENTRY == get_keystore_used_count()) {
    delay_scr_init(ui_text_card_pairing_success, DELAY_TIME);
  } else {
    // Display newly paired cards to the user
    if (0 < new_cards_paired) {
      char msg[100] = {0};
      snprintf(msg, sizeof(msg), PAIR_CARD_MESSAGE, new_cards_paired);
      delay_scr_init(msg, DELAY_TIME);
    }
    delay_scr_init(ui_text_card_pairing_warning, DELAY_TIME);
  }

  return;
}

void view_card_version(void) {
  uint8_t card_version[CARD_VERSION_SIZE] = {0};

  if (!read_card_version(card_version, NULL, ui_text_tap_a_card)) {
    return;
  }

  char msg[100] = {0};
  char git_revision[2 * CARD_VERSION_GIT_REV_SIZE + 1] = {0};

  uint8_t major_version = (card_version[0] & 0xf0) >> 4;
  uint8_t minor_version = (card_version[0] & 0x0f);
  uint8_t patch = card_version[1];

  byte_array_to_hex_string(
      &card_version[2], 4, git_revision, sizeof(git_revision));

  snprintf(msg,
           sizeof(msg),
           UI_TEXT_CARD_VERSION,
           major_version,
           minor_version,
           patch,
           git_revision);

  message_scr_init(msg);

  // Do not care about the return value from confirmation screen
  (void)get_state_on_confirm_scr(0, 0, 0);

  return;
}
