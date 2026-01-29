/**
 * @file    sia_txn.c
 * @author  Cypherock X1 Team
 * @brief   Source file to handle transaction signing logic for Sia protocol
 *
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2025 by HODL TECH PTE LTD
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
#include "constant_texts.h"
#include "ed25519.h"
#include "exchange_main.h"
#include "reconstruct_wallet_flow.h"
#include "sia_api.h"
#include "sia_context.h"
#include "sia_helpers.h"
#include "sia_priv.h"
#include "sia_txn_helpers.h"
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
typedef sia_sign_txn_signature_response_t sia_sig_t;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host sia app and return false.
 *
 * @param query Reference to an instance of sia_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const sia_query_t *query,
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
 * @param request Reference to an instance of sia_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(const sia_sign_txn_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * SIA_SIGN_TXN_REQUEST_INITIATE_TAG type of request. Additionally, the
 * wallet-id is validated for sanity and the derivation path for the account is
 * also validated. After the validations, user is prompted about the action for
 * confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the sia_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const sia_query_t *query);

/**
 * @brief Receives unsigned txn from the host. If reception is successful, it
 * also parses the txn to ensure it's validity.
 * @note In case of any failure, a corresponding message is conveyed to the host
 *
 * @param query Reference to buffer of type sia_query_t
 * @return true If the txn is received in the internal buffers and is valid
 * @return false If the txn could not be received or it's validation failed
 */
static bool fetch_valid_input(sia_query_t *query);

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
 * @brief Compute Sia V2TransactionSemantics hash for signing
 * @param txn Parsed transaction structure
 * @param hash Output buffer for 32-byte hash
 * @return int 0 on success, -1 on error
 */
static int sia_compute_semantic_hash(const sia_transaction_t *txn,
                                     uint8_t hash[32]);

/**
 * @brief Create ED25519 signature for Sia transaction
 * @param private_key 32-byte private key for signing
 * @param signature Output buffer for 64-byte signature
 * @return int 0 on success, -1 on error
 */
static int sia_create_signature(const uint8_t *private_key, uint8_t *signature);

/**
 * @brief Calculates ED25519 curve based signature over the digest of the user
 * verified unsigned txn.
 * @details Seed reconstruction takes place within this function. Returns only
 * the 64-byte signature
 *
 * @param signature Reference to buffer where the signature will be
 * populated
 * @return true If the signature was computed successfully
 * @return false If signature could not be computed - maybe due to some error
 * during seed reconstruction phase
 */
static bool sign_txn(uint8_t *signature);

/**
 * @brief Sends signature of the SIA unsigned txn to the host
 * @details The function waits for the host to send a request of type
 * SIA_SIGN_TXN_REQUEST_SIGNATURE_TAG and sends the response
 *
 * @param query Reference to buffer of type sia_query_t
 * @param signature Reference to signature to be sent to the host
 * @return true If the signature was sent successfully
 * @return false If the signature could not be sent - maybe due to and P0 event
 * or invalid request received from the host
 */
