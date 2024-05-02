/**
 * @file    bittensor_sign_txn.c
 * @author  Cypherock X1 Team
 * @brief   Bittensor chain transaction signing logic
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

#include "../../vendor/cScale/src/scale.h"
#include "bittensor_api.h"
#include "bittensor_helpers.h"
#include "bittensor_priv.h"
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
 * @param query Reference to an instance of bittensor_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const bittensor_query_t *query,
                                pb_size_t which_request);

/**
 * @brief Validates the derivation path received in the request from host
 * @details The function validates the provided account derivation path in the
 * request. If invalid path is detected, the function will send an error to the
 * host and return false.
 *
 * @param request Reference to an instance of bittensor_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(const bittensor_sign_txn_request_t *request);

/**
 * @brief The function prepares and sends empty responses
 *
 * @param which_response Constant value for the response type to be sent
 */
static void send_response(pb_size_t which_response);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * BITTENSOR_SIGN_TXN type of request. Additionally, the wallet-id is validated
 * for sanity and the derivation path for the account is also validated. After
 * the validations, user is prompted about the action for confirmation. The
 * function returns true indicating all the validation and user confirmation was
 * a success. The function also duplicates the data from query into the
 * bittensor_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool bittensor_handle_initiate_query(const bittensor_query_t *query);

/**
 * @brief Fetches complete raw transaction to be signed for verification
 * @details The function will try to fetch the transaction by referring to the
 * declared size in bittensor_txn_context. The function will store complete
 * transaction into bittensor_txn_context.transaction.
 *
 * @param query Reference to an instance of bittensor_query_t for storing the
 * transient transaction chunks.
 *
 * @return bool Indicating if the whole transaction received and verified
 * @retval true If all the transaction was fetched and verified
 * @retval false If the transaction failed verification or wasn't fetched
 */
static bool bittensor_fetch_valid_transaction(bittensor_query_t *query);

/**
 * @brief Aggregates user consent for the transaction info
 * @details The function decodes the receiver address along with the
 * corresponding transfer value in SOL.
 *
 *
 * @return bool Indicating if the user confirmed the transaction
 * @retval true If user confirmed the receiver address along with
 * the corresponding value.
 * @retval false Immediate return if any of the confirmation is disapproved
 */
static bool bittensor_get_user_verification();

/**
 * @brief Fetches seed for generating public and private key
 * @details The function internally calls wallet reconstruction sub-flow to get
 * access to the seed.
 *
 * @param query Reference to an instance of bittensor_query_t to store transient
 * request from the host
 * @param seed_out seed recontructed from wallet
 * @return bool Indicating if signature for the provided transaction was
 * successfully generated
 * @retval true If all the seed is generated without any error
 * @retval false If seed failed to generate
 */
static bool fetch_seed(bittensor_query_t *query, uint8_t *seed_out);

/**
 * @brief Sends the generated signature to the host
 * @details The function internally updates the unsigned transaction with a
 * recent blockhash and signs the transaction before sending to the host
 *
 * @param query Reference to an instance of bittensor_query_t to store transient
 * request from the host
 * @param seed seed reconstructed from wallet
 * @param sig Reference to the struct having the signature components
 * @return bool Indicating if the signature is sent to the host
 * @retval true If the signature was sent to host successfully
 * @retval false If the host responded with unknown/wrong query
 */
static bool send_signature(bittensor_query_t *query,
                           uint8_t *seed,
                           bittensor_sign_txn_signature_response_t *sig);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

