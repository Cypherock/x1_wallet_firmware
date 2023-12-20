/**
 * @file    card_flow_delete_wallet.c
 * @author  Cypherock X1 Team
 * @brief   Source file containing card flow to delete wallet on the X1 cards
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
#include "card_operations.h"
#include "constant_texts.h"
#include "core_error.h"
#include "nfc.h"
#include "ui_instruction.h"

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
 * @brief The function "check_card_state_and_delete_wallet" is used to delete a
 * wallet from the flash memory.
 *
 * @param wallet_name Name of the wallet on flash to be deleted
 *
 * @return void, which means it does not return any value.
 */
static void check_card_state_and_delete_wallet(const char *wallet_name);

/**
 * @brief The function checks if a wallet has already been deleted from a card
 * and deletes it if the card number is 4.
 *
 * @param delete_config A pointer to a structure of type
 * card_delete_share_cfg_t, which contains the following members:
 *
 * @return a boolean value.
 */
static bool check_wallet_already_deleted_on_card(
    card_delete_share_cfg_t *delete_config);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void check_card_state_and_delete_wallet(const char *wallet_name) {
  uint8_t flash_wallet_index = 0xFF;
  ASSERT(SUCCESS == get_index_by_name(wallet_name, &flash_wallet_index));

  if (0 == get_wallet_card_state(flash_wallet_index)) {
    ASSERT(SUCCESS_ == delete_wallet_share_from_sec_flash(flash_wallet_index));
    ASSERT(SUCCESS_ == delete_wallet_from_flash(flash_wallet_index));
  }
  return;
}

static bool check_wallet_already_deleted_on_card(
    card_delete_share_cfg_t *delete_config) {
  uint8_t flash_wallet_index = 0xFF;
  ASSERT(SUCCESS ==
         get_index_by_name((const char *)delete_config->wallet->wallet_name,
                           &flash_wallet_index));

  if (card_already_deleted_flash(flash_wallet_index,
                                 delete_config->card_number)) {
    return true;
  }
  return false;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
card_error_type_e card_flow_delete_wallet(Wallet *selected_wallet) {
  card_delete_share_cfg_t cfg = {.wallet = selected_wallet, .card_number = 0};
  card_error_type_e error_code = 0;

  card_fetch_share_config_t configuration = {0};
  card_fetch_share_response_t response = {0};
  char heading[MAX_HEADING_LEN] = "";
  configuration.xcor = 0;
  configuration.operation.expected_family_id = get_family_id();
  configuration.frontend.msg = ui_text_place_card_below;
  configuration.frontend.heading = heading;
  configuration.frontend.unexpected_card_error = ui_text_wrong_card_sequence;
  configuration.operation.skip_card_removal = true;
  configuration.operation.buzzer_on_success = false;
  response.card_info.tapped_family_id = NULL;

  for (int i = 1; i <= 4; i++) {
    snprintf(heading, MAX_HEADING_LEN, UI_TEXT_TAP_CARD, i);
    configuration.operation.acceptable_cards = encode_card_number(i);

    error_code = CARD_OPERATION_DEFAULT_INVALID;
    cfg.card_number = i;

    // If wallet already deleted from ith card, skip card tapping
    if (true == check_wallet_already_deleted_on_card(&cfg)) {
      error_code = CARD_OPERATION_SUCCESS;
      continue;
    }

    error_code = card_fetch_share(&configuration, &response);

    if (CARD_OPERATION_SUCCESS != error_code) {
      if (CARD_OPERATION_ABORT_OPERATION == error_code &&
          SW_RECORD_NOT_FOUND == response.card_info.status) {
        // In case wallet is not found on card, consider it as a success case as
        // wallet is already deleted or not created on the card.
        clear_core_error_screen();
      } else {
        break;
      }
    }

    // Operation to delete wallet from card and update card state on flash
    error_code = card_delete_share(&cfg);

    if (CARD_OPERATION_SUCCESS != error_code) {
      break;
    }
  }

  // If wallet is deleted on all cards, delete from flash as well
  check_card_state_and_delete_wallet((const char *)cfg.wallet->wallet_name);

  return error_code;
}