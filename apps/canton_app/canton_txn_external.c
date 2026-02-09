/**
 * @file    canton_txn.c
 * @author  Cypherock X1 Team
 * @brief   Source file to handle transaction signing logic for CANTON protocol
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

#include "canton/canton_topology_txn.pb.h"
#include "canton/sign_txn_external.pb.h"
#include "canton_api.h"
#include "canton_context.h"
#include "canton_helpers.h"
#include "canton_priv.h"
#include "coin_utils.h"
#include "constant_texts.h"
#include "curves.h"
#include "reconstruct_wallet_flow.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_delay.h"
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
typedef canton_sign_txn_external_signature_response_t canton_external_sig_t;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host bitcoin app and return false.
 *
 * @param query Reference to an instance of canton_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const canton_query_t *query,
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
 * @param request Reference to an instance of canton_sign_txn_external_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(
    const canton_sign_txn_external_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * CANTON_SIGN_TXN_EXTERNAL_REQUEST_INITIATE_TAG type of request. Additionally,
 * the wallet-id is validated for sanity and the derivation path for the account
 * is also validated. After the validations, user is prompted about the action
 * for confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the canton_txn_external_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const canton_query_t *query);

/**
 * @brief Receives unsigned txn from the host. If reception is successful, it
 * also parses the txn to ensure it's validity.
 * @note In case of any failure, a corresponding message is conveyed to the host
 *
 * @param query Reference to buffer of type canton_query_t
 * @return true If the txn is received in the internal buffers and is valid
 * @return false If the txn could not be received or it's validation failed
 */
static bool fetch_valid_input(canton_query_t *query);

/**
 * @brief This function executes user verification flow of the unsigned txn
 * received from the host.
 * @details The user verification flow is different for different type of
 * transaction types identified from the unsigned txn
 * @note This function expected that the unsigned txn is parsed using the helper
 * function as only few transaction types are supported currently.
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
static bool sign_txn_external(canton_external_sig_t *sig);

/**
 * @brief Sends signature of the CANTON unsigned txn to the host
 * @details The function waits for the host to send a request of type
 * CANTON_SIGN_TXN_EXTERNAL_REQUEST_SIGNATURE_TAG and sends the response
 *
 * @param query Reference to buffer of type canton_query_t
 * @param signature Reference to signature to be sent to the host
 * @return true If the signature was sent successfully
 * @return false If the signature could not be sent - maybe due to and P0 event
 * or invalid request received from the host
 */
static bool send_signature(canton_query_t *query,
                           const canton_external_sig_t *sig);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static canton_txn_external_context_t *canton_txn_external_context = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool check_which_request(const canton_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn_external.which_request) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static void send_response(const pb_size_t which_response) {
  canton_result_t result =
      init_canton_result(CANTON_RESULT_SIGN_TXN_EXTERNAL_TAG);
  result.sign_txn_external.which_response = which_response;
  canton_send_result(&result);
}

static bool validate_request_data(
    const canton_sign_txn_external_request_t *request) {
  bool status = true;

  if (!canton_derivation_path_guard(request->initiate.derivation_path,
                                    request->initiate.derivation_path_count)) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  return status;
}