static bittensor_txn_context_t *bittensor_txn_context = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const bittensor_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    bittensor_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request_data(const bittensor_sign_txn_request_t *request) {
  bool status = true;

  if (!bittensor_derivation_path_guard(
          request->initiate.derivation_path,
          request->initiate.derivation_path_count)) {
    bittensor_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  return status;
}

static void send_response(const pb_size_t which_response) {
  bittensor_result_t result =
      init_bittensor_result(BITTENSOR_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  bittensor_send_result(&result);
}

static bool bittensor_handle_initiate_query(const bittensor_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";
  if (!check_which_request(query, BITTENSOR_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             bittensor_send_error)) {
    return false;
  }
  printf("%s:%d", __func__, __LINE__);
  snprintf(msg,
           sizeof(msg),
           UI_TEXT_SEND_TOKEN_PROMPT,
           BITTENSOR_LUNIT,
           BITTENSOR_NAME,
           wallet_name);
  // Take user consent to sign the transaction for the wallet
  if (!core_confirmation(msg, bittensor_send_error)) {
    return false;
  }

  set_app_flow_status(BITTENSOR_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&bittensor_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(bittensor_sign_txn_initiate_request_t));
  send_response(BITTENSOR_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  printf("%s:%d", __func__, __LINE__);
  return true;
}

static bool bittensor_fetch_valid_transaction(bittensor_query_t *query) {
  uint32_t size = 0;
  bittensor_result_t response =
      init_bittensor_result(BITTENSOR_RESULT_SIGN_TXN_TAG);
  uint32_t total_size = bittensor_txn_context->init_info.transaction_size;
  const bittensor_sign_txn_data_t *txn_data = &query->sign_txn.txn_data;
  const common_chunk_payload_t *payload = &txn_data->chunk_payload;
  const common_chunk_payload_chunk_t *chunk = &txn_data->chunk_payload.chunk;

  // allocate memory for storing transaction
  bittensor_txn_context->transaction = (uint8_t *)malloc(total_size);
  while (1) {
    if (!bittensor_get_query(query, BITTENSOR_QUERY_SIGN_TXN_TAG) &&
        !check_which_request(query, BITTENSOR_SIGN_TXN_REQUEST_TXN_DATA_TAG)) {
      return false;
    }

    if (!txn_data->has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        size + payload->chunk.size > total_size) {
      bittensor_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    memcpy(
        &bittensor_txn_context->transaction[size], chunk->bytes, chunk->size);
    size += chunk->size;
    // Send chunk ack to host
    response.sign_txn.which_response =
        BITTENSOR_SIGN_TXN_RESPONSE_DATA_ACCEPTED_TAG;
    response.sign_txn.data_accepted.has_chunk_ack = true;
    response.sign_txn.data_accepted.chunk_ack.chunk_index =
        payload->chunk_index;
    bittensor_send_result(&response);

    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }

  // make sure all chunks were received
  if (size != total_size) {
    bittensor_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }
  printf("%s:%d", __func__, __LINE__);
  // decode and verify the received transaction
  // if (SOL_OK != bittensor_byte_array_to_unsigned_txn(
  //                   bittensor_txn_context->transaction,
  //                   total_size,
  //                   &bittensor_txn_context->transaction_info) ||
  //     SOL_OK != bittensor_validate_unsigned_txn(
  //                   &bittensor_txn_context->transaction_info)) {
  //   return false;
  // }

  char buffer[300] = {0};

  memzero(buffer, 300);
  byte_array_to_hex_string(
      &query->sign_txn.txn_data, 114, buffer, sizeof(buffer));
  printf("\n%s:%d buffer: %s", __func__, __LINE__, buffer);

  return true;
}

double hexArrayToDouble(const uint8_t *hexArray, size_t length) {
  double result = 0.0;
  for (size_t i = 0; i < length; i++) {
    result *= 256.0;          // Shift existing bits by one byte
    result += hexArray[i];    // Add current byte value
  }
  return result;
}

static bool bittensor_get_user_verification() {
  char address[100] = "";
  size_t address_size = sizeof(address);

  uint8_t recipient_public_key[35] = {0};
  memcpy(recipient_public_key, &bittensor_txn_context->transaction[3], 32);

  char buffer[100] = {0};
  byte_array_to_hex_string(recipient_public_key, 32, buffer, sizeof(buffer));
  printf("\n%s:%d buffer: %s", __func__, __LINE__, buffer);

  // verify recipient address;
  if (!ss58enc(address,
               &address_size,
               42,    // for substrate
               recipient_public_key)) {
    bittensor_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  memzero(buffer, 100);
  byte_array_to_hex_string(
      address, BITTENSOR_ACCOUNT_ADDRESS_LENGTH, buffer, sizeof(buffer));
  printf("\n%s:%d buffer: %s", __func__, __LINE__, buffer);

  if (!core_scroll_page(
          ui_text_verify_address, address, bittensor_send_error)) {
    return false;
  }

  size_t amount_len = bittensor_txn_context->init_info.transaction_size - 111;
  printf("\n%s:%d total_size: %d",
         __func__,
         __LINE__,
         bittensor_txn_context->init_info.transaction_size);
  printf("\n%s:%d amount_len: %d", __func__, __LINE__, amount_len);

  uint8_t amount[10] = {0};
  memcpy(amount, &bittensor_txn_context->transaction[3 + 32], amount_len);

  // scale_compact_int compact = SCALE_COMPACT_INT_INIT;
  // memcpy(compact.data, amount, amount_len);
  // compact.mode = amount_len;

  // char *compact_hex = decode_compact_to_hex(&compact);
  // double decimalValue = strtoull(compact_hex, NULL, 16);
  // printf("%s = %u\n", compact_hex, decimalValue);
  // free(compact_hex);

  double decimalValue = hexArrayToDouble(amount, amount_len);
  printf("\n%s:%d decimalValue: %d", __func__, __LINE__, decimalValue);
  decimalValue *= 1e-12;
  printf("\n%s:%d decimalValue: %d", __func__, __LINE__, decimalValue);
  static char amount_str[20];
  sprintf(amount_str, "%.12f", decimalValue);

  byte_array_to_hex_string(amount_str, 20, buffer, sizeof(buffer));
  printf("\n%s:%d amount_str: %s", __func__, __LINE__, buffer);

  char display[100] = "";
  snprintf(display,
           sizeof(display),
           UI_TEXT_VERIFY_AMOUNT,
           amount_str,
           BITTENSOR_LUNIT);
  if (!core_confirmation(display, bittensor_send_error)) {
    return false;
  }

  set_app_flow_status(BITTENSOR_SIGN_TXN_STATUS_VERIFY);
  return true;
}

static bool fetch_seed(bittensor_query_t *query, uint8_t *seed_out) {
  if (!bittensor_get_query(query, BITTENSOR_QUERY_SIGN_TXN_TAG) &&
      !check_which_request(query, BITTENSOR_SIGN_TXN_REQUEST_VERIFY_TAG)) {
    return false;
  }

  if (!reconstruct_seed(bittensor_txn_context->init_info.wallet_id,
                        seed_out,
                        bittensor_send_error)) {
    memzero(seed_out, sizeof(seed_out));
    return false;
  }

  char buffer[100] = {0};
  byte_array_to_hex_string(seed_out, 32, buffer, sizeof(buffer));
  printf("\n%s:%d buffer: %s", __func__, __LINE__, buffer);

  set_app_flow_status(BITTENSOR_SIGN_TXN_STATUS_SEED_GENERATED);
  send_response(BITTENSOR_SIGN_TXN_RESPONSE_VERIFY_TAG);
  return true;
}

static bool send_signature(bittensor_query_t *query,
                           uint8_t *seed,
                           bittensor_sign_txn_signature_response_t *sig) {
  // HDNode hdnode = {0};
  // const size_t depth =
  // bittensor_txn_context->init_info.derivation_path_count; const uint32_t
  // *hd_path = bittensor_txn_context->init_info.derivation_path;
  bittensor_result_t result =
      init_bittensor_result(BITTENSOR_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = BITTENSOR_SIGN_TXN_RESPONSE_SIGNATURE_TAG;
  if (!bittensor_get_query(query, BITTENSOR_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, BITTENSOR_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }
  // recieve latest blockhash
  // uint8_t bittensor_latest_blockhash[BITTENSOR_BLOCKHASH_LENGTH] = {0};
  // memcpy(bittensor_latest_blockhash,
  //        query->sign_txn.signature.blockhash,
  //        BITTENSOR_BLOCKHASH_LENGTH);

  // // update unsigned transaction with latest blockhash
  // int update_status = bittensor_update_blockhash_in_byte_array(
  //     bittensor_txn_context->transaction, bittensor_latest_blockhash);
  // if (update_status != SOL_OK)
  //   return false;

  // sign updated transaction

  uint8_t public_key[32] = {0};
  ed25519_publickey(seed, public_key);

  char buffer[100] = {0};
  byte_array_to_hex_string(seed, 32, buffer, sizeof(buffer));
  printf("\n%s:%d buffer: %s", __func__, __LINE__, buffer);

  ed25519_sign(bittensor_txn_context->transaction,
               bittensor_txn_context->init_info.transaction_size,
               seed,
               public_key,
               sig->signature);
  int check_sig =
      ed25519_sign_open(bittensor_txn_context->transaction,
                        bittensor_txn_context->init_info.transaction_size,
                        public_key,
                        sig->signature);

  if (check_sig == 0) {
    printf("\nSignature Valid");
  } else {
    printf("\nSignature Invalid");
  }

  byte_array_to_hex_string(bittensor_txn_context->transaction,
                           bittensor_txn_context->init_info.transaction_size,
                           buffer,
                           sizeof(buffer));
  printf("\n%s:%d buffer: %s", __func__, __LINE__, buffer);

  byte_array_to_hex_string(sig->signature, 64, buffer, sizeof(buffer));
  printf("\n%s:%d buffer: %s", __func__, __LINE__, buffer);

  memzero(seed, sizeof(seed));

  memcpy(&result.sign_txn.signature,
         sig,
         sizeof(bittensor_sign_txn_signature_response_t));

  bittensor_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void bittensor_sign_transaction(bittensor_query_t *query) {
  bittensor_txn_context =
      (bittensor_txn_context_t *)malloc(sizeof(bittensor_txn_context_t));
  memzero(bittensor_txn_context, sizeof(bittensor_txn_context_t));
  bittensor_sign_txn_signature_response_t sig = {0};
  uint8_t seed[64] = {0};

  if (bittensor_handle_initiate_query(query) &&
      bittensor_fetch_valid_transaction(query) &&
      bittensor_get_user_verification() && fetch_seed(query, seed) &&
      send_signature(query, seed, &sig)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  memzero(seed, sizeof(seed));

  if (NULL != bittensor_txn_context->transaction) {
    free(bittensor_txn_context->transaction);
  }
  if (NULL != bittensor_txn_context) {
    free(bittensor_txn_context);
    bittensor_txn_context = NULL;
  }
}