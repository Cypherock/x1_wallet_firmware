/**
 * @file    constellation_txn.c
 * @author  Cypherock X1 Team
 * @brief   Source file to handle transaction signing logic for CONSTELLATION
 *protocol
 *
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2024 by HODL TECH PTE LTD
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

#include <stdbool.h>
#include <stdint.h>

#include "coin_utils.h"
#include "composable_app_queue.h"
#include "constellation/core.pb.h"
#include "constellation/sign_txn.pb.h"
#include "constellation_api.h"
#include "constellation_context.h"
#include "constellation_helpers.h"
#include "constellation_priv.h"
#include "exchange_main.h"
#include "reconstruct_wallet_flow.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "utils.h"
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
typedef constellation_sign_txn_signature_response_signature_t der_sig_t;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host bitcoin app and return false.
 *
 * @param query Reference to an instance of constellation_query_t containing
 * query received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const constellation_query_t *query,
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
 * @param request Reference to an instance of constellation_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(
    const constellation_sign_txn_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * CONSTELLATION_SIGN_TXN_REQUEST_INITIATE_TAG type of request. Additionally,
 * the wallet-id is validated for sanity and the derivation path for the account
 * is also validated. After the validations, user is prompted about the action
 * for confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the constellation_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const constellation_query_t *query);

/**
 * @brief Receives unsigned txn from the host. If reception is successful, it
 * also parses the txn to ensure it's validity.
 * @note In case of any failure, a corresponding message is conveyed to the host
 *
 * @param query Reference to buffer of type constellation_query_t
 * @return true If the txn is received in the internal buffers and is valid
 * @return false If the txn could not be received or it's validation failed
 */
static bool fetch_valid_input(constellation_query_t *query);

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
static bool sign_txn(der_sig_t *der_signature);

/**
 * @brief Sends signature of the CONSTELLATION unsigned txn to the host
 * @details The function waits for the host to send a request of type
 * CONSTELLATION_SIGN_TXN_REQUEST_SIGNATURE_TAG and sends the response
 *
 * @param query Reference to buffer of type constellation_query_t
 * @param signature Reference to signature to be sent to the host
 * @return true If the signature was sent successfully
 * @return false If the signature could not be sent - maybe due to and P0 event
 * or invalid request received from the host
 */
static bool send_signature(constellation_query_t *query,
                           const der_sig_t *der_signature);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static constellation_txn_context_t *constellation_txn_context = NULL;
