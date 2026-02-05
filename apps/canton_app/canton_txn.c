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

#include <sha2.h>
#include <stdint.h>
#include <string.h>

#include "bip32.h"
#include "canton/core.pb.h"
#include "canton/sign_txn.pb.h"
#include "canton_api.h"
#include "canton_context.h"
#include "canton_helpers.h"
#include "canton_instruments.h"
#include "canton_priv.h"
#include "canton_txn_encoding.h"
#include "coin_utils.h"
#include "composable_app_queue.h"
#include "constant_texts.h"
#include "ed25519.h"
#include "exchange_main.h"
#include "reconstruct_wallet_flow.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "wallet_list.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/
canton_txn_context_t *canton_txn_context = NULL;
/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef canton_sign_txn_signature_response_t canton_sig_t;

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
 * @param request Reference to an instance of canton_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(const canton_sign_txn_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * CANTON_SIGN_TXN_REQUEST_INITIATE_TAG type of request. Additionally, the
 * wallet-id is validated for sanity and the derivation path for the account is
 * also validated. After the validations, user is prompted about the action for
 * confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the canton_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const canton_query_t *query);

/**
 * @brief Handles fetching of the metadata/top-level transaction elements
 * @details The function waits on USB event then decoding and validation of the
 * received query. Post validation, based on the values in the query, the
 * function allocates memory for storing node_seeds & node hashes in
 * canton_txn_context. Also, the data received in the query is duplicated into
 * canton_txn_context.
 *
 * @param query Reference to storage for decoding query from host
 *
 * @return bool Indicating if the function actions succeeded or failed
 */
static bool fetch_transaction_meta(canton_query_t *query);

/**
 * @brief Fetches each transaction node seed
 * @details The function will try to fetch and consequently verify each node
 * seed by referring to the declared node seed count in canton_txn_context . The
 * function will duplicate each node seed information into canton_txn_context.
 *
 * @param query Reference to an instance of canton_query_t for storing the
 * transient node seeds.
 *
 * @return bool Indicating if all the node seeds are received and verified
 * @retval true If all the node seeds are fetched and verified
 * @retval flase If any of the node seeds failed verification or weren't fetched
 */
static bool fetch_valid_txn_node_seed(canton_query_t *query);

/**
 * @brief Fetches each transaction node
 * @details The function will try to fetch and consequently decode and verify
 * each node by referring to the declared node count in canton_txn_context . The
 * function will validate, encode and hash each node and store the hash in
 * canton_txn_context.
 *
 * @param query Reference to an instance of canton_query_t for storing the
 * transient nodes.
 *
 * @return bool Indicating if all the nodes are received, decoded, verified and
 * hashed
 * @retval true If all the nodes are fetched, decoded, verified and hashed
 * @retval flase If any of the nodes failed verification, decoding or weren't
 * fetched
 */
static bool fetch_and_encode_valid_txn_node(canton_query_t *query);

/**
 * @brief Handles fetching of the canton metadata
 * @details The function waits on USB event then decoding and validation of the
 * received query. Post validation, based on the values in the query, the
 * function allocates memory for storing input contracts hashes in
 * canton_txn_context. Also, the data received in the query is duplicated into
 * canton_txn_context.
 *
 * @param query Reference to storage for decoding query from host
 *
 * @return bool Indicating if the function actions succeeded or failed
 */
static bool fetch_canton_meta(canton_query_t *query);

/**
 * @brief Fetches each transaction input contract
 * @details The function will try to fetch and consequently decode and verify
 * each input contract by referring to the declared input contract count in
 * canton_txn_context . The function will validate, encode and hash each input
 * contract and store the hash in canton_txn_context.
 *
 * @param query Reference to an instance of canton_query_t for storing the
 * transient input contracts.
 *
 * @return bool Indicating if all the input contracts are received, decoded,
 * verified and hashed
 * @retval true If all the input contracts are fetched, decoded, verified and
 * hashed
 * @retval flase If any of the input contracts failed verification, decoding or
 * weren't fetched
 */
