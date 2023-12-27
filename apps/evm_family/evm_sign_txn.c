/**
 * @file    evm_sign_txn.c
 * @author  Cypherock X1 Team
 * @brief   EVM chain transaction signing logic
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

#include "address.h"
#include "evm_api.h"
#include "evm_helpers.h"
#include "evm_priv.h"
#include "evm_user_verification.h"
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
 * fails, then it will send an error to the host bitcoin app and return false.
 *
 * @param query Reference to an instance of evm_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const evm_query_t *query,
                                pb_size_t which_request);

/**
 * @brief Validates the derivation path received in the request from host
 * @details The function validates the provided account derivation path in the
 * request. If invalid path is detected, the function will send an error to the
 * host and return false.
 *
 * @param request Reference to an instance of evm_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(const evm_sign_txn_request_t *request);

/**
 * @brief The function prepares and sends empty responses
 *
 * @param which_response Constant value for the response type to be sent
 */
static void send_response(pb_size_t which_response);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the EVM_SIGN_TXN type
 * of request. Additionally, the wallet-id is validated for sanity and the
 * derivation path for the account is also validated. After the validations,
 * user is prompted about the action for confirmation. The function returns true
 * indicating all the validation and user confirmation was a success. The
 * function also duplicates the data from query into the evm_txn_context  for
 * further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
STATIC bool handle_initiate_query(const evm_query_t *query);

/**
 * @brief Fetches complete raw transaction to be signed for verification
 * @details The function will try to fetch the transaction by referring to the
 * declared size in evm_txn_context. The function will store complete
 * transaction into evm_txn_context.transaction.
 *
 * @param query Reference to an instance of evm_query_t for storing the
 * transient transaction chunks.
 *
 * @return bool Indicating if the whole transaction received and verified
 * @retval true If all the transaction was fetched and verified
 * @retval false If the transaction failed verification or wasn't fetched
 */
STATIC bool fetch_valid_transaction(evm_query_t *query);

/**
 * @brief Aggregates user consent for the transaction info
 * @details The function decodes the receiver address along with the
 * corresponding transfer value in ETH. It also calculates the transaction fee
 * and confirms with the user.
 *
 * @return bool Indicating if the user confirmed the transaction
 * @retval true If user confirmed the fee and the receiver address along with
 * the corresponding value.
 * @retval false Immediate return if any of the confirmation is disapproved
 */
STATIC bool get_user_verification();

/**
 * @brief Signs the user verified transaction and returns the signature.
 * @details The function internally calls wallet reconstruction sub-flow to get
 * access to the seed. The function will ensure clearing of all the keys
 * and other sensitive data (such as seed, HDNode, etc.) before exiting and
 * should ensure no leakage of secret data.
 *
 * @param sig Reference to the struct having storage for signature components
 *
 * @return bool Indicating if signature for the provided transaction was
 * successfully generated
 * @retval true If all the signature is generated without any error
 * @retval false If any of the singature failed to generate
 */
static bool sign_transaction(evm_sign_txn_signature_response_t *sig);

/**
 * @brief Sends the generated signature to the host
 *
 * @param query Reference to an instance of evm_query_t to store transient
 * request from the host
 * @param sig Reference to the struct having the signature components
 *
 * @return bool Indicating if the signature is sent to the host
 * @retval true If the signature was sent to host successfully
 * @retval false If the host responded with unknown/wrong query
 */
static bool send_signature(evm_query_t *query,
                           evm_sign_txn_signature_response_t *sig);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