static bool use_signature_verification = false;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool check_which_request(const constellation_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    constellation_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static void send_response(const pb_size_t which_response) {
  constellation_result_t result =
      init_constellation_result(CONSTELLATION_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  constellation_send_result(&result);
}

static bool validate_request_data(
    const constellation_sign_txn_request_t *request) {
  bool status = true;

  if (!constellation_derivation_path_guard(
          request->initiate.derivation_path,
          request->initiate.derivation_path_count)) {
    constellation_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  caq_node_data_t data = {.applet_id = get_applet_id()};

  memzero(data.params, sizeof(data.params));
  memcpy(data.params,
         request->initiate.wallet_id,
         sizeof(request->initiate.wallet_id));
  data.params[32] = EXCHANGE_FLOW_TAG_SEND;

  use_signature_verification = exchange_app_validate_caq(data);

  return status;
}

static bool handle_initiate_query(const constellation_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  // TODO: Handle wallet search failures - eg: Wallet ID not found, Wallet
  // ID found but is invalid/locked wallet
  if (!check_which_request(query,
                           CONSTELLATION_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             constellation_send_error)) {
    return false;
  }

  snprintf(msg,
           sizeof(msg),
           UI_TEXT_SIGN_TXN_PROMPT,
           CONSTELLATION_NAME,
           wallet_name);
  // Take user consent to sign transaction for the wallet
  if (!core_confirmation(msg, constellation_send_error)) {
    return false;
  }

  set_app_flow_status(CONSTELLATION_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&constellation_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(constellation_sign_txn_initiate_request_t));

  send_response(CONSTELLATION_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool fetch_valid_input(constellation_query_t *query) {
  if (!constellation_get_query(query, CONSTELLATION_QUERY_SIGN_TXN_TAG) &&
      !check_which_request(query,
                           CONSTELLATION_SIGN_TXN_REQUEST_TXN_DATA_TAG)) {
    return false;
  }

  const constellation_sign_txn_data_t *sign_txn_data =
      &query->sign_txn.txn_data;
  if (sign_txn_data->has_txn == false) {
    return false;
  }

  constellation_txn_context->txn = &sign_txn_data->txn;

  send_response(CONSTELLATION_SIGN_TXN_RESPONSE_UNSIGNED_TXN_ACCEPTED_TAG);

  return true;
}

static bool get_user_verification(void) {
  const constellation_transaction_t *txn = constellation_txn_context->txn;

  if (use_signature_verification) {
    if (!exchange_validate_stored_signature((char *)txn->destination,
                                            sizeof(txn->destination))) {
      return false;
    }
  }

  // verify recipient address
  if (!core_scroll_page(
          ui_text_verify_address, txn->destination, constellation_send_error)) {
    return false;
  }

  // verify recipient amount
  char amount_string[30] = {'\0'};
  double decimal_amount = (double)txn->amount;
  decimal_amount *= 1e-8;
  snprintf(amount_string, sizeof(amount_string), "%.*g", 8, decimal_amount);

  char display[100] = {'\0'};
  snprintf(display,
           sizeof(display),
           UI_TEXT_VERIFY_AMOUNT,
           amount_string,
           "");    // We don't know the which transaction(coin or token and/or
                   // which token) is it, hence we don't know unit

  if (!core_confirmation(display, constellation_send_error)) {
    return false;
  }

  // verify fee
  char fee_string[30] = {'\0'};
  double decimal_fee = (double)txn->fee;
  decimal_fee *= 1e-8;
  snprintf(fee_string, sizeof(fee_string), "%.*g", 8, decimal_fee);

  char fee_display[100] = {'\0'};
  snprintf(
      fee_display, sizeof(fee_display), UI_TEXT_VERIFY_FEE, fee_string, "");

  if (!core_confirmation(fee_display, constellation_send_error)) {
    return false;
  }

  set_app_flow_status(CONSTELLATION_SIGN_TXN_STATUS_VERIFY);

  return true;
}

static bool sign_txn(der_sig_t *der_signature) {
  uint8_t seed[64] = {0};
  if (!reconstruct_seed(constellation_txn_context->init_info.wallet_id,
                        seed,
                        constellation_send_error)) {
    memzero(seed, sizeof(seed));
    // TODO: handle errors of reconstruction flow
    return false;
  }

  set_app_flow_status(CONSTELLATION_SIGN_TXN_STATUS_SEED_GENERATED);

  uint8_t serialized_txn[1024] = {0};
  size_t serialized_txn_len = 0;
  serialize_txn(
      constellation_txn_context->txn, serialized_txn, &serialized_txn_len);

  uint8_t sha256_digest[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw(serialized_txn, serialized_txn_len, sha256_digest);

  char sha256_hex_str[SHA256_DIGEST_LENGTH * 2 + 1] = "";
  byte_array_to_hex_string(sha256_digest,
                           sizeof(sha256_digest),
                           sha256_hex_str,
                           sizeof(sha256_hex_str));

  uint8_t sha512_digest[SHA512_DIGEST_LENGTH] = {0};
  sha512_Raw((uint8_t *)sha256_hex_str, strlen(sha256_hex_str), sha512_digest);

  HDNode hdnode = {0};
  derive_hdnode_from_path(
      constellation_txn_context->init_info.derivation_path,
      constellation_txn_context->init_info.derivation_path_count,
      SECP256K1_NAME,
      seed,
      &hdnode);

  uint8_t signature[64] = {0};
  ecdsa_sign_digest(
      &secp256k1, hdnode.private_key, sha512_digest, signature, NULL, NULL);

  der_signature->size = ecdsa_sig_to_der(signature, der_signature->bytes);

  memzero(serialized_txn, sizeof(serialized_txn));
  memzero(sha256_digest, sizeof(sha256_digest));
  memzero(sha512_digest, sizeof(sha512_digest));
  memzero(seed, sizeof(seed));
  memzero(&hdnode, sizeof(hdnode));
  memzero(signature, sizeof(signature));

  return true;
}

static bool send_signature(constellation_query_t *query,
                           const der_sig_t *der_signature) {
  constellation_result_t result =
      init_constellation_result(CONSTELLATION_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response =
      CONSTELLATION_SIGN_TXN_RESPONSE_SIGNATURE_TAG;

  if (!constellation_get_query(query, CONSTELLATION_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query,
                           CONSTELLATION_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(
      &result.sign_txn.signature.signature, der_signature, sizeof(der_sig_t));

  constellation_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void constellation_sign_transaction(constellation_query_t *query) {
  constellation_txn_context = (constellation_txn_context_t *)malloc(
      sizeof(constellation_txn_context_t));
  memzero(constellation_txn_context, sizeof(constellation_txn_context_t));

  der_sig_t der_signature = {0};

  if (handle_initiate_query(query) && fetch_valid_input(query) &&
      get_user_verification() && sign_txn(&der_signature) &&
      send_signature(query, &der_signature)) {
    delay_scr_init(ui_text_check_software_wallet_app, DELAY_TIME);
  }

  if (constellation_txn_context) {
    free(constellation_txn_context);
    constellation_txn_context = NULL;
  }
}