static bool fetch_and_encode_valid_meta_input_contract(canton_query_t *query);

/**
 * @brief Receives all the unsigned txn data from the host.
 * @note In case of any failure, a corresponding message is conveyed to the host
 *
 * @param query Reference to buffer of type canton_query_t
 * @return true If all the txn data is received and all the data is valid
 * @return false If any of the txn data could not be received or it's validation
 * failed
 */
static bool fetch_and_encode_valid_unsigned_txn_data(canton_query_t *query);

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
static bool sign_txn(canton_sig_t *der_signature);

/**
 * @brief Sends signature of the CANTON unsigned txn to the host
 * @details The function waits for the host to send a request of type
 * CANTON_SIGN_TXN_REQUEST_SIGNATURE_TAG and sends the response
 *
 * @param query Reference to buffer of type canton_query_t
 * @param signature Reference to signature to be sent to the host
 * @return true If the signature was sent successfully
 * @return false If the signature could not be sent - maybe due to and P0 event
 * or invalid request received from the host
 */
static bool send_signature(canton_query_t *query,
                           const canton_sig_t *der_signature);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static bool use_signature_verification = false;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool check_which_request(const canton_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static void send_response(const pb_size_t which_response) {
  canton_result_t result = init_canton_result(CANTON_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  canton_send_result(&result);
}

static bool validate_request_data(const canton_sign_txn_request_t *request) {
  bool status = true;

  if (!canton_derivation_path_guard(request->initiate.derivation_path,
                                    request->initiate.derivation_path_count)) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
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

static bool handle_initiate_query(const canton_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  // TODO: Handle wallet search failures - eg: Wallet ID not found, Wallet
  // ID found but is invalid/locked wallet
  if (!check_which_request(query, CANTON_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             canton_send_error)) {
    return false;
  }

  snprintf(msg, sizeof(msg), UI_TEXT_SIGN_TXN_PROMPT, CANTON_NAME, wallet_name);
  // Take user consent to sign transaction for the wallet
  if (!core_confirmation(msg, canton_send_error)) {
    return false;
  }

  set_app_flow_status(CANTON_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&canton_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(canton_sign_txn_initiate_request_t));

  send_response(CANTON_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool fetch_transaction_meta(canton_query_t *query) {
  if (!canton_get_query(query, CANTON_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, CANTON_SIGN_TXN_REQUEST_TXN_META_TAG)) {
    return false;
  }

  memcpy(&canton_txn_context->unsigned_txn.txn_meta,
         &query->sign_txn.txn_meta,
         sizeof(canton_sign_txn_transaction_metadata_t));

  uint32_t node_seeds_count = query->sign_txn.txn_meta.node_seeds_count;
  uint32_t nodes_count = query->sign_txn.txn_meta.nodes_count;

  // Allowing maximum 20 nodes in a transaction
  if (nodes_count > 20) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  // we now know the number of node seeds and nodes
  // allocate memory for node seeds and node hashes in canton_txn_context
  canton_txn_context->unsigned_txn.txn_node_seeds =
      (canton_node_seed_t *)malloc(sizeof(canton_node_seed_t) *
                                   node_seeds_count);
  canton_txn_context->unsigned_txn.txn_node_hashes =
      (canton_txn_node_hash_t *)malloc(sizeof(canton_txn_node_hash_t) *
                                       nodes_count);

  if (canton_txn_context->unsigned_txn.txn_node_seeds == NULL ||
      canton_txn_context->unsigned_txn.txn_node_hashes == NULL) {
    canton_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }
  send_response(CANTON_SIGN_TXN_RESPONSE_TXN_META_ACCEPTED_TAG);
  return true;
}

static bool fetch_valid_txn_node_seed(canton_query_t *query) {
  for (int idx = 0;
       idx < canton_txn_context->unsigned_txn.txn_meta.node_seeds_count;
       idx++) {
    if (!canton_get_query(query, CANTON_QUERY_SIGN_TXN_TAG) ||
        !check_which_request(query,
                             CANTON_SIGN_TXN_REQUEST_TXN_NODE_SEED_TAG)) {
      return false;
    }

    if (!query->sign_txn.txn_node_seed.has_node_seed) {
      canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    memcpy(&canton_txn_context->unsigned_txn.txn_node_seeds[idx],
           &query->sign_txn.txn_node_seed.node_seed,
           sizeof(canton_node_seed_t));

    send_response(CANTON_SIGN_TXN_RESPONSE_TXN_NODE_SEED_ACCEPTED_TAG);
  }

  return true;
}

static bool fetch_and_encode_valid_txn_node(canton_query_t *query) {
  for (int idx = 0; idx < canton_txn_context->unsigned_txn.txn_meta.nodes_count;
       idx++) {
    if (!canton_get_query(query, CANTON_QUERY_SIGN_TXN_TAG) ||
        !check_which_request(query,
                             CANTON_SIGN_TXN_REQUEST_TXN_NODE_META_TAG)) {
      return false;
    }
    uint32_t txn_node_total_size =
        query->sign_txn.txn_node_meta.serialized_data_size;

    if (txn_node_total_size == 0) {
      canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    send_response(CANTON_SIGN_TXN_RESPONSE_TXN_NODE_META_ACCEPTED_TAG);

    uint8_t *txn_serialized_node = (uint8_t *)malloc(txn_node_total_size);
    if (txn_serialized_node == NULL) {
      canton_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    uint32_t size = 0;
    canton_result_t response = init_canton_result(CANTON_RESULT_SIGN_TXN_TAG);
    const canton_sign_txn_serialized_data_t *txn_node_data =
        &query->sign_txn.txn_node;
    const common_chunk_payload_t *payload = &txn_node_data->chunk_payload;
    const common_chunk_payload_chunk_t *chunk =
        &txn_node_data->chunk_payload.chunk;

    while (1) {
      if (!canton_get_query(query, CANTON_QUERY_SIGN_TXN_TAG) ||
          !check_which_request(query, CANTON_SIGN_TXN_REQUEST_TXN_NODE_TAG)) {
        free(txn_serialized_node);
        return false;
      }

      if (!txn_node_data->has_chunk_payload ||
          payload->chunk_index >= payload->total_chunks ||
          size + payload->chunk.size > txn_node_total_size) {
        canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                          ERROR_DATA_FLOW_INVALID_DATA);
        free(txn_serialized_node);
        return false;
      }

      memcpy(&txn_serialized_node[size], chunk->bytes, chunk->size);
      size += chunk->size;
      // Send chunk ack to host
      response.sign_txn.which_response =
          CANTON_SIGN_TXN_RESPONSE_TXN_NODE_ACCEPTED_TAG;
      response.sign_txn.txn_node_accepted.has_chunk_ack = true;
      response.sign_txn.txn_node_accepted.chunk_ack.chunk_index =
          payload->chunk_index;
      canton_send_result(&response);

      if (0 == payload->remaining_size ||
          payload->chunk_index + 1 == payload->total_chunks) {
        break;
      }
    }

    // make sure all chunks were received
    if (size != txn_node_total_size) {
      canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      free(txn_serialized_node);
      return false;
    }

    if (!parse_and_hash_canton_txn_node(
            txn_serialized_node,
            txn_node_total_size,
            &canton_txn_context->unsigned_txn.txn_node_hashes[idx])) {
      canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      free(txn_serialized_node);
      return false;
    }

    free(txn_serialized_node);
  }

  return true;
}

static bool fetch_canton_meta(canton_query_t *query) {
  if (!canton_get_query(query, CANTON_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, CANTON_SIGN_TXN_REQUEST_CANTON_META_TAG)) {
    return false;
  }

  memcpy(&canton_txn_context->unsigned_txn.canton_meta,
         &query->sign_txn.canton_meta,
         sizeof(canton_sign_txn_canton_metadata_t));

  uint32_t input_contracts_count =
      canton_txn_context->unsigned_txn.canton_meta.input_contracts_count;

  // Allowing maximum 20 input contracts in a transaction
  if (input_contracts_count > 20) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  // we now know the number of input contracts
  // allocate memory for input contracts in
  // canton_txn_context
  canton_txn_context->unsigned_txn.input_contract_hashes =
      (canton_txn_input_contract_hash_t *)malloc(
          sizeof(canton_txn_input_contract_hash_t) * input_contracts_count);

  if (canton_txn_context->unsigned_txn.input_contract_hashes == NULL) {
    canton_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }
  send_response(CANTON_SIGN_TXN_RESPONSE_CANTON_META_ACCEPTED_TAG);
  return true;
}

static bool fetch_and_encode_valid_meta_input_contract(canton_query_t *query) {
  for (int idx = 0;
       idx < canton_txn_context->unsigned_txn.canton_meta.input_contracts_count;
       idx++) {
    if (!canton_get_query(query, CANTON_QUERY_SIGN_TXN_TAG) ||
        !check_which_request(
            query, CANTON_SIGN_TXN_REQUEST_META_INPUT_CONTRACT_META_TAG)) {
      return false;
    }
    uint32_t input_contract_total_size =
        query->sign_txn.meta_input_contract_meta.serialized_data_size;

    if (input_contract_total_size == 0) {
      canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    send_response(
        CANTON_SIGN_TXN_RESPONSE_META_INPUT_CONTRACT_META_ACCEPTED_TAG);

    uint8_t *txn_serialized_input_contract =
        (uint8_t *)malloc(input_contract_total_size);
    if (txn_serialized_input_contract == NULL) {
      canton_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    uint32_t size = 0;
    canton_result_t response = init_canton_result(CANTON_RESULT_SIGN_TXN_TAG);
    const canton_sign_txn_serialized_data_t *txn_input_contract_data =
        &query->sign_txn.meta_input_contract;
    const common_chunk_payload_t *payload =
        &txn_input_contract_data->chunk_payload;
    const common_chunk_payload_chunk_t *chunk =
        &txn_input_contract_data->chunk_payload.chunk;

    while (1) {
      if (!canton_get_query(query, CANTON_QUERY_SIGN_TXN_TAG) ||
          !check_which_request(
              query, CANTON_SIGN_TXN_REQUEST_META_INPUT_CONTRACT_TAG)) {
        free(txn_serialized_input_contract);
        return false;
      }

      if (!txn_input_contract_data->has_chunk_payload ||
          payload->chunk_index >= payload->total_chunks ||
          size + payload->chunk.size > input_contract_total_size) {
        canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                          ERROR_DATA_FLOW_INVALID_DATA);
        free(txn_serialized_input_contract);
        return false;
      }

      memcpy(&txn_serialized_input_contract[size], chunk->bytes, chunk->size);
      size += chunk->size;
      // Send chunk ack to host
      response.sign_txn.which_response =
          CANTON_SIGN_TXN_RESPONSE_META_INPUT_CONTRACT_ACCEPTED_TAG;
      response.sign_txn.meta_input_contract_accepted.has_chunk_ack = true;
      response.sign_txn.meta_input_contract_accepted.chunk_ack.chunk_index =
          payload->chunk_index;
      canton_send_result(&response);

      if (0 == payload->remaining_size ||
          payload->chunk_index + 1 == payload->total_chunks) {
        break;
      }
    }

    // make sure all chunks were received
    if (size != input_contract_total_size) {
      canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      free(txn_serialized_input_contract);
      return false;
    }

    if (!parse_and_hash_canton_metadata_input_contract(
            txn_serialized_input_contract,
            input_contract_total_size,
            &canton_txn_context->unsigned_txn.input_contract_hashes[idx])) {
      canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      free(txn_serialized_input_contract);
      return false;
    }

    free(txn_serialized_input_contract);
  }
  return true;
}

static bool fetch_and_encode_valid_unsigned_txn_data(canton_query_t *query) {
  return fetch_transaction_meta(query) && fetch_valid_txn_node_seed(query) &&
         fetch_and_encode_valid_txn_node(query) && fetch_canton_meta(query) &&
         fetch_and_encode_valid_meta_input_contract(query) &&
         validate_and_encode_canton_unsigned_txn();
}

static void get_expiry_display(uint64_t expiry_time,
                               uint64_t start_time,
                               char *expiry_display) {
  uint64_t diff = expiry_time - start_time;

  uint64_t days = 0;
  uint64_t hours = 0;
  uint64_t mins = 0;
  uint64_t days_factor = ((uint64_t)24 * 60 * 60 * 1000 * 1000);
  uint64_t hours_factor = ((uint64_t)60 * 60 * 1000 * 1000);
  uint64_t mins_factor = ((uint64_t)60 * 1000 * 1000);

  days = diff / days_factor;
  diff %= days_factor;
  hours = diff / hours_factor;
  diff %= hours_factor;
  mins = diff / mins_factor;

  if (days > 0) {
    char display[30] = {'\0'};
    snprintf(display, sizeof(display), UI_TEXT_DAYS, days);
    strcat(expiry_display, display);
  }
  if (hours > 0) {
    char display[30] = {'\0'};
    snprintf(display, sizeof(display), UI_TEXT_HOURS, hours);
    strcat(expiry_display, display);
  }
  if (mins > 0) {
    char display[30] = {'\0'};
    snprintf(display, sizeof(display), UI_TEXT_MINS, mins);
    strcat(expiry_display, display);
  }
}

static bool is_instrument_whitelisted(
    canton_instrument_t *input_instrument,
    canton_instrument_data_t *output_instrument_data) {
  const canton_instrument_data_t *match = NULL;
  bool status = false;

  for (int16_t i = 0; i < CANTON_WHITELISTED_INSTRUMENT_COUNT; i++) {
    if (strcmp(input_instrument->id, canton_instrument_data[i].instrument.id) ==
            0 &&
        strcmp(input_instrument->admin,
               canton_instrument_data[i].instrument.admin) == 0) {
      match = &canton_instrument_data[i];
      status = true;
      break;
    }
  }

  if (NULL != output_instrument_data && NULL != match) {
    memcpy(output_instrument_data, match, sizeof(*match));
  }

  // return unknown empty instrument data if not found in the whitelist
  if (!status) {
    canton_instrument_data_t empty_instrument_data = {
        .instrument = {.id = "", .admin = ""},
        .symbol = "",
        .decimal = 0,
    };
    memcpy(output_instrument_data,
           &empty_instrument_data,
           sizeof(empty_instrument_data));
  }

  return status;
}

static bool get_user_verification(void) {
  canton_txn_display_info_t *display_info =
      &canton_txn_context->unsigned_txn.txn_display_info;

  char *sender_party_id = display_info->sender_party_id;
  char *receiver_party_id = display_info->receiver_party_id;
  char *amount_string = display_info->amount;
  canton_transaction_type_t txn_type = display_info->txn_type;

  if (use_signature_verification) {
    if (!exchange_validate_stored_signature(receiver_party_id,
                                            sizeof(receiver_party_id))) {
      return false;
    }
  }

  canton_instrument_data_t instrument_data = {0};
  if (txn_type != CANTON_TXN_TYPE_PREAPPROVAL &&
      txn_type != CANTON_TXN_TYPE_MERGE_DELEGATION_PROPOSAL) {
    if (!is_instrument_whitelisted(&display_info->instrument,
                                   &instrument_data)) {
      // Instrument Unverifed, Display warning
      delay_scr_init(ui_text_unverified_token, DELAY_TIME);

      // verify instrument id and admin
      if (!core_scroll_page(UI_TEXT_VERIFY_INSTRUMENT_ID,
                            display_info->instrument.id,
                            canton_send_error) ||
          !core_scroll_page(UI_TEXT_VERIFY_INSTRUMENT_ADMIN,
                            display_info->instrument.admin,
                            canton_send_error)) {
        return false;
      }

    } else if (strcmp(display_info->instrument.id, "Amulet") !=
               0) {    // Only show confirmation for instruments other than
                       // Amulet(Coin)
      char msg[100] = "";
      snprintf(msg,
               sizeof(msg),
               UI_TEXT_SIGN_TOKEN_TXN_PROMPT,
               instrument_data.symbol,
               CANTON_NAME);
      if (!core_confirmation(msg, canton_send_error)) {
        return false;
      }
    }
  }

  // verify transaction type
  char txn_type_text[50] = {'\0'};
  switch (txn_type) {
    case CANTON_TXN_TYPE_TAP: {
      strcpy(txn_type_text, TAP_TXN_TYPE_TEXT);
      break;
    }
    case CANTON_TXN_TYPE_TRANSFER: {
      strcpy(txn_type_text, TRANSFER_TXN_TYPE_TEXT);
      break;
    }
    case CANTON_TXN_TYPE_WITHDRAW: {
      strcpy(txn_type_text, WITHDRAW_TXN_TYPE_TEXT);
      break;
    }
    case CANTON_TXN_TYPE_ACCEPT: {
      strcpy(txn_type_text, ACCEPT_TXN_TYPE_TEXT);
      break;
    }
    case CANTON_TXN_TYPE_REJECT: {
      strcpy(txn_type_text, REJECT_TXN_TYPE_TEXT);
      break;
    }
    case CANTON_TXN_TYPE_PREAPPROVAL: {
      strcpy(txn_type_text, PREAPPROVAL_TXN_TYPE_TEXT);
      break;
    }
    case CANTON_TXN_TYPE_MERGE_DELEGATION_PROPOSAL: {
      strcpy(txn_type_text, MERGE_DELEGATION_PROPOSAL_TXN_TYPE_TEXT);
      break;
    }
    default: {
      strcpy(txn_type_text, "Unknown");
      break;
    }
  }

  if (!core_scroll_page(
          UI_TEXT_TRANSACTION_TYPE, txn_type_text, canton_send_error)) {
    return false;
  }

  if (txn_type == CANTON_TXN_TYPE_PREAPPROVAL ||
      txn_type == CANTON_TXN_TYPE_MERGE_DELEGATION_PROPOSAL) {
    return true;
  }

  // verify sender
  if (txn_type != CANTON_TXN_TYPE_TAP &&
      txn_type != CANTON_TXN_TYPE_PREAPPROVAL &&
      txn_type != CANTON_TXN_TYPE_MERGE_DELEGATION_PROPOSAL) {
    if (!core_scroll_page(
            UI_TEXT_SENDER_PARTY_ID, sender_party_id, canton_send_error)) {
      return false;
    }
  }

  // verify receiver
  if (!core_scroll_page(
          UI_TEXT_RECEIVER_PARTY_ID, receiver_party_id, canton_send_error)) {
    return false;
  }

  if (txn_type != CANTON_TXN_TYPE_PREAPPROVAL &&
      txn_type != CANTON_TXN_TYPE_MERGE_DELEGATION_PROPOSAL) {
    // verify recipient amount
    char display[100] = {'\0'};
    snprintf(display,
             sizeof(display),
             UI_TEXT_VERIFY_AMOUNT,
             amount_string,
             instrument_data.symbol);

    if (!core_confirmation(display, canton_send_error)) {
      return false;
    }
  }

  // verify expiry
  if (txn_type == CANTON_TXN_TYPE_TRANSFER && display_info->start_time != 0 &&
      display_info->expiry_time != 0) {
    char expiry_display[100] = {'\0'};
    get_expiry_display(
        display_info->expiry_time, display_info->start_time, expiry_display);

    if (!core_scroll_page(UI_TEXT_EXPIRY, expiry_display, canton_send_error)) {
      return false;
    }
  }

  set_app_flow_status(CANTON_SIGN_TXN_STATUS_VERIFY);

  return true;
}

static bool sign_txn(canton_sig_t *sig) {
  uint8_t seed[64] = {0};
  if (!reconstruct_seed(
          canton_txn_context->init_info.wallet_id, seed, canton_send_error)) {
    memzero(seed, sizeof(seed));
    // TODO: handle errors of reconstruction flow
    return false;
  }

  set_app_flow_status(CANTON_SIGN_TXN_STATUS_SEED_GENERATED);

  uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw(canton_txn_context->encoded_txn, ENCODED_TXN_LENGTH, digest);

  HDNode hdnode = {0};
  derive_hdnode_from_path(canton_txn_context->init_info.derivation_path,
                          canton_txn_context->init_info.derivation_path_count,
                          ED25519_NAME,
                          seed,
                          &hdnode);

  // match partyId(derivied from public key) with the partyId in the transfer
  // pre-approval transaction
  canton_txn_display_info_t *display_info =
      &canton_txn_context->unsigned_txn.txn_display_info;
  if (display_info->txn_type == CANTON_TXN_TYPE_PREAPPROVAL ||
      display_info->txn_type == CANTON_TXN_TYPE_MERGE_DELEGATION_PROPOSAL) {
    if (!verify_party_id(hdnode.public_key + 1,
                         display_info->txn_type == CANTON_TXN_TYPE_PREAPPROVAL
                             ? display_info->receiver_party_id
                             : display_info->owner_party_id)) {
      canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      memzero(digest, sizeof(digest));
      memzero(seed, sizeof(seed));
      memzero(&hdnode, sizeof(hdnode));
      return false;
    }
  }

  ed25519_sign(digest,
               SHA256_DIGEST_LENGTH,
               hdnode.private_key,
               hdnode.public_key + 1,
               sig->signature);

  memzero(digest, sizeof(digest));
  memzero(seed, sizeof(seed));
  memzero(&hdnode, sizeof(hdnode));

  return true;
}

static bool send_signature(canton_query_t *query, const canton_sig_t *sig) {
  canton_result_t result = init_canton_result(CANTON_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = CANTON_SIGN_TXN_RESPONSE_SIGNATURE_TAG;

  if (!canton_get_query(query, CANTON_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, CANTON_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(&result.sign_txn.signature, sig, sizeof(canton_sig_t));

  canton_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void canton_sign_transaction(canton_query_t *query) {
  canton_txn_context =
      (canton_txn_context_t *)malloc(sizeof(canton_txn_context_t));
  memzero(canton_txn_context, sizeof(canton_txn_context_t));
  canton_txn_context->unsigned_txn.txn_display_info.txn_type =
      CANTON_TXN_TYPE_UNRECOGNIZED;
  canton_txn_context->unsigned_txn.input_contract_hashes = NULL;
  canton_txn_context->unsigned_txn.txn_node_seeds = NULL;
  canton_txn_context->unsigned_txn.txn_node_hashes = NULL;

  canton_sig_t sig = {0};

  if (handle_initiate_query(query) &&
      fetch_and_encode_valid_unsigned_txn_data(query) &&
      get_user_verification() && sign_txn(&sig) &&
      send_signature(query, &sig)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  if (canton_txn_context) {
    if (canton_txn_context->unsigned_txn.txn_node_seeds) {
      free(canton_txn_context->unsigned_txn.txn_node_seeds);
    }
    if (canton_txn_context->unsigned_txn.txn_node_hashes) {
      free(canton_txn_context->unsigned_txn.txn_node_hashes);
    }
    if (canton_txn_context->unsigned_txn.input_contract_hashes) {
      free(canton_txn_context->unsigned_txn.input_contract_hashes);
    }
    free(canton_txn_context);
    canton_txn_context = NULL;
  }

  return;
}
