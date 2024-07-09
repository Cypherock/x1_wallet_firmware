/**
 * @file    card_fetch_wallet_list.c
 * @author  Cypherock X1 Team
 * @brief   Source file supporting fetching wallet list from the X1 card
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
#include "card_fetch_wallet_list.h"

#include "card_internal.h"
#include "card_operation_typedefs.h"
#include "card_utils.h"
#include "flash_api.h"
#include "nfc.h"
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
card_error_type_e card_fetch_wallet_list(
    const card_fetch_wallet_list_config_t *config,
    card_fetch_wallet_list_response_t *response) {
  if (NULL == config || NULL == response || NULL == response->wallet_list) {
    return CARD_OPERATION_DEFAULT_INVALID;
  }

  // Render the instruction screen
  instruction_scr_init(config->frontend.msg, config->frontend.heading);

  card_operation_data_t card_data = {0};
  card_data.nfc_data.retries = 5;
  card_data.nfc_data.init_session_keys = true;

  while (1) {
    card_data.nfc_data.acceptable_cards = config->operation.acceptable_cards;
    memcpy(card_data.nfc_data.family_id,
           config->operation.expected_family_id,
           FAMILY_ID_SIZE);

    card_initialize_applet(&card_data);

    if (CARD_OPERATION_SUCCESS == card_data.error_type) {
      card_data.nfc_data.status = nfc_list_all_wallet(response->wallet_list);

      if (card_data.nfc_data.status == SW_NO_ERROR ||
          card_data.nfc_data.status == SW_RECORD_NOT_FOUND) {
        buzzer_start(BUZZER_DURATION);
        if (!config->operation.skip_card_removal) {
          wait_for_card_removal();
        }
        break;
      } else {
        card_handle_errors(&card_data);
      }
    }

    if (CARD_OPERATION_CARD_REMOVED == card_data.error_type ||
        CARD_OPERATION_RETAP_BY_USER_REQUIRED == card_data.error_type) {
      const char *error_msg = card_data.error_message;

      /**
       * In case the same card as before is tapped, the user should be told to
       * tap a different card instead of the default message "Wrong card
       * sequence"
       */
      if (SW_CONDITIONS_NOT_SATISFIED == card_data.nfc_data.status) {
        error_msg = ui_text_tap_another_card;
      }

      if (CARD_OPERATION_SUCCESS == indicate_card_error(error_msg)) {
        // Re-render the instruction screen
        instruction_scr_init(config->frontend.msg, config->frontend.heading);
        continue;
      }
    }

    // If control reached here, it is an unrecoverable error, so break
    break;
  }

  if (response->card_info.tapped_family_id) {
    memcpy(card_data.nfc_data.family_id,
           config->operation.expected_family_id,
           FAMILY_ID_SIZE);
  }
  response->card_info.pairing_error = card_data.nfc_data.pairing_error;
  response->card_info.tapped_card = card_data.nfc_data.tapped_card;
  response->card_info.recovery_mode = card_data.nfc_data.recovery_mode;
  response->card_info.status = card_data.nfc_data.status;

  nfc_deselect_card();
  return card_data.error_type;
}

bool card_get_wallet_name(uint8_t *wallet_id, const char *wallet_name) {
  wallet_list_t wallets_in_card = {0};

  card_fetch_wallet_list_config_t configuration = {0};
  configuration.operation.acceptable_cards = ACCEPTABLE_CARDS_ALL;
  configuration.operation.skip_card_removal = true;
  configuration.operation.expected_family_id = get_family_id();
  configuration.frontend.heading = NULL;
  configuration.frontend.msg = ui_text_card_get_wallet_info;

  card_fetch_wallet_list_response_t response = {0};
  response.wallet_list = &wallets_in_card;
  response.card_info.tapped_card = 0;
  response.card_info.recovery_mode = 0;
  response.card_info.status = 0;
  response.card_info.tapped_family_id = NULL;

  // P0 abort is the only condition we want to exit the flow
  // Card abort error will be explicitly shown here as error codes
  card_error_type_e status = card_fetch_wallet_list(&configuration, &response);
  if (CARD_OPERATION_P0_OCCURED == status) {
    return false;
  }

  // If the tapped card is not paired, it is a terminal case in the flow
  if (true == response.card_info.pairing_error) {
    return false;
  }

  // At this stage, either there is no core error message set, or it is set but
  // we want to overwrite the error message using user facing messages in this
  // flow
  clear_core_error_screen();

  uint32_t card_fault_status = 0;
  if (1 == response.card_info.recovery_mode) {
    card_fault_status = NFC_NULL_PTR_ERROR;
  } else if (CARD_OPERATION_SUCCESS != status) {
    card_fault_status = response.card_info.status;
  }

  char display_msg[100] = "";
  for (uint8_t i = 0; i < wallets_in_card.count; i++) {
    if (memcmp(wallet_id, wallets_in_card.wallet->id, WALLET_ID_SIZE) == 0) {
      memcpy(
          wallet_name, (const char *)wallets_in_card.wallet[i].name, NAME_SIZE);
      snprintf(display_msg,
               sizeof(display_msg),
               UI_TEXT_CARD_FETCH_WALLET,
               wallet_name);
      break;
    }
  }

  if (wallet_name == ""){
    return false;
  }

  if (!core_scroll_page(NULL, display_msg, NULL)) {
    return false;
  }

  // Do not care about the return value from confirmation screen
  (void)get_state_on_confirm_scr(0, 0, 0);
  // }

  return true;
}

