/**
 * @file    btc_txn.c
 * @author  Cypherock X1 Team
 * @brief   Bitcoin family transaction flow.
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

#include "btc.h"
#include "btc_api.h"
#include "btc_helpers.h"
#include "btc_priv.h"
#include "constant_texts.h"
#include "reconstruct_seed_flow.h"
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

typedef struct {
  pb_byte_t prev_txn_hash[32];
  uint32_t prev_output_index;
  uint64_t value;
  btc_sign_txn_input_script_pub_key_t script_pub_key;
  uint32_t sequence;
  uint32_t change_index;
  uint32_t address_index;
} sign_txn_input_t;

typedef struct {
  btc_sign_txn_initiate_request_t init_info;
  btc_sign_txn_metadata_t metadata;

  btc_sign_txn_output_t *outputs;
  sign_txn_input_t *inputs;
} txn_context;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host bitcoin app and return false.
 *
 * @param query Reference to an instance of btc_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const btc_query_t *query,
                                pb_size_t which_request);

/**
 * @brief Validates the derivation path received in the request from host
 * @details The function validates the provided account derivation path in the
 * request. If invalid path is detected, the function will send an error to the
 * host and return false.
 *
 * @param request Reference to an instance of btc_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(const btc_sign_txn_request_t *request);

/**
 * @brief The function prepares and sends empty responses
 *
 * @param which_response Constant value for the response type to be sent
 */
static void send_response(pb_size_t which_response);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the BTC_SIGN_TXN type
 * of request. Additionally, the wallet-id is validated for sanity and the
 * derivation path for the account is also validated. After the validations,
 * user is prompted about the action for confirmation. The function returns true
 * indicating all the validation and user confirmation was a success. The
 * function also duplicates the data from query into the btc_txn_context  for
 * further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const btc_query_t *query);

/**
 * @brief Handles fetching of the metadata/top-level transaction elements
 * @details The function waits on USB event then decoding and validation of the
 * received query. Post validation, based on the values in the query, the
 * function allocates memory for storing input & output UTXOs in btc_txn_context
 * . Also, the data received in the query is duplicated into btc_txn_context .
 *
 * @param query Reference to storage for decoding query from host
 *
 * @return bool Indicating if the function actions succeeded or failed
 */
static bool fetch_transaction_meta(btc_query_t *query);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

static txn_context *btc_txn_context = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const btc_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    btc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request_data(const btc_sign_txn_request_t *request) {
  bool status = true;

  if (!btc_derivation_path_guard(request->initiate.derivation_path,
                                 request->initiate.derivation_path_count)) {
    btc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }
  return status;
}

static void send_response(const pb_size_t which_response) {
  btc_result_t result = init_btc_result(BTC_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  btc_send_result(&result);
}

static bool handle_initiate_query(const btc_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  // TODO: Handle wallet search failures - eg: Wallet ID not found, Wallet
  // ID found but is invalid/locked wallet
  if (!check_which_request(query, BTC_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name)) {
    return false;
  }

  snprintf(msg, sizeof(msg), UI_TEXT_BTC_SEND_PROMPT, g_app->name, wallet_name);
  // Take user consent to sign transaction for the wallet
  if (!core_confirmation(msg, btc_send_error)) {
    return false;
  }

  core_status_set_flow_status(BTC_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&btc_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(btc_sign_txn_initiate_request_t));
  send_response(BTC_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  return true;
}

static bool fetch_transaction_meta(btc_query_t *query) {
  if (!btc_get_query(query, BTC_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, BTC_SIGN_TXN_REQUEST_META_TAG)) {
    return false;
  }
  // TODO: add checks for validating the metadata

  // we now know the number of input and output UTXOs
  // allocate memory for input and output UTXOs in btc_txn_context
  memcpy(&btc_txn_context->metadata,
         &query->sign_txn.meta,
         sizeof(btc_sign_txn_metadata_t));
  btc_txn_context->inputs = (sign_txn_input_t *)malloc(
      sizeof(sign_txn_input_t) * btc_txn_context->metadata.input_count);
  btc_txn_context->outputs = (btc_sign_txn_output_t *)malloc(
      sizeof(btc_sign_txn_output_t) * btc_txn_context->metadata.output_count);
  // TODO: check if malloc failed; report to host and exit
  send_response(BTC_SIGN_TXN_RESPONSE_META_ACCEPTED_TAG);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void btc_sign_transaction(btc_query_t *query) {
  btc_txn_context = (txn_context *)malloc(sizeof(txn_context));
  memzero(btc_txn_context, sizeof(txn_context));

  if (!handle_initiate_query(query) && !fetch_transaction_meta(query)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  if (NULL != btc_txn_context && NULL != btc_txn_context->inputs) {
    free(btc_txn_context->inputs);
  }
  if (NULL != btc_txn_context && NULL != btc_txn_context->outputs) {
    free(btc_txn_context->outputs);
  }
  if (NULL != btc_txn_context) {
    free(btc_txn_context);
    btc_txn_context = NULL;
  }
}