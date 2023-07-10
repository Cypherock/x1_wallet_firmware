/**
 * @file    card_flow_pairing.c
 * @author  Cypherock X1 Team
 * @brief   Source file containing card flow to pair X1 cards with X1 vault
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
#include "card_flow_reconstruct_wallet.h"
#include "card_operations.h"
#include "constant_texts.h"
#include "events.h"
#include "ui_screens.h"

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
bool card_flow_pairing(uint8_t *cards_paired) {
  uint8_t cards_paired_in_flow = 0;
  uint8_t card_number;
  char display[40];
  evt_status_t event = {0};

  for (card_number = 1; card_number <= MAX_KEYSTORE_ENTRY; card_number++) {
    // Check if the card #x is already paired
    if (1 == get_keystore_used_status(card_number - 1)) {
      continue;
    }

    snprintf(display, sizeof(display), UI_TEXT_PAIRING_TAP_CARD, card_number);
    skip_instruction_scr_init(display);

    // Enable task to select NFC card
    nfc_en_select_card_task();
    event =
        get_events(EVENT_CONFIG_NFC | EVENT_CONFIG_UI, MAX_INACTIVITY_TIMEOUT);

    if (true == event.p0_event.flag) {
      return false;
    } else if (true == event.ui_event.event_occured &&
               UI_EVENT_REJECT == event.ui_event.event_type) {
      continue;
    }

    // If control reached here, an NFC event occurred
    nfc_deselect_card();

    card_error_type_e card_error = CARD_OPERATION_ABORT_OPERATION;
    card_error =
        card_pair_operation(card_number, display, ui_text_place_card_below);
    if (CARD_OPERATION_ABORT_OPERATION == card_error) {
      return false;
    }

    cards_paired_in_flow += 1;
  }

  if (NULL != cards_paired) {
    *cards_paired = cards_paired_in_flow;
  }

  return true;
}