/**
 * @file    card_flow_create_wallet.c
 * @author  Cypherock X1 Team
 * @brief   Source file containing card flow to create wallet on the X1 cards
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
card_error_type_e card_flow_reconstruct_wallet(uint8_t threshold,
                                               uint32_t *error_status_OUT) {
  card_error_type_e result = CARD_OPERATION_DEFAULT_INVALID;

  // Validate threshold
  if (threshold > MAX_KEYSTORE_ENTRY) {
    return result;
  }

  card_fetch_share_config_t configuration = {0};
  configuration.xcor = 0;
  configuration.operation.acceptable_cards = ACCEPTABLE_CARDS_ALL;
  configuration.operation.expected_family_id = get_family_id();
  configuration.frontend.heading = ui_text_tap_1_2_cards;
  configuration.frontend.msg = ui_text_place_card_below;
  configuration.frontend.unexpected_card_error = ui_text_tap_another_card;
  configuration.operation.skip_card_removal = false;
  configuration.operation.buzzer_on_success = true;

  card_fetch_share_response_t response = {0};
  response.card_info.tapped_family_id = NULL;

  for (uint8_t xcoord = 0; xcoord < threshold; xcoord++) {
    // Don't accept the same card again
    configuration.operation.acceptable_cards ^= response.card_info.tapped_card;

    // Accept only paired cards
    configuration.operation.expected_family_id = get_family_id();

    // Change heading for second card onwards
    if (0 != xcoord) {
      configuration.frontend.heading = ui_text_tap_2_2_cards;
    }

    // Skip card removal in last card
    if (threshold == xcoord + 1) {
      configuration.operation.skip_card_removal = true;
    }

    configuration.xcor = xcoord;

    // Reset response
    response.card_info.status = 0;
    response.card_info.tapped_card = 0;
    response.card_info.recovery_mode = 0;

    result = CARD_OPERATION_DEFAULT_INVALID;
    result = card_fetch_share(&configuration, &response);

    if (CARD_OPERATION_SUCCESS != result) {
      break;
    }
  }

  if (NULL != error_status_OUT) {
    *error_status_OUT = response.card_info.status;
  }
  return result;
}