static bool send_signature(sia_query_t *query, const sia_sig_t *signature);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static sia_txn_context_t *sia_txn_context = NULL;
static bool use_signature_verification = false;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const sia_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    sia_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static void send_response(const pb_size_t which_response) {
  sia_result_t result = init_sia_result(SIA_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  sia_send_result(&result);
}

static bool validate_request_data(const sia_sign_txn_request_t *request) {
  bool status = true;

  if (!sia_derivation_path_guard(request->initiate.derivation_path,
                                 request->initiate.derivation_path_count)) {
    sia_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
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

static bool handle_initiate_query(const sia_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(query, SIA_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             sia_send_error)) {
    return false;
  }

  snprintf(msg, sizeof(msg), UI_TEXT_SIGN_TXN_PROMPT, SIA_NAME, wallet_name);
  if (!core_confirmation(msg, sia_send_error)) {
    return false;
  }

  set_app_flow_status(SIA_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&sia_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(sia_sign_txn_initiate_request_t));

  send_response(SIA_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool fetch_valid_input(sia_query_t *query) {
  uint32_t size = 0;
  sia_result_t response = init_sia_result(SIA_RESULT_SIGN_TXN_TAG);
  uint32_t total_size = sia_txn_context->init_info.transaction_size;
  const sia_sign_txn_data_t *txn_data = &query->sign_txn.txn_data;
  const common_chunk_payload_t *payload = &txn_data->chunk_payload;
  const common_chunk_payload_chunk_t *chunk = &txn_data->chunk_payload.chunk;

  // allocate memory for storing transaction
  sia_txn_context->transaction = (uint8_t *)malloc(total_size);

  while (1) {
    if (!sia_get_query(query, SIA_QUERY_SIGN_TXN_TAG) ||
        !check_which_request(query, SIA_SIGN_TXN_REQUEST_TXN_DATA_TAG)) {
      return false;
    }

    if (!txn_data->has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        size + payload->chunk.size > total_size) {
      sia_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                     ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    memcpy(&sia_txn_context->transaction[size], chunk->bytes, chunk->size);
    size += chunk->size;

    // Send chunk ack to host
    response.sign_txn.which_response = SIA_SIGN_TXN_RESPONSE_DATA_ACCEPTED_TAG;
    response.sign_txn.data_accepted.has_chunk_ack = true;
    response.sign_txn.data_accepted.chunk_ack.chunk_index =
        payload->chunk_index;
    sia_send_result(&response);

    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }

  // make sure all chunks were received
  if (size != total_size) {
    sia_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  sia_txn_context->txn = (sia_transaction_t *)malloc(sizeof(sia_transaction_t));

  if (sia_parse_transaction(sia_txn_context->transaction,
                            total_size,
                            sia_txn_context->txn) == false) {
    sia_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  return true;
}

static bool get_user_verification(void) {
  const sia_transaction_t *decoded_txn = sia_txn_context->txn;

  char to_address[SIA_ADDRESS_SIZE] = "";

  // Generate full address(32 byte hash  + 6 byte checksum)
  if (!sia_full_address(decoded_txn->outputs[0].address_hash, to_address)) {
    sia_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  // Exchange validation
  if (use_signature_verification) {
    if (!exchange_validate_stored_signature(to_address, sizeof(to_address))) {
      return false;
    }
  }

  // Verify destination address
  if (!core_scroll_page(ui_text_verify_address, to_address, sia_send_error)) {
    return false;
  }

  // Verify amount

  double amount_sc = sia_convert_to_sc(decoded_txn->outputs[0].value_lo,
                                       decoded_txn->outputs[0].value_hi);
  char amount_str[50];
  snprintf(amount_str, sizeof(amount_str), "%.6f", amount_sc);

  char amount_display[100] = {0};
  snprintf(amount_display,
           sizeof(amount_display),
           UI_TEXT_VERIFY_AMOUNT,
           amount_str,
           SIA_LUNIT);

  if (!core_confirmation(amount_display, sia_send_error)) {
    return false;
  }

  // Verify fee
  double fee_sc = sia_convert_to_sc(decoded_txn->fee_lo, decoded_txn->fee_hi);
  char fee_str[50];
  snprintf(fee_str, sizeof(fee_str), "%.6f", fee_sc);
  char fee_display[100] = {0};
  snprintf(
      fee_display, sizeof(fee_display), UI_TEXT_VERIFY_FEE, fee_str, SIA_LUNIT);

  if (!core_confirmation(fee_display, sia_send_error)) {
    return false;
  }

  set_app_flow_status(SIA_SIGN_TXN_STATUS_VERIFY);
  return true;
}

// See https://github.com/SiaFoundation/core/blob/master/consensus/state.go#L588
// InputSigHash() calls hashAll("sig/input", v2ReplayPrefix(),
// V2TransactionSemantics)
//
// hashAll uses WriteDistinguisher which prepends "sia/" + distinguisher + "|"
// See: https://github.com/SiaFoundation/core/blob/master/types/hash.go#L29
// Result: "sia/" + "sig/input" + "|" = "sia/sig/input|"
//
// Replay protection:
// https://github.com/SiaFoundation/core/blob/master/consensus/state.go#L448
// v2ReplayPrefix() returns 2
//
// V2TransactionSemantics structure:
// https://github.com/SiaFoundation/core/blob/master/types/encoding.go#L814
// V2Currency encoding:
// https://github.com/SiaFoundation/core/blob/master/types/encoding.go#L396

static int sia_compute_semantic_hash(const sia_transaction_t *txn,
                                     uint8_t hash[32]) {
  blake2b_state hasher;
  blake2b_Init(&hasher, 32);
  size_t offset = 0;

  // Signature prefix
  const uint8_t sig_prefix[] = {
      's', 'i', 'a', '/', 's', 'i', 'g', '/', 'i', 'n', 'p', 'u', 't', '|'};
  blake2b_Update(&hasher, sig_prefix, sizeof(sig_prefix));

  // Replay protection
  const uint8_t replay = 2;
  blake2b_Update(&hasher, &replay, 1);

  // Use raw buffer: Input count + Parent IDs + Output count
  size_t len = 8 + (txn->input_count * 32) + 8;
  blake2b_Update(&hasher, &sia_txn_context->transaction[offset], len);
  offset += len;

  for (int i = 0; i < txn->output_count; i++) {
    // Raw format: Address(32) + Value_Lo(8) + Value_Hi(8)
    // Hash format: Value_Lo(8) + Value_Hi(8) + Address(32)
    blake2b_Update(&hasher, &sia_txn_context->transaction[offset + 32], 8);
    blake2b_Update(&hasher, &sia_txn_context->transaction[offset + 40], 8);
    blake2b_Update(&hasher, &sia_txn_context->transaction[offset], 32);
    offset += 48;
  }

  // Empty fields (56 bytes of zeros + 1 false byte) for
  // 1. Sia Funds inputs count
  // 2. Sia Funds outputs count
  // 3. File contracts count
  // 4. File contract revisions count
  // 5. File contract resolutions count
  // 6. Attestations count
  // 7. Arbitrary data length
  uint8_t zero_bytes[8 * 7] = {0};
  blake2b_Update(&hasher, zero_bytes, 56);
  uint8_t false_byte = 0;
  blake2b_Update(&hasher, &false_byte, 1);    // New foundation address

  // Miner fee (V2Currency format)
  blake2b_Update(&hasher, &sia_txn_context->transaction[offset], 16);

  blake2b_Final(&hasher, hash, 32);
  return 0;
}

static int sia_create_signature(const uint8_t *private_key,
                                uint8_t *signature) {
  uint8_t transaction_hash[32];
  if (sia_compute_semantic_hash(sia_txn_context->txn, transaction_hash) != 0) {
    return -1;
  }

  uint8_t public_key[32];
  ed25519_publickey(private_key, public_key);
  ed25519_sign(transaction_hash, 32, private_key, public_key, signature);

  return 0;
}

static bool sign_txn(uint8_t *signature) {
  uint8_t seed[32] = {0};

  // Reconstruct seed
  if (!reconstruct_sia_seed(
          sia_txn_context->init_info.wallet_id, seed, sia_send_error)) {
    memzero(seed, sizeof(seed));
    return false;
  }

  set_app_flow_status(SIA_SIGN_TXN_STATUS_SEED_GENERATED);

  uint8_t buffer[40] = {0}, private_key[SIA_PRIVATE_KEY_SIZE] = {
                                0};    // buffer size 40 = 32(seed) + 8(index)
  memcpy(buffer, seed, 32);
  uint64_t index = sia_txn_context->init_info.derivation_path[0];

  for (int i = 0; i < 8; i++) {
    buffer[32 + i] = (index >> (i * 8)) & 0xFF;
  }

  if (blake2b(buffer, 40, private_key, 32) != 0) {
    sia_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    memzero(seed, sizeof(seed));
    memzero(buffer, sizeof(buffer));
    memzero(private_key, sizeof(private_key));
    return false;
  }

  int result = sia_create_signature(private_key, signature);

  memzero(seed, sizeof(seed));
  memzero(buffer, sizeof(buffer));
  memzero(private_key, sizeof(private_key));

  if (result != 0) {
    sia_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  return true;
}

static bool send_signature(sia_query_t *query, const sia_sig_t *signature) {
  sia_result_t result = init_sia_result(SIA_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = SIA_SIGN_TXN_RESPONSE_SIGNATURE_TAG;

  if (!sia_get_query(query, SIA_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, SIA_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(&result.sign_txn.signature, signature, sizeof(sia_sig_t));

  sia_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void sia_sign_transaction(sia_query_t *query) {
  sia_txn_context = (sia_txn_context_t *)malloc(sizeof(sia_txn_context_t));
  memzero(sia_txn_context, sizeof(sia_txn_context_t));

  sia_sig_t sig = {0};

  if (handle_initiate_query(query) && fetch_valid_input(query) &&
      get_user_verification() && sign_txn(sig.signature) &&
      send_signature(query, &sig)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  // Clean up
  if (sia_txn_context) {
    if (sia_txn_context->transaction) {
      free(sia_txn_context->transaction);
    }
    if (sia_txn_context->txn) {
      free(sia_txn_context->txn);
    }
    free(sia_txn_context);
    sia_txn_context = NULL;
  }

  return;
}