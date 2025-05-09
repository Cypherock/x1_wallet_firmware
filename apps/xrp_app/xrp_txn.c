/**
 * @file    xrp_txn.c
 * @author  Cypherock X1 Team
 * @brief   Source file to handle transaction signing logic for XRP protocol
 *
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
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

#include <stdint.h>

#include "composable_app_queue.h"
#include "exchange_main.h"
#include "reconstruct_wallet_flow.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "wallet_list.h"
#include "xrp_api.h"
#include "xrp_context.h"
#include "xrp_helpers.h"
#include "xrp_priv.h"
#include "xrp_txn_helpers.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef xrp_sign_txn_signature_response_signature_t der_sig_t;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host bitcoin app and return false.
 *
 * @param query Reference to an instance of xrp_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const xrp_query_t *query,
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
 * @param request Reference to an instance of xrp_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(const xrp_sign_txn_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * XRP_SIGN_TXN_REQUEST_INITIATE_TAG type of request. Additionally, the
 * wallet-id is validated for sanity and the derivation path for the account is
 * also validated. After the validations, user is prompted about the action for
 * confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the xrp_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const xrp_query_t *query);

/**
 * @brief Receives unsigned txn from the host. If reception is successful, it
 * also parses the txn to ensure it's validity.
 * @note In case of any failure, a corresponding message is conveyed to the host
 *
 * @param query Reference to buffer of type xrp_query_t
 * @return true If the txn is received in the internal buffers and is valid
 * @return false If the txn could not be received or it's validation failed
 */
static bool fetch_valid_input(xrp_query_t *query);

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
 * @brief Sends signature of the XRP unsigned txn to the host
 * @details The function waits for the host to send a request of type
 * XRP_SIGN_TXN_REQUEST_SIGNATURE_TAG and sends the response
 *
 * @param query Reference to buffer of type xrp_query_t
 * @param signature Reference to signature to be sent to the host
 * @return true If the signature was sent successfully
 * @return false If the signature could not be sent - maybe due to and P0 event
 * or invalid request received from the host
 */
