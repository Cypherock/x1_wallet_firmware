/**
 * @file    inheritance_decrypt_data.c
 * @author  Cypherock X1 Team
 * @brief   Inheritance message decryption login
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

#include <stdint.h>

#include "inheritance/core.pb.h"
#include "inheritance/decrypt_data_with_pin.pb.h"
#include "inheritance_api.h"
#include "inheritance_priv.h"
#include "reconstruct_wallet_flow.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "wallet_list.h"

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
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host inheritance app and return
 * false.
 *
 * @param query Reference to an instance of inheritance_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const inheritance_query_t *query,
                                pb_size_t which_request);

/**
 * @brief Validates the derivation path received in the request from host
 * @details The function validates the provided account derivation path in the
 * request. If invalid path is detected, the function will send an error to the
 * host and return false.
 *
 * @param request Reference to an instance of inheritance_decrypt_data_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(
    const inheritance_decrypt_data_with_pin_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * INHERITANCE_DECRYPT_DATA type of request. Additionally, the wallet-id is
 * validated for sanity and the derivation path for the account is also
 * validated. After the validations, user is prompted about the action for
 * confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the inheritance_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
STATIC bool inheritance_handle_initiate_query(const inheritance_query_t *query);

/**
 * @brief Aggregates user consent for the decryption info
 * @details The function displays the required messages for user to very
 *
 *
 * @return bool Indicating if the user confirmed the messages
 * @retval true If user confirmed the messages displayed
 * @retval false Immediate return if any of the messages are disapproved
 */
STATIC bool inheritance_get_user_pin_verification(inheritance_query_t *query);

/**
 * @brief Sends the decrypted data to the host
 * @details The function decrypts the data and sends it to the host
 *
 * @param query Reference to an instance of inheritance_query_t to store
 * transient request from the host
 * @return bool Indicating if the decrypted data is sent to the host
 * @retval true If the decrypted data was sent to host successfully
 * @retval false If the host responded with unknown/wrong query
 */
static bool send_decrypted_data(inheritance_query_t *query);

/**
 * @brief The function prepares and sends empty responses
 *
 * @param which_response Constant value for the response type to be sent
 */
static void send_response(pb_size_t which_response);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

STATIC inheritance_decryption_context_t *inheritance_decryption_context = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const inheritance_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->decrypt.which_request) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request_data(
    const inheritance_decrypt_data_with_pin_request_t *request) {
  bool status = true;

  // TODO: check the current request and session validity here

  return status;
}

STATIC bool inheritance_handle_initiate_query(
    const inheritance_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(
          query, INHERITANCE_DECRYPT_DATA_WITH_PIN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->decrypt) ||
      !get_wallet_name_by_id(query->decrypt.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             inheritance_send_error)) {
    return false;
  }

  snprintf(msg, sizeof(msg), "Test %s", wallet_name);    // TODO: update message
                                                         //
  // Take user consent to sign the transaction for the wallet
  if (!core_confirmation(msg, inheritance_send_error)) {
    return false;
  }

  set_app_flow_status(INHERITANCE_DECRYPT_DATA_STATUS_USER_CONFIRMED);

  // TODO: copy data to local context;

  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static void send_response(const pb_size_t which_response) {
  inheritance_result_t result =
      init_inheritance_result(INHERITANCE_RESULT_DECRYPT_TAG);
  result.decrypt.which_response = which_response;
  inheritance_send_result(&result);
}

static bool send_decrypted_data(inheritance_query_t *query) {
  inheritance_result_t result =
      init_inheritance_result(INHERITANCE_RESULT_DECRYPT_TAG);
  result.decrypt.which_response =
      INHERITANCE_DECRYPT_DATA_WITH_PIN_RESPONSE_MESSAGES_TAG;
  if (!inheritance_get_query(query, INHERITANCE_QUERY_DECRYPT_TAG) ||
      !check_which_request(
          query, INHERITANCE_DECRYPT_DATA_WITH_PIN_REQUEST_INITIATE_TAG)) {
    return false;
  }

  inheritance_decrypt_data_with_pin_messages_response_t dummy = {0};

  dummy.plain_data_count = 1;
  dummy.plain_data[0].message.size = 1;
  dummy.plain_data[0].message.bytes[0] = 97;

  memcpy(&result.decrypt.messages,
         &dummy,
         sizeof(inheritance_decrypt_data_with_pin_messages_response_t));

  set_app_flow_status(INHERITANCE_DECRYPT_DATA_STATUS_MESSAGE_DECRYPTED);
  inheritance_send_result(&result);
  return true;
}

STATIC bool inheritance_get_user_pin_verification(inheritance_query_t *query) {
  if (!inheritance_get_query(query, INHERITANCE_QUERY_DECRYPT_TAG) ||
      !check_which_request(query,
                           INHERITANCE_DECRYPT_DATA_WITH_PIN_REQUEST_ACK_TAG)) {
    return false;
  }
  // TODO: Show user the pin
  if (!core_scroll_page("Your PIN", "1234", inheritance_send_error)) {
    return false;
  }

  set_app_flow_status(INHERITANCE_DECRYPT_DATA_STATUS_PIN_VERIFIED);
  send_response(INHERITANCE_DECRYPT_DATA_WITH_PIN_RESPONSE_ACK_TAG);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void inheritance_decrypt_data(inheritance_query_t *query) {
  inheritance_decryption_context = (inheritance_decryption_context_t *)malloc(
      sizeof(inheritance_decryption_context_t));
  memzero(inheritance_decryption_context,
          sizeof(inheritance_decryption_context_t));

  // TODO: add actual decryption and decrypiton function
  if (inheritance_handle_initiate_query(query) && send_decrypted_data(query) &&
      inheritance_get_user_pin_verification(query)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  free(inheritance_decryption_context);
  inheritance_decryption_context = NULL;
}
