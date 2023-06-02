/**
 * @file    card_utils.c
 * @author  Cypherock X1 Team
 * @brief   Card operations common utilities
 *
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
#include "card_utils.h"

#include "card_internal.h"
#include "constant_texts.h"
#include "controller_tap_cards.h"
#include "events.h"
#include "ui_instruction.h"
#include "ui_message.h"

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
NFC_connection_data init_nfc_connection_data(const uint8_t *family_id,
                                             uint8_t acceptable_cards) {
  NFC_connection_data nfc_data = {0};

  nfc_data.acceptable_cards = acceptable_cards;
  if (NULL != family_id) {
    memcpy(nfc_data.family_id, family_id, FAMILY_ID_SIZE);
  }

  return nfc_data;
}

card_error_type_e display_error_message(const char *error_message) {
  ASSERT(NULL != error_message);

  message_scr_init(error_message);

  evt_status_t status = get_events(EVENT_CONFIG_UI, MAX_INACTIVITY_TIMEOUT);

  if (true == status.p0_event.flag) {
    return CARD_OPERATION_P0_OCCURED;
  }

  if (true == status.ui_event.event_occured &&
      UI_EVENT_CONFIRM == status.ui_event.event_type) {
    return CARD_OPERATION_SUCCESS;
  }

  return CARD_OPERATION_DEFAULT_INVALID;
}

void get_card_serial(NFC_connection_data *nfc_data, uint8_t *serial) {
  ASSERT(NULL != nfc_data && NULL != serial);
  uint8_t card_number = 0xFF;

  memcpy(serial, nfc_data->family_id, FAMILY_ID_SIZE);
  card_number = decode_card_number(nfc_data->tapped_card);
  serial[CARD_ID_SIZE - 1] = card_number;
}

card_error_type_e wait_for_card_removal(void) {
  if (0 != nfc_en_wait_for_card_removal_task()) {
    // Card not selected or removed
    return CARD_OPERATION_SUCCESS;
  }

  instruction_scr_change_text(ui_text_remove_card_prompt, true);

  evt_status_t status = get_events(EVENT_CONFIG_NFC, MAX_INACTIVITY_TIMEOUT);
  if (true == status.p0_event.flag) {
    return CARD_OPERATION_P0_OCCURED;
  }

  if (true == status.nfc_event.event_occured &&
      NFC_EVENT_CARD_REMOVED == status.nfc_event.event_type) {
    return CARD_OPERATION_SUCCESS;
  }

  // Shouldn't reach here
  return CARD_OPERATION_DEFAULT_INVALID;
}