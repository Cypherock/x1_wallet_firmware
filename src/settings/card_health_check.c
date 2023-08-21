/**
 * @file    card_health_check.c
 * @author  Cypherock X1 Team
 * @brief   Logic to check card health and view wallet list in X1 card
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
#include "card_operations.h"
#include "constant_texts.h"
#include "core_error.h"
#include "flash_api.h"
#include "settings_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
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
void card_health_check(void) {
  wallet_list_t wallets_in_card = {0};

  card_fetch_wallet_list_config_t configuration = {0};
  configuration.operation.acceptable_cards = ACCEPTABLE_CARDS_ALL;
  configuration.operation.skip_card_removal = true;
  configuration.operation.expected_family_id = get_family_id();
  configuration.frontend.heading = NULL;
  configuration.frontend.msg = ui_text_card_health_check_start;

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
    return;
  }

  // If the tapped card is not paired, it is a terminal case in the flow
  if (true == response.card_info.pairing_error) {
    return;
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

  uint8_t screens = 3;
  char display_msg[100] = "";
  const char *msg[3];

  if (0 == card_fault_status) {
    screens = 2;
    msg[0] = ui_text_card_seems_healthy;
    if (0 < wallets_in_card.count) {
      msg[1] = ui_text_click_to_view_wallets;
    } else {
      msg[1] = ui_text_no_wallets_fetched;
    }
  } else {
    snprintf(display_msg,
             sizeof(display_msg),
             "%s: C%04lX",
             ui_text_card_health_check_error[0],
             card_fault_status);
    msg[0] = (const char *)display_msg;
    msg[1] = ui_text_card_health_check_error[1];
    if (0 < wallets_in_card.count) {
      msg[2] = ui_text_click_to_view_wallets;
    } else {
      msg[2] = ui_text_no_wallets_fetched;
    }
  }

  typedef enum {
    CARD_HC_SHOW_WALLETS,
    CARD_HC_EXIT_FLOW,
  } card_health_check_states_e;

  card_health_check_states_e state_on_confirm = CARD_HC_EXIT_FLOW;
  if (0 < wallets_in_card.count) {
    state_on_confirm = CARD_HC_SHOW_WALLETS;
  }

  multi_instruction_init(msg, screens, DELAY_TIME, true);

  if (CARD_HC_SHOW_WALLETS == get_state_on_confirm_scr(state_on_confirm,
                                                       CARD_HC_EXIT_FLOW,
                                                       CARD_HC_EXIT_FLOW)) {
    memzero(display_msg, sizeof(display_msg));
    snprintf(display_msg,
             sizeof(display_msg),
             UI_TEXT_CARD_HEALTH_CHECK_ERROR,
             decode_card_number(response.card_info.tapped_card));

    char wallet_list[MAX_WALLETS_ALLOWED][NAME_SIZE] = {"", "", "", ""};
    for (uint8_t i = 0; i < wallets_in_card.count; i++) {
      snprintf(
          wallet_list[i], NAME_SIZE, (char *)wallets_in_card.wallet[i].name);
    }
    list_init(wallet_list, wallets_in_card.count, display_msg, false);

    // Do not care about the return value from confirmation screen
    (void)get_state_on_confirm_scr(0, 0, 0);
  }

  return;
}