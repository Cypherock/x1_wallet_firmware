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

typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by handle_initiate_query()
   */
  evm_sign_txn_initiate_request_t init_info;

  /// remembers the allocated buffer for holding complete unsigned transaction
  uint8_t *transaction;
  /// store for decoded unsigned transaction info
  eth_unsigned_txn transaction_info;
} evm_txn_context_t;

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
static bool handle_initiate_query(const evm_query_t *query);

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
static bool fetch_valid_transaction(evm_query_t *query);

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
static bool get_user_verification();

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

static evm_txn_context_t *txn_context = NULL;

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
  if (EVM_TRANSACTION_SIZE_CAP < txn_context->init_info.transaction_size) {
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

static bool handle_initiate_query(const evm_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(query, EVM_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             evm_send_error)) {
    return false;
  }

  // TODO: need to show token symbol for token transactions
  snprintf(msg,
           sizeof(msg),
           UI_TEXT_SEND_PROMPT,
           g_evm_app->lunit_name,
           g_evm_app->name,
           wallet_name);
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

static bool fetch_valid_transaction(evm_query_t *query) {
  bool status = false;

  // TODO: stitch transactions split over multiple chunks
  if (!evm_get_query(query, EVM_QUERY_SIGN_TXN_TAG) &&
      !check_which_request(query, EVM_SIGN_TXN_REQUEST_TXN_DATA_TAG)) {
    return status;
  }
  const evm_sign_txn_data_t *txn_data = &query->sign_txn.txn_data;
  if (!txn_data->has_chunk_payload) {
    evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    return status;
  }

  // store the received transaction
  if (NULL == txn_context->transaction) {
    txn_context->transaction =
        (uint8_t *)malloc(sizeof(common_chunk_payload_chunk_t));
  }
  const common_chunk_payload_chunk_t *chunk = &txn_data->chunk_payload.chunk;
  txn_metadata dummy_metadata;
  memcpy(txn_context->transaction, chunk->bytes, chunk->size);
  // decode and verify the received transaction
  if (0 != eth_byte_array_to_unsigned_txn(
               txn_context->transaction,
               txn_context->init_info.transaction_size,
               &txn_context->transaction_info,
               &dummy_metadata) ||
      !eth_validate_unsigned_txn(&txn_context->transaction_info,
                                 &dummy_metadata)) {
    return status;
  }

  status = true;
  return status;
}

static bool get_user_verification() {
  bool status = false;
  char address[43] = "0x";
  uint8_t *to_address = NULL;
  uint32_t *hd_path = txn_context->init_info.derivation_path;
  size_t depth = txn_context->init_info.derivation_path_count;

  // show warning for not-whitelisted contracts; take user consent
  if (PAYLOAD_CONTRACT_NOT_WHITELISTED ==
      txn_context->transaction_info.payload_status) {
    to_address = txn_context->transaction_info.to_address;
    byte_array_to_hex_string(
        to_address, EVM_ADDRESS_LENGTH, &address[2], sizeof(address) - 2);
    delay_scr_init(ui_text_unverified_contract, DELAY_TIME);

    if (!core_scroll_page(ui_text_verify_contract, address, evm_send_error)) {
      return status;
    }
  }

  // TODO: decide on handling blind signing
  // show warning for unknown EVM function; take user consent
  if (PAYLOAD_SIGNATURE_NOT_WHITELISTED ==
      txn_context->transaction_info.payload_status) {
    hd_path_array_to_string(hd_path, depth, false, address, sizeof(address));
    if (!core_confirmation(UI_TEXT_BLIND_SIGNING_WARNING, evm_send_error) ||
        !core_scroll_page(UI_TEXT_VERIFY_HD_PATH, address, evm_send_error)) {
      return status;
    }
  }

  // TODO: verify transaction nonce; this is pending on settings option
#if 0
  // TODO: convert byte to value
  if (!core_scroll_page("Verify nonce", address, evm_send_error)) {
    return status;
  }
#endif

  // verify recipient address; TODO: handle harmony address encoding
  address[0] = '0';
  address[1] = 'x';
  eth_get_to_address(&txn_context->transaction_info, to_address);
  ethereum_address_checksum(
      to_address, &address[2], sizeof(address) - 2, false);
  if (!core_scroll_page(ui_text_verify_address, address, evm_send_error)) {
    return status;
  }

  // verify recipient amount
  // TODO: handle contract transactions
  char value_string[65] = {'\0'};
  char value_decimal_string[30] = {'\0'};
  char display[100] = "";
  uint8_t len = eth_get_value(&eth_unsigned_txn_ptr, value_string);
  if (!convert_byte_array_to_decimal_string(len,
                                            ETH_DECIMAL,
                                            value_string,
                                            value_decimal_string,
                                            sizeof(value_decimal_string))) {
    evm_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    return status;
  }

  snprintf(display,
           sizeof(display),
           UI_TEXT_VERIFY_AMOUNT,
           value_decimal_string,
           g_evm_app->lunit_name);
  if (!core_confirmation(display, evm_send_error)) {
    return status;
  }

  // verify transaction fee
  eth_get_fee_string(
      &eth_unsigned_txn_ptr, value_string, sizeof(value_string), ETH_DECIMAL);
  snprintf(display,
           sizeof(display),
           UI_TEXT_SEND_TXN_FEE,
           value_string,
           g_evm_app->lunit_name);
  if (!core_confirmation(display, evm_send_error)) {
    return status;
  }
  status = true;
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