STATIC evm_txn_context_t *txn_context = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const evm_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request_data(const evm_sign_txn_request_t *request) {
  bool status = true;

  if (!evm_derivation_path_guard(request->initiate.derivation_path,
                                 request->initiate.derivation_path_count)) {
    evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  /* hard limit on the transaction size that can be fetched and kept in RAM.
   * this limit can be removed once the RLP decoder can operate on running
   * transaction buffer stream and get user confirmations on the go
   */
  if (EVM_TRANSACTION_SIZE_CAP < request->initiate.transaction_size) {
    evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }
  return status;
}

static void send_response(const pb_size_t which_response) {
  evm_result_t result = init_evm_result(EVM_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  evm_send_result(&result);
}

STATIC bool handle_initiate_query(const evm_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(query, EVM_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             evm_send_error)) {
    return false;
  }

  // TODO: handle prompts for different transaction types
  snprintf(
      msg, sizeof(msg), UI_TEXT_SIGN_TXN_PROMPT, g_evm_app->name, wallet_name);
  // Take user consent to sign the transaction for the wallet
  if (!core_confirmation(msg, evm_send_error)) {
    return false;
  }

  set_app_flow_status(EVM_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(evm_sign_txn_initiate_request_t));
  send_response(EVM_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

STATIC bool fetch_valid_transaction(evm_query_t *query) {
  bool status = false;
  uint32_t size = 0;
  evm_result_t result = init_evm_result(EVM_RESULT_SIGN_TXN_TAG);
  uint32_t total_size = txn_context->init_info.transaction_size;
  const evm_sign_txn_data_t *txn_data = &query->sign_txn.txn_data;
  const common_chunk_payload_t *payload = &txn_data->chunk_payload;
  const common_chunk_payload_chunk_t *chunk = &txn_data->chunk_payload.chunk;

  result.sign_txn.which_response = EVM_SIGN_TXN_RESPONSE_DATA_ACCEPTED_TAG;
  result.sign_txn.data_accepted.has_chunk_ack = true;
  // allocate memory for storing transaction
  txn_context->transaction = (uint8_t *)malloc(total_size);
  while (true) {
    if (!evm_get_query(query, EVM_QUERY_SIGN_TXN_TAG) &&
        !check_which_request(query, EVM_SIGN_TXN_REQUEST_TXN_DATA_TAG)) {
      return status;
    }
    if (!txn_data->has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        size + payload->chunk.size > total_size) {
      evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                     ERROR_DATA_FLOW_INVALID_DATA);
      return status;
    }

    memcpy(&txn_context->transaction[size], chunk->bytes, chunk->size);
    size += chunk->size;
    result.sign_txn.data_accepted.chunk_ack.chunk_index = payload->chunk_index;
    evm_send_result(&result);
    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }

  // make sure all chunks were received
  if (size != total_size) {
    evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    return status;
  }
  // decode and verify the received transaction
  if (0 != evm_decode_unsigned_txn(
               txn_context->transaction, total_size, txn_context) ||
      EVM_TXN_INVALID_DATA == txn_context->txn_type ||
      !evm_validate_unsigned_txn(txn_context)) {
    evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    return status;
  }

  status = true;
  return status;
}

STATIC bool get_user_verification() {
  bool status = false;
  switch (txn_context->txn_type) {
    case EVM_TXN_NO_DATA:
    case EVM_TXN_TOKEN_TRANSFER_FUNC:
      status = evm_verify_transfer(txn_context);
      break;

    case EVM_TXN_KNOWN_FUNC_SIG:
      status = evm_verify_clear_signing(txn_context);
      break;

    case EVM_TXN_UNKNOWN_FUNC_SIG:
      // TODO: link with wallet setting
      status = evm_verify_blind_signing(txn_context);
      break;

    case EVM_TXN_INVALID_DATA:
    default:
      // cannot reach; should be caught already
      break;
  }

  if (status) {
    set_app_flow_status(EVM_SIGN_TXN_STATUS_VERIFY);
  }
  return status;
}

static bool sign_transaction(evm_sign_txn_signature_response_t *sig) {
  bool status = false;
  HDNode node = {0};
  uint8_t buffer[64] = {0};
  const size_t depth = txn_context->init_info.derivation_path_count;
  const uint32_t *hd_path = txn_context->init_info.derivation_path;
  const ecdsa_curve *curve = get_curve_by_name(SECP256K1_NAME)->params;

  if (!reconstruct_seed(
          txn_context->init_info.wallet_id, buffer, evm_send_error)) {
    memzero(buffer, sizeof(buffer));
    return status;
  }

  set_app_flow_status(EVM_SIGN_TXN_STATUS_SEED_GENERATED);

  status =
      derive_hdnode_from_path(hd_path, depth, SECP256K1_NAME, buffer, &node);
  memzero(buffer, sizeof(buffer));
  if (!status) {
    evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
  } else {
    status = true;
    keccak_256(txn_context->transaction,
               txn_context->init_info.transaction_size,
               buffer);

    if (0 != ecdsa_sign_digest(
                 curve, node.private_key, buffer, sig->r, sig->v, NULL)) {
      evm_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
      status = false;
    }
  }
  memzero(&node, sizeof(HDNode));
  return status;
}

static bool send_signature(evm_query_t *query,
                           evm_sign_txn_signature_response_t *sig) {
  evm_result_t result = init_evm_result(EVM_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = EVM_SIGN_TXN_RESPONSE_SIGNATURE_TAG;
  if (!evm_get_query(query, EVM_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, EVM_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(&result.sign_txn.signature,
         sig,
         sizeof(evm_sign_txn_signature_response_t));
  evm_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void evm_sign_transaction(evm_query_t *query) {
  txn_context = (evm_txn_context_t *)malloc(sizeof(evm_txn_context_t));
  memzero(txn_context, sizeof(evm_txn_context_t));
  evm_sign_txn_signature_response_t sig = {0};

  if (handle_initiate_query(query) && fetch_valid_transaction(query) &&
      get_user_verification() && sign_transaction(&sig) &&
      send_signature(query, &sig)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  if (NULL != txn_context->transaction) {
    free(txn_context->transaction);
  }
  if (NULL != txn_context) {
    free(txn_context);
    txn_context = NULL;
  }
}