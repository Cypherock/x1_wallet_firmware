/**
 * @file    check_pairing.c
 * @author  Cypherock X1 Team
 * @brief   Fetches applet information and detects pairing status.
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

#include "check_pairing.h"

#include "card_internal.h"
#include "events.h"
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

/**
 * TODO: add description when this api gets concrete definition
 *
 * @param connection_data
 */
static void init_tap_card_data(NFC_connection_data *connection_data);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static void init_tap_card_data(NFC_connection_data *connection_data) {
  memset(connection_data, 0, sizeof(NFC_connection_data));
  connection_data->retries = 5;
  connection_data->acceptable_cards = ACCEPTABLE_CARDS_ALL;
  memset(connection_data->family_id, 0xff, sizeof(connection_data->family_id));
  memset(connection_data->card_key_id, 0, sizeof(connection_data->card_key_id));
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

card_error_type_e card_check_pairing(check_pairing_result_t *result) {
  LOG_SWV("%s (%d): %p\n", __func__, __LINE__, result);
  card_error_type_e status = CARD_OPERATION_DEFAULT_INVALID;
  if (NULL == result) {
    return status;
  }

  memset(result, 0, sizeof(check_pairing_result_t));
  result->card_number = 0;
  card_operation_data_t operation_data = {{0}, NULL, 0};
  init_tap_card_data(&operation_data.nfc_data);
  status = card_initialize_applet(&operation_data);
  nfc_deselect_card();

  buzzer_start(100);
  if (CARD_OPERATION_SUCCESS == status) {
    result->card_number =
        decode_card_number(operation_data.nfc_data.tapped_card);
    memcpy(
        result->family_id, operation_data.nfc_data.family_id, FAMILY_ID_SIZE);
    if (-1 == get_paired_card_index(operation_data.nfc_data.card_key_id)) {
      // keystore index is -1; the keystore does not have key-id
      result->is_paired = false;
    } else {
      result->is_paired = true;
    }
  }

  result->error_code = operation_data.nfc_data.status;

  return status;
}
