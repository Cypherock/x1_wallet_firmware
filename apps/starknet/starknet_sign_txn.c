/**
 * @file    starknet_txn.c
 * @author  Cypherock X1 Team
 * @brief   Source file to handle transaction signing logic for Starknet
 *protocol
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

#include "reconstruct_wallet_flow.h"
#include "starknet_api.h"
#include "starknet_context.h"
#include "starknet_helpers.h"
#include "starknet_priv.h"
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
 * fails, then it will send an error to the host bitcoin app and return false.
 *
 * @param query Reference to an instance of starknet_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const starknet_query_t *query,
                                pb_size_t which_request);

/**
 * @brief The function prepares and sends empty responses
 *
 * @param which_response Constant value for the response type to be sent
 */
static void send_response(const pb_size_t which_response);

/**
 * @brief Validates the derivation path received in the request from host
 * @details The function validates the provided account derivation path in the
 * request. If invalid path is detected, the function will send an error to the
 * host and return false.
 *
 * @param request Reference to an instance of starknet_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(const starknet_sign_txn_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * STARKNET_SIGN_TXN_REQUEST_INITIATE_TAG type of request. Additionally, the
 * wallet-id is validated for sanity and the derivation path for the account is
 * also validated. After the validations, user is prompted about the action for
 * confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the starknet_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const starknet_query_t *query);

/**
 * @brief Receives unsigned txn from the host. If reception is successful, it
 * also parses the txn to ensure it's validity.
 * @note In case of any failure, a corresponding message is conveyed to the host
 *
 * @param query Reference to buffer of type starknet_query_t
 * @return true If the txn is received in the internal buffers and is valid
 * @return false If the txn could not be received or it's validation failed
 */
static bool fetch_valid_input(starknet_query_t *query);

/**
 * @brief This function executes user verification flow of the unsigned txn
 * received from the host.
 * @details The user verification flow is different for different type of action
 * types identified from the unsigned txn
 * @note This function expected that the unsigned txn is parsed using the helper
 * function as only few action types are supported currently.
 *
 * @return true If the user accepted the transaction display
 * @return false If any user rejection occured or P0 event occured
 */
static bool get_user_verification(void);

/**
 * @brief Calculates ED25519 curve based signature over the digest of the user
 * verified unsigned txn.
 * @details Seed reconstruction takes place within this function
 *
 * @param signature_buffer Reference to buffer where the signature will be
 * populated
 * @return true If the signature was computed successfully
 * @return false If signature could not be computed - maybe due to some error
 * during seed reconstruction phase
 */
static bool sign_txn(uint8_t *signature_buffer);

/**
 * @brief Sends signature of the STARKNET unsigned txn to the host
 * @details The function waits for the host to send a request of type
 * STARKNET_SIGN_TXN_REQUEST_SIGNATURE_TAG and sends the response
 *
 * @param query Reference to buffer of type starknet_query_t
 * @param signature Reference to signature to be sent to the host
 * @return true If the signature was sent successfully
 * @return false If the signature could not be sent - maybe due to and P0 event
 * or invalid request received from the host
 */
static bool send_signature(starknet_query_t *query, const uint8_t *signature);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static starknet_txn_context_t *starknet_txn_context = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool check_which_request(const starknet_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    starknet_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static void send_response(const pb_size_t which_response) {
  starknet_result_t result = init_starknet_result(STARKNET_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  starknet_send_result(&result);
}

static bool validate_request_data(const starknet_sign_txn_request_t *request) {
  bool status = true;

  if (!starknet_derivation_path_guard(
          request->initiate.derivation_path,
          request->initiate.derivation_path_count)) {
    starknet_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }
  return status;
}

static bool handle_initiate_query(const starknet_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(query, STARKNET_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             starknet_send_error)) {
    return false;
  }

  snprintf(msg,
           sizeof(msg),
           UI_TEXT_SIGN_TXN_PROMPT,
           starknet_app.name,
           wallet_name);
  // Take user consent to sign transaction for the wallet
  if (!core_confirmation(msg, starknet_send_error)) {
    return false;
  }

  set_app_flow_status(STARKNET_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&starknet_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(starknet_sign_txn_initiate_request_t));

  send_response(STARKNET_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool fetch_valid_input(starknet_query_t *query) {
  if (!starknet_get_query(query, STARKNET_QUERY_SIGN_TXN_TAG) &&
      !check_which_request(query, STARKNET_SIGN_TXN_REQUEST_TXN_TAG)) {
    return false;
  }

  memcpy(starknet_txn_context->transaction,
         query->sign_txn.txn.txn.bytes,
         query->sign_txn.txn.txn.size);

  if (1) {
    send_response(STARKNET_SIGN_TXN_RESPONSE_UNSIGNED_TXN_ACCEPTED_TAG);
    return true;
  } else {
    starknet_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  return false;
}

static bool get_user_verification(void) {
  bool user_verified = false;
  char msg[128] = "0x";

  byte_array_to_hex_string(starknet_txn_context->transaction, 32, &msg[2], 126);
  // TODO: Add blind signing warning
  user_verified =
      core_confirmation(UI_TEXT_BLIND_SIGNING_WARNING, starknet_send_error);
  user_verified &= core_scroll_page(NULL, msg, starknet_send_error);

  if (user_verified) {
    set_app_flow_status(STARKNET_SIGN_TXN_STATUS_VERIFY);
  }

  return user_verified;
}

static bool sign_txn(uint8_t *signature_buffer) {
  uint8_t seed[64] = {0};
  if (!reconstruct_seed(starknet_txn_context->init_info.wallet_id,
                        seed,
                        starknet_send_error)) {
    memzero(seed, sizeof(seed));
    return false;
  }

  set_app_flow_status(STARKNET_SIGN_TXN_STATUS_SEED_GENERATED);

  uint8_t stark_key[32] = {0};
  if (starknet_derive_bip32_node(seed, stark_key) &&
      starknet_derive_key_from_seed(
          stark_key,
          starknet_txn_context->init_info.derivation_path,
          starknet_txn_context->init_info.derivation_path_count,
          stark_key)) {
    // TODO: Generate signature using stark_key
    memcpy(signature_buffer, stark_key, sizeof(stark_key));
  } else {
    starknet_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
  }

  memzero(seed, sizeof(seed));
  memzero(stark_key, sizeof(stark_key));

  return true;
}

static bool send_signature(starknet_query_t *query, const uint8_t *signature) {
  starknet_result_t result = init_starknet_result(STARKNET_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = STARKNET_SIGN_TXN_RESPONSE_SIGNATURE_TAG;

  if (!starknet_get_query(query, STARKNET_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, STARKNET_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(&result.sign_txn.signature.signature[0],
         signature,
         sizeof(result.sign_txn.signature.signature));
  starknet_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void starknet_sign_transaction(starknet_query_t *query) {
  starknet_txn_context =
      (starknet_txn_context_t *)malloc(sizeof(starknet_txn_context_t));
  memzero(starknet_txn_context, sizeof(starknet_txn_context_t));

  uint8_t signature[64] = {0};
  memzero(signature, sizeof(signature));

  if (handle_initiate_query(query) && fetch_valid_input(query) &&
      get_user_verification() && sign_txn(signature) &&
      send_signature(query, (const uint8_t *)signature)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  memzero(signature, sizeof(signature));

  if (starknet_txn_context) {
    free(starknet_txn_context);
    starknet_txn_context = NULL;
  }

  return;
}