/**
 * @file    card_training.c
 * @author  Cypherock X1 Team
 * @brief   Card training flow for user on-boarding.
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
#include "common_error.h"
#include "flash_api.h"
#include "manager_api.h"
#include "onboarding.h"
#include "status_api.h"
#include "ui_delay.h"
#include "ui_instruction.h"
#include "wallet.h"

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
 * @brief Sends the received message to host app.
 * @details The function internally calls manager_send_result to send the
 * message to host.
 *
 * @param resp Reference to a filled structure instance of
 * manager_train_card_response_t
 */
static void send_training_response(manager_train_card_response_t *resp);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static void send_training_response(manager_train_card_response_t *resp) {
  manager_result_t result = init_manager_result(MANAGER_RESULT_TRAIN_CARD_TAG);
  memcpy(&(result.train_card), resp, sizeof(manager_train_card_response_t));
  manager_send_result(&result);
  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void manager_card_training(manager_query_t *query) {
  if (!onboarding_step_allowed(MANAGER_ONBOARDING_STEP_CARD_CHECKUP)) {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_QUERY_NOT_ALLOWED);
    return;
  }

  if (MANAGER_TRAIN_CARD_REQUEST_INITIATE_TAG !=
      query->train_card.which_request) {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return;
  }

  // TODO: verify on-boarding state for safety
  core_status_set_device_waiting_on(CORE_DEVICE_WAITING_ON_BUSY_IP_CARD);

  instruction_scr_init(UI_TEXT_TAP_CARD_TO_TEST, NULL);

  check_pairing_result_t pair_result = {false, 0, {0}, 0};
  manager_train_card_response_t result = MANAGER_TRAIN_CARD_RESPONSE_INIT_ZERO;
  result.which_response = MANAGER_TRAIN_CARD_RESPONSE_RESULT_TAG;
  card_error_type_e status = card_check_pairing(&pair_result);
  if (CARD_OPERATION_SUCCESS != status) {
    LOG_SWV("%s (%d)\n", __func__, __LINE__);
    manager_send_error(ERROR_COMMON_ERROR_CARD_ERROR_TAG,
                       get_card_error_from_nfc_status(pair_result.error_code));
    return;
  }
  result.result.card_paired = pair_result.is_paired;
  if (U32_READ_BE_ARRAY(pair_result.family_id) !=
      U32_READ_BE_ARRAY(get_family_id())) {
    /* Always update the tapped family-id if it's different. We need to allow
     * all the next steps without family-id errors.
     */
    set_family_id_flash(pair_result.family_id);
  }

  // always pair the card
  // TODO: Update the pairing flow to support flexible storage of shared key

  // TODO: Fetch wallet list; send dummy data
  result.result.card_paired = false;
  result.result.wallet_list_count = 0;
  send_training_response(&result);

  char msg[64] = "";
  snprintf(msg, sizeof(msg), UI_TEXT_CARD_TAPPED, pair_result.card_number);
  delay_scr_init(msg, DELAY_TIME);
  // TODO: Show wallets if exist and wait for user acceptance on via app
  onboarding_set_step_done(MANAGER_ONBOARDING_STEP_CARD_CHECKUP);
}