static bool send_signature(xrp_query_t *query, const der_sig_t *der_signature);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static xrp_txn_context_t *xrp_txn_context = NULL;
static bool use_signature_verification = false;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool check_which_request(const xrp_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    xrp_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static void send_response(const pb_size_t which_response) {
  xrp_result_t result = init_xrp_result(XRP_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  xrp_send_result(&result);
}

static bool validate_request_data(const xrp_sign_txn_request_t *request) {
  bool status = true;

  if (!xrp_derivation_path_guard(request->initiate.derivation_path,
                                 request->initiate.derivation_path_count)) {
    xrp_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
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

static bool handle_initiate_query(const xrp_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  // TODO: Handle wallet search failures - eg: Wallet ID not found, Wallet
  // ID found but is invalid/locked wallet
  if (!check_which_request(query, XRP_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             xrp_send_error)) {
    return false;
  }

  snprintf(msg, sizeof(msg), UI_TEXT_SIGN_TXN_PROMPT, XRP_NAME, wallet_name);
  // Take user consent to sign transaction for the wallet
  if (!core_confirmation(msg, xrp_send_error)) {
    return false;
  }

  set_app_flow_status(XRP_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&xrp_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(xrp_sign_txn_initiate_request_t));

  send_response(XRP_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool fetch_valid_input(xrp_query_t *query) {
  uint32_t size = 0;
  xrp_result_t response = init_xrp_result(XRP_RESULT_SIGN_TXN_TAG);
  uint32_t total_size = xrp_txn_context->init_info.transaction_size;
  const xrp_sign_txn_data_t *txn_data = &query->sign_txn.txn_data;
  const common_chunk_payload_t *payload = &txn_data->chunk_payload;
  const common_chunk_payload_chunk_t *chunk = &txn_data->chunk_payload.chunk;

  // allocate memory for storing transaction
  xrp_txn_context->transaction = (uint8_t *)malloc(total_size);
  while (1) {
    if (!xrp_get_query(query, XRP_QUERY_SIGN_TXN_TAG) ||
        !check_which_request(query, XRP_SIGN_TXN_REQUEST_TXN_DATA_TAG)) {
      return false;
    }

    if (!txn_data->has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        size + payload->chunk.size > total_size) {
      xrp_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                     ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    memcpy(&xrp_txn_context->transaction[size], chunk->bytes, chunk->size);
    size += chunk->size;
    // Send chunk ack to host
    response.sign_txn.which_response = XRP_SIGN_TXN_RESPONSE_DATA_ACCEPTED_TAG;
    response.sign_txn.data_accepted.has_chunk_ack = true;
    response.sign_txn.data_accepted.chunk_ack.chunk_index =
        payload->chunk_index;
    xrp_send_result(&response);

    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }

  // make sure all chunks were received
  if (size != total_size) {
    xrp_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }
  xrp_txn_context->raw_txn =
      (xrp_unsigned_txn *)malloc(sizeof(xrp_unsigned_txn));

  if (!xrp_parse_transaction(
          xrp_txn_context->transaction, total_size, xrp_txn_context->raw_txn)) {
    xrp_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  return true;
}

static bool get_user_verification(void) {
  const xrp_unsigned_txn *decoded_utxn = xrp_txn_context->raw_txn;

  char to_address[XRP_ACCOUNT_ADDRESS_LENGTH] = "";

  uint8_t prefixed_account_id[XRP_PREFIXED_ACCOUNT_ID_LENGTH];
  prefixed_account_id[0] = 0x00;
  memcpy(prefixed_account_id + 1, decoded_utxn->Destination, 20);

  // xrp uses different base58 dictionary, that's why a custom function
  if (!base58_encode_check_with_custom_digits_order(
          prefixed_account_id,
          XRP_PREFIXED_ACCOUNT_ID_LENGTH,
          HASHER_SHA2D,
          to_address,
          XRP_ACCOUNT_ADDRESS_LENGTH + 1,
          XRP_BASE58_DIGITS_ORDERED)) {
    xrp_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  if (use_signature_verification) {
    if (!exchange_validate_stored_signature(to_address, sizeof(to_address))) {
      return false;
    }
  }

  if (!core_scroll_page(ui_text_verify_address, to_address, xrp_send_error)) {
    return false;
  }

  // verify recipient amount
  uint64_t amount = 0;
  memcpy(&amount, &decoded_utxn->Amount, sizeof(uint64_t));
  char amount_string[30] = {'\0'};
  double decimal_amount = (double)amount;
  decimal_amount *= 1e-6;
  snprintf(amount_string, sizeof(amount_string), "%.6f", decimal_amount);

  char display[100] = {'\0'};
  snprintf(display,
           sizeof(display),
           UI_TEXT_VERIFY_AMOUNT,
           amount_string,
           XRP_LUNIT);

  if (!core_confirmation(display, xrp_send_error)) {
    return false;
  }

  if (decoded_utxn->hasDestinationTag) {
    // verify destination tag
    uint32_t tag = 0;
    memcpy(&tag, &decoded_utxn->DestinationTag, sizeof(uint32_t));

    char display_tag[50] = {'\0'};
    snprintf(
        display_tag, sizeof(display_tag), UI_TEXT_VERIFY_DESTINATION_TAG, tag);

    if (!core_confirmation(display_tag, xrp_send_error)) {
      return false;
    }
  }

  set_app_flow_status(XRP_SIGN_TXN_STATUS_VERIFY);

  return true;
}

static bool sign_txn(der_sig_t *der_signature) {
  uint8_t seed[64] = {0};
  if (!reconstruct_seed(
          xrp_txn_context->init_info.wallet_id, seed, xrp_send_error)) {
    memzero(seed, sizeof(seed));
    // TODO: handle errors of reconstruction flow
    return false;
  }

  set_app_flow_status(XRP_SIGN_TXN_STATUS_SEED_GENERATED);

  uint8_t digest[SHA512_DIGEST_LENGTH] = {0};
  sha512_Raw(xrp_txn_context->transaction,
             xrp_txn_context->init_info.transaction_size,
             digest);

  HDNode hdnode = {0};
  derive_hdnode_from_path(xrp_txn_context->init_info.derivation_path,
                          xrp_txn_context->init_info.derivation_path_count,
                          SECP256K1_NAME,
                          seed,
                          &hdnode);

  uint8_t signature[64];
  ecdsa_sign_digest(
      &secp256k1, hdnode.private_key, digest, signature, NULL, NULL);

  der_signature->size = ecdsa_sig_to_der(signature, der_signature->bytes);

  memzero(digest, sizeof(digest));
  memzero(seed, sizeof(seed));
  memzero(&hdnode, sizeof(hdnode));
  memzero(signature, sizeof(signature));

  return true;
}

static bool send_signature(xrp_query_t *query, const der_sig_t *der_signature) {
  xrp_result_t result = init_xrp_result(XRP_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = XRP_SIGN_TXN_RESPONSE_SIGNATURE_TAG;

  if (!xrp_get_query(query, XRP_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, XRP_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(
      &result.sign_txn.signature.signature, der_signature, sizeof(der_sig_t));

  xrp_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void xrp_sign_transaction(xrp_query_t *query) {
  xrp_txn_context = (xrp_txn_context_t *)malloc(sizeof(xrp_txn_context_t));
  memzero(xrp_txn_context, sizeof(xrp_txn_context_t));

  der_sig_t der_signature = {0};

  if (handle_initiate_query(query) && fetch_valid_input(query) &&
      get_user_verification() && sign_txn(&der_signature) &&
      send_signature(query, &der_signature)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  if (xrp_txn_context) {
    free(xrp_txn_context);
    xrp_txn_context = NULL;
  }

  return;
}
