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
#include "flash_api.h"
#include "manager_api.h"
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
 * TODO: Replace with api provided by manager app
 *
 * @param training
 */
static void send_message_to_host(manager_train_card_response_t *training);

/**
 * TODO: Replace with api provided by manager app
 *
 * @param error_code
 */
static void send_training_error(uint32_t error_code);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static void send_message_to_host(manager_train_card_response_t *training) {
  manager_result_t result =
      get_manager_result_template(MANAGER_RESULT_TRAIN_CARD_TAG);
  uint8_t result_buffer[1024] = {0};

  memcpy(&result.train_card, training, sizeof(manager_train_card_response_t));
  ASSERT(encode_and_send_manager_result(
      &result, result_buffer, sizeof(result_buffer)));
}

static void send_training_error(uint32_t error_code) {
  manager_train_card_response_t training =
      MANAGER_TRAIN_CARD_RESPONSE_INIT_ZERO;
  training.which_response = MANAGER_TRAIN_CARD_RESPONSE_COMMON_ERROR_TAG;
  training.common_error.which_error = ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG;
  training.common_error.unknown_error = error_code;
  send_message_to_host(&training);
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void manager_card_training(manager_query_t *query) {
  LOG_SWV("%s (%d)\n", __func__, __LINE__);
  if (NULL == query) {
    // TODO: verify card training query tag for completeness
    return;
  }
  // TODO: verify on-boarding state for safety
  core_status_set_device_waiting_on(CORE_DEVICE_WAITING_ON_BUSY_IP_CARD);

  instruction_scr_init(UI_TEXT_TAP_CARD_TO_TEST, NULL);

  check_pairing_result_t pair_result = {false, 0, {0}};
  manager_train_card_response_t result = MANAGER_TRAIN_CARD_RESPONSE_INIT_ZERO;
  result.which_response = MANAGER_TRAIN_CARD_RESPONSE_RESULT_TAG;
  card_error_type_e status = card_check_pairing(&pair_result);
  if (CARD_OPERATION_SUCCESS != status) {
    LOG_SWV("%s (%d)\n", __func__, __LINE__);
    send_training_error(1);
    return;
  }
  result.result.card_paired = pair_result.is_paired;
  if (DEFAULT_UINT32_IN_FLASH == U32_READ_BE_ARRAY(get_family_id())) {
    set_family_id_flash(pair_result.family_id);
  }

  // always pair the card
  // TODO: Update the pairing flow to support flexible storage of shared key

  // TODO: Fetch wallet list; send dummy data
  result.result.card_paired = false;
  result.result.wallet_list_count = 0;
  send_message_to_host(&result);

  char msg[64] = "";
  snprintf(msg, sizeof(msg), UI_TEXT_CARD_TAPPED, pair_result.card_number);
  delay_scr_init(msg, DELAY_TIME);
  // TODO: Show wallets if exist and wait for user acceptance on via app
}