static bool handle_initiate_query(const canton_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  // TODO: Handle wallet search failures - eg: Wallet ID not found, Wallet
  // ID found but is invalid/locked wallet
  if (!check_which_request(query,
                           CANTON_SIGN_TXN_EXTERNAL_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn_external) ||
      !get_wallet_name_by_id(query->sign_txn_external.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             canton_send_error)) {
    return false;
  }

  (void)snprintf(msg,
                 sizeof(msg),
                 UI_TEXT_SIGN_TXN_EXTERNAL_PROMPT,
                 CANTON_NAME,
                 wallet_name);
  // Take user consent to sign transaction for the wallet
  if (!core_scroll_page(NULL, msg, canton_send_error)) {
    return false;
  }

  set_app_flow_status(CANTON_SIGN_TXN_EXTERNAL_STATUS_CONFIRM);
  memcpy(&canton_txn_external_context->init_info,
         &query->sign_txn_external.initiate,
         sizeof(canton_sign_txn_external_initiate_request_t));
  canton_txn_external_context->unsigned_txn_external_info.txn_type =
      query->sign_txn_external.initiate.txn_type;

  send_response(CANTON_SIGN_TXN_EXTERNAL_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool validate_and_store_namespace_delegation_proposal(
    const canton_generate_transaction_request_proposal_t *proposal,
    canton_unsigned_txn_external_info_t *ut_txn) {
  // topology txns are of type ADD_REPLACE only
  if (proposal->operation != CANTON_TOPOLOGY_CHANGE_OP_ADD_REPLACE ||
      !proposal->has_mapping ||
      proposal->mapping.which_mapping !=
          CANTON_TOPOLOGY_MAPPING_NAMESPACE_DELEGATION_TAG) {
    return false;
  }

  canton_namespace_delegation_t namespace_delegation =
      proposal->mapping.namespace_delegation;
  if (/*namespace_delegation.which_restriction !=
          CANTON_NAMESPACE_DELEGATION_CAN_SIGN_ALL_MAPPINGS_TAG ||*/
      !namespace_delegation.has_target_key) {
    return false;
  }

  // TODO: research about the first 12 bytes in the public key field
  uint8_t *public_key = namespace_delegation.target_key.public_key + 12;

  memcpy(ut_txn->display_info.namespace_delegation.target_public_key,
         public_key,
         CANTON_PUB_KEY_SIZE);

  memcpy(ut_txn->display_info.namespace_delegation.namespace,
         namespace_delegation.namespace,
         CANTON_FINGERPRINT_STR_SIZE);

  return true;
}

static bool parse_hash_and_validate_transaction_external(
    const uint8_t *serialized_txn,
    uint32_t serialized_txn_size,
    canton_unsigned_txn_external_info_t *ut_txn) {
  memzero(ut_txn->hash, CANTON_HASH_PREFIX_SIZE + SHA256_DIGEST_LENGTH);

  switch (ut_txn->txn_type) {
    case CANTON_SUPPORTED_TXN_TYPE_NAMESPACE_DELEGATION: {
      canton_generate_transaction_request_proposal_t decoded_proposal =
          CANTON_GENERATE_TRANSACTION_REQUEST_PROPOSAL_INIT_ZERO;
      if (!decode_canton_serialized_data(
              serialized_txn + 3,    // skip the length prefix
              serialized_txn_size - 3,
              CANTON_GENERATE_TRANSACTION_REQUEST_PROPOSAL_FIELDS,
              &decoded_proposal) ||
          !validate_and_store_namespace_delegation_proposal(&decoded_proposal,
                                                            ut_txn)) {
        return false;
      }
      sha256_canton(CANTON_TOPOLOGY_TXN_HASH_PURPOSE,
                    serialized_txn,
                    serialized_txn_size,
                    ut_txn->hash);
      break;
    }
    default: {
      return false;
    }
  }

  return true;
}

static bool fetch_valid_input(canton_query_t *query) {
  canton_unsigned_txn_external_info_t *ut_txn =
      &canton_txn_external_context->unsigned_txn_external_info;

  uint32_t transaction_size =
      query->sign_txn_external.initiate.transaction_size;
  uint8_t *transaction = (uint8_t *)malloc(transaction_size);
  if (transaction == NULL) {
    canton_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  uint32_t size = 0;
  canton_result_t response =
      init_canton_result(CANTON_RESULT_SIGN_TXN_EXTERNAL_TAG);
  const canton_sign_txn_external_data_t *txn_data =
      &query->sign_txn_external.txn_data;
  const common_chunk_payload_t *payload = &txn_data->chunk_payload;
  const common_chunk_payload_chunk_t *chunk = &txn_data->chunk_payload.chunk;

  while (1) {
    if (!canton_get_query(query, CANTON_QUERY_SIGN_TXN_EXTERNAL_TAG) ||
        !check_which_request(query,
                             CANTON_SIGN_TXN_EXTERNAL_REQUEST_TXN_DATA_TAG)) {
      free(transaction);
      return false;
    }

    if (!txn_data->has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        size + payload->chunk.size > transaction_size) {
      canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      free(transaction);
      return false;
    }

    memcpy(&transaction[size], chunk->bytes, chunk->size);
    size += chunk->size;
    // Send chunk ack to host
    response.sign_txn_external.which_response =
        CANTON_SIGN_TXN_EXTERNAL_RESPONSE_DATA_ACCEPTED_TAG;
    response.sign_txn_external.data_accepted.has_chunk_ack = true;
    response.sign_txn_external.data_accepted.chunk_ack.chunk_index =
        payload->chunk_index;
    canton_send_result(&response);

    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }

  // make sure all chunks were received
  if (size != transaction_size) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    free(transaction);
    return false;
  }

  if (!parse_hash_and_validate_transaction_external(
          transaction, transaction_size, ut_txn)) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    free(transaction);
    return false;
  }

  free(transaction);

  return true;
}

static bool get_user_verification(void) {
  canton_unsigned_txn_external_info_t *ut_txn_info =
      &canton_txn_external_context->unsigned_txn_external_info;

  canton_sign_txn_external_supported_txn_types_t txn_type =
      ut_txn_info->txn_type;

  switch (txn_type) {
    case CANTON_SUPPORTED_TXN_TYPE_NAMESPACE_DELEGATION: {
      if (!core_scroll_page(UI_TEXT_TRANSACTION_TYPE,
                            NAMESPACE_DELEGATION_TXN_TYPE_TEXT,
                            canton_send_error) ||
          !core_scroll_page(
              UI_TEXT_NAMESPACE,
              ut_txn_info->display_info.namespace_delegation.namespace,
              canton_send_error)) {
        return false;
      }
      break;
    }
    default: {
      return false;
    }
  }

  set_app_flow_status(CANTON_SIGN_TXN_EXTERNAL_STATUS_VERIFY);

  return true;
}

static bool sign_txn_external(canton_external_sig_t *sig) {
  canton_unsigned_txn_external_info_t *ut_txn_info =
      &canton_txn_external_context->unsigned_txn_external_info;
  uint8_t seed[64] = {0};
  if (!reconstruct_seed(canton_txn_external_context->init_info.wallet_id,
                        seed,
                        canton_send_error)) {
    memzero(seed, sizeof(seed));
    // TODO: handle errors of reconstruction flow
    return false;
  }

  set_app_flow_status(CANTON_SIGN_TXN_EXTERNAL_STATUS_SEED_GENERATED);

  HDNode hdnode = {0};
  derive_hdnode_from_path(
      canton_txn_external_context->init_info.derivation_path,
      canton_txn_external_context->init_info.derivation_path_count,
      ED25519_NAME,
      seed,
      &hdnode);

  if (ut_txn_info->txn_type == CANTON_SUPPORTED_TXN_TYPE_NAMESPACE_DELEGATION &&
      memcmp(hdnode.public_key + 1,
             ut_txn_info->display_info.namespace_delegation.target_public_key,
             CANTON_PUB_KEY_SIZE) != 0) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    memzero(seed, sizeof(seed));
    memzero(&hdnode, sizeof(hdnode));
    return false;
  }

  ed25519_sign(ut_txn_info->hash,
               CANTON_HASH_SIZE,
               hdnode.private_key,
               hdnode.public_key + 1,
               sig->signature);

  memzero(seed, sizeof(seed));
  memzero(&hdnode, sizeof(hdnode));

  return true;
}

static bool send_signature(canton_query_t *query,
                           const canton_external_sig_t *sig) {
  canton_result_t result =
      init_canton_result(CANTON_RESULT_SIGN_TXN_EXTERNAL_TAG);
  result.sign_txn_external.which_response =
      CANTON_SIGN_TXN_EXTERNAL_RESPONSE_SIGNATURE_TAG;

  if (!canton_get_query(query, CANTON_QUERY_SIGN_TXN_EXTERNAL_TAG) ||
      !check_which_request(query,
                           CANTON_SIGN_TXN_EXTERNAL_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(
      &result.sign_txn_external.signature, sig, sizeof(canton_external_sig_t));

  canton_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void canton_sign_txn_external(canton_query_t *query) {
  canton_txn_external_context = (canton_txn_external_context_t *)malloc(
      sizeof(canton_txn_external_context_t));
  memzero(canton_txn_external_context, sizeof(canton_txn_external_context_t));

  canton_external_sig_t sig = {0};

  if (handle_initiate_query(query) && fetch_valid_input(query) &&
      get_user_verification() && sign_txn_external(&sig) &&
      send_signature(query, &sig)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  if (canton_txn_external_context) {
    free(canton_txn_external_context);
    canton_txn_external_context = NULL;
  }

  return;
}
