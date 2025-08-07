/**
 * @file    stellar_txn.c
 * @author  Cypherock X1 Team
 * @brief   Source file to handle transaction signing logic for Stellar protocol
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

#include "bip32.h"
#include "composable_app_queue.h"
#include "constant_texts.h"
#include "curves.h"
#include "ed25519-donna/ed25519-donna.h"
#include "ed25519-donna/ed25519-hash-custom.h"
#include "exchange_main.h"
#include "hasher.h"
#include "reconstruct_wallet_flow.h"
#include "sha2.h"
#include "status_api.h"
#include "stellar_api.h"
#include "stellar_context.h"
#include "stellar_helpers.h"
#include "stellar_priv.h"
#include "stellar_txn_helpers.h"
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
typedef stellar_sign_txn_signature_response_t stellar_sig_t;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host stellar app and return false.
 *
 * @param query Reference to an instance of stellar_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const stellar_query_t *query,
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
 * @param request Reference to an instance of stellar_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(const stellar_sign_txn_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * STELLAR_SIGN_TXN_REQUEST_INITIATE_TAG type of request. Additionally, the
 * wallet-id is validated for sanity and the derivation path for the account is
 * also validated. After the validations, user is prompted about the action for
 * confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the stellar_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const stellar_query_t *query);

/**
 * @brief Receives unsigned txn from the host. If reception is successful, it
 * also parses the txn to ensure it's validity.
 * @note In case of any failure, a corresponding message is conveyed to the host
 *
 * @param query Reference to buffer of type stellar_query_t
 * @return true If the txn is received in the internal buffers and is valid
 * @return false If the txn could not be received or it's validation failed
 */
static bool fetch_valid_input(stellar_query_t *query);

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
 * @details Seed reconstruction takes place within this function. Returns only
 * the 64-byte signature, not the full transaction envelope.
 *
 * @param signature Reference to buffer where the signature will be
 * populated
 * @return true If the signature was computed successfully
 * @return false If signature could not be computed - maybe due to some error
 * during seed reconstruction phase
 */
static bool sign_txn(uint8_t *signature);

/**
 * @brief Creates signature base data for Stellar transaction signing
 * @details Combines network passphrase hash with truncated XDR transaction data
 *
 * @param network_passphrase Network passphrase string (mainnet/testnet)
 * @param transaction_xdr Raw XDR transaction data
 * @param tagged_txn_len Length of signature-relevant tagged txn XDR
 * @param output Output buffer for signature base data
 * @return int 0 on success, non-zero on error
 */

static int create_signature_base(const char *network_passphrase,
                                 uint8_t *transaction_xdr,
                                 size_t tagged_txn_len,
                                 uint8_t *output);
/**
 * @brief Creates ED25519 signature for Stellar transaction
 * @details Signs the transaction hash using ED25519 algorithm
 * See
 * https://github.com/trezor/trezor-firmware/blob/main/core/src/apps/stellar/sign_tx.py
 *
 * @param private_key ED25519 private key for signing
 * @param public_key ED25519 public key for verification
 * @param network Target network (mainnet/testnet)
 * @param signature Output buffer for 64-byte signature
 * @return int 0 on success, non-zero on error
 */
static int stellar_create_signature(const uint8_t *private_key,
                                    const uint8_t *public_key,
                                    stellar_network_t network,
                                    uint8_t *signature);

/**
 * @brief Sends signature of the STELLAR unsigned txn to the host
 * @details The function waits for the host to send a request of type
 * STELLAR_SIGN_TXN_REQUEST_SIGNATURE_TAG and sends the response
 *
 * @param query Reference to buffer of type stellar_query_t
 * @param signature Reference to signature to be sent to the host
 * @return true If the signature was sent successfully
 * @return false If the signature could not be sent - maybe due to and P0 event
 * or invalid request received from the host
 */
static bool send_signature(stellar_query_t *query,
                           const stellar_sig_t *signature);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static stellar_txn_context_t *stellar_txn_context = NULL;
static bool use_signature_verification = false;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const stellar_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    stellar_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static void send_response(const pb_size_t which_response) {
  stellar_result_t result = init_stellar_result(STELLAR_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  stellar_send_result(&result);
}

static bool validate_request_data(const stellar_sign_txn_request_t *request) {
  bool status = true;

  if (!stellar_derivation_path_guard(request->initiate.derivation_path,
                                     request->initiate.derivation_path_count)) {
    stellar_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
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

static bool handle_initiate_query(const stellar_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(query, STELLAR_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             stellar_send_error)) {
    return false;
  }

  snprintf(
      msg, sizeof(msg), UI_TEXT_SIGN_TXN_PROMPT, STELLAR_NAME, wallet_name);
  if (!core_confirmation(msg, stellar_send_error)) {
    return false;
  }

  set_app_flow_status(STELLAR_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&stellar_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(stellar_sign_txn_initiate_request_t));

  send_response(STELLAR_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool fetch_valid_input(stellar_query_t *query) {
  uint32_t size = 0;
  stellar_result_t response = init_stellar_result(STELLAR_RESULT_SIGN_TXN_TAG);
  uint32_t total_size = stellar_txn_context->init_info.transaction_size;
  const stellar_sign_txn_data_t *txn_data = &query->sign_txn.txn_data;
  const common_chunk_payload_t *payload = &txn_data->chunk_payload;
  const common_chunk_payload_chunk_t *chunk = &txn_data->chunk_payload.chunk;

  // allocate memory for storing transaction
  stellar_txn_context->transaction = (uint8_t *)malloc(total_size);

  while (1) {
    if (!stellar_get_query(query, STELLAR_QUERY_SIGN_TXN_TAG) ||
        !check_which_request(query, STELLAR_SIGN_TXN_REQUEST_TXN_DATA_TAG)) {
      return false;
    }

    if (!txn_data->has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        size + payload->chunk.size > total_size) {
      stellar_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    memcpy(&stellar_txn_context->transaction[size], chunk->bytes, chunk->size);
    size += chunk->size;

    // Send chunk ack to host
    response.sign_txn.which_response =
        STELLAR_SIGN_TXN_RESPONSE_DATA_ACCEPTED_TAG;
    response.sign_txn.data_accepted.has_chunk_ack = true;
    response.sign_txn.data_accepted.chunk_ack.chunk_index =
        payload->chunk_index;
    stellar_send_result(&response);

    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }

  // make sure all chunks were received
  if (size != total_size) {
    stellar_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  stellar_txn_context->txn =
      (stellar_transaction_t *)malloc(sizeof(stellar_transaction_t));

  if (stellar_parse_transaction(stellar_txn_context->transaction,
                                total_size,
                                stellar_txn_context->txn,
                                &stellar_txn_context->tagged_txn_len) != 0) {
    stellar_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  return true;
}

static bool verify_memo_details(const stellar_transaction_t *decoded_txn) {
  char memo_display[100] = {'\0'};
  switch (decoded_txn->memo_type) {
    case STELLAR_MEMO_NONE: {
      return true;
    }
    case STELLAR_MEMO_TEXT: {
      (void)snprintf(memo_display,
                     sizeof(memo_display),
                     UI_TEXT_VERIFY_MEMO,
                     decoded_txn->memo.text);
      break;
    }
    case STELLAR_MEMO_ID: {
      (void)snprintf(memo_display,
                     sizeof(memo_display),
                     UI_TEXT_VERIFY_MEMO_ID,
                     decoded_txn->memo.id);
      break;
    }
    case STELLAR_MEMO_HASH:
    case STELLAR_MEMO_RETURN: {
      char hex_hash[32 * 2 + 1] = "";
      byte_array_to_hex_string(
          decoded_txn->memo.hash, 32, hex_hash, sizeof(hex_hash));
      (void)snprintf(memo_display,
                     sizeof(memo_display),
                     UI_TEXT_VERIFY_MEMO_HASH,
                     hex_hash);
      break;
    }
    default: {
      return false;
    }
  }
  return core_confirmation(memo_display, stellar_send_error);
}

static bool get_user_verification(void) {
  const stellar_transaction_t *decoded_txn = stellar_txn_context->txn;

  char to_address[STELLAR_ADDRESS_LENGTH] = "";

  // Generate addresses for display
  if (!stellar_generate_address(decoded_txn->operations[0].destination,
                                to_address)) {
    stellar_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  // Exchange validation
  if (use_signature_verification) {
    if (!exchange_validate_stored_signature(to_address, sizeof(to_address))) {
      return false;
    }
  }

  // Verify destination address
  if (!core_scroll_page(
          ui_text_verify_address, to_address, stellar_send_error)) {
    return false;
  }

  // Verify amount
  char amount_string[30] = {'\0'};
  double decimal_amount = (double)decoded_txn->operations[0].amount;
  decimal_amount *= 1e-7;    // Convert stroops to XLM
  snprintf(amount_string, sizeof(amount_string), "%.7f", decimal_amount);

  char display[100] = {'\0'};
  snprintf(display,
           sizeof(display),
           UI_TEXT_VERIFY_AMOUNT,
           amount_string,
           STELLAR_LUNIT);

  if (!core_confirmation(display, stellar_send_error)) {
    return false;
  }

  // Verify fee
  char fee_string[30] = {'\0'};
  double decimal_fee = (double)decoded_txn->fee;
  decimal_fee *= 1e-7;    // Convert stroops to XLM
  snprintf(fee_string, sizeof(fee_string), "%.7f", decimal_fee);

  char fee_display[50] = {'\0'};
  snprintf(fee_display,
           sizeof(fee_display),
           UI_TEXT_VERIFY_FEE,
           fee_string,
           STELLAR_LUNIT);

  if (!core_confirmation(fee_display, stellar_send_error)) {
    return false;
  }

  // Handle memo display
  if (!verify_memo_details(decoded_txn)) {
    return false;
  }

  set_app_flow_status(STELLAR_SIGN_TXN_STATUS_VERIFY);
  return true;
}

static int create_signature_base(const char *network_passphrase,
                                 uint8_t *transaction_xdr,
                                 size_t tagged_txn_len,
                                 uint8_t *output) {
  // Hash network passphrase
  uint8_t network_hash[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw((const uint8_t *)network_passphrase,
             strlen(network_passphrase),
             network_hash);

  // Combine: network_hash + truncated_xdr (only signature-relevant part)
  memcpy(output, network_hash, SHA256_DIGEST_LENGTH);
  memcpy(output + SHA256_DIGEST_LENGTH, transaction_xdr, tagged_txn_len);

  return 0;
}

static int stellar_create_signature(const uint8_t *private_key,
                                    const uint8_t *public_key,
                                    stellar_network_t network,
                                    uint8_t *signature) {
  size_t tagged_txn_len = stellar_txn_context->tagged_txn_len;
  size_t base_len = SHA256_DIGEST_LENGTH + tagged_txn_len;
  uint8_t signature_base[base_len];
  memzero(signature_base, base_len);

  // Create signature base
  const char *passphrase = (network == STELLAR_NETWORK_TESTNET)
                               ? TESTNET_PASSPHRASE
                               : MAINNET_PASSPHRASE;

  int result = create_signature_base(passphrase,
                                     stellar_txn_context->transaction,
                                     tagged_txn_len,
                                     signature_base);

  if (result != 0) {
    return result;
  }

  // Hash the signature base
  uint8_t transaction_hash[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw(signature_base, base_len, transaction_hash);

  // Sign the hash
  ed25519_signature sig = {0};
  ed25519_sign(
      transaction_hash, SHA256_DIGEST_LENGTH, private_key, public_key, sig);
  memcpy(signature, sig, STELLAR_SIGNATURE_SIZE);

  return 0;
}

static bool sign_txn(uint8_t *signature) {
  uint8_t seed[64] = {0};

  // Reconstruct seed
  if (!reconstruct_seed(
          stellar_txn_context->init_info.wallet_id, seed, stellar_send_error)) {
    memzero(seed, sizeof(seed));
    return false;
  }

  set_app_flow_status(STELLAR_SIGN_TXN_STATUS_SEED_GENERATED);

  // Create HDNode from seed and derive private key using
  // derive_hdnode_from_path
  HDNode node = {0};
  if (!derive_hdnode_from_path(
          stellar_txn_context->init_info.derivation_path,
          stellar_txn_context->init_info.derivation_path_count,
          ED25519_NAME,
          seed,
          &node)) {
    stellar_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    memzero(seed, sizeof(seed));
    memzero(&node, sizeof(HDNode));
    return false;
  }

  // Create signature only (not full envelope)
  int result = stellar_create_signature(
      node.private_key,
      node.public_key + 1,    // Skip first byte for Stellar raw public key
      STELLAR_NETWORK_MAINNET,
      signature);

  // Clean up sensitive data
  memzero(seed, sizeof(seed));
  memzero(&node, sizeof(HDNode));

  if (result != 0) {
    stellar_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  return true;
}

static bool send_signature(stellar_query_t *query,
                           const stellar_sig_t *signature) {
  stellar_result_t result = init_stellar_result(STELLAR_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = STELLAR_SIGN_TXN_RESPONSE_SIGNATURE_TAG;

  if (!stellar_get_query(query, STELLAR_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, STELLAR_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(&result.sign_txn.signature, signature, sizeof(stellar_sig_t));

  stellar_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void stellar_sign_transaction(stellar_query_t *query) {
  stellar_txn_context =
      (stellar_txn_context_t *)malloc(sizeof(stellar_txn_context_t));
  memzero(stellar_txn_context, sizeof(stellar_txn_context_t));

  stellar_sig_t sig = {0};

  if (handle_initiate_query(query) && fetch_valid_input(query) &&
      get_user_verification() && sign_txn(sig.signature) &&
      send_signature(query, &sig)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  // Clean up
  if (stellar_txn_context) {
    if (stellar_txn_context->transaction) {
      free(stellar_txn_context->transaction);
    }
    if (stellar_txn_context->txn) {
      free(stellar_txn_context->txn);
    }
    free(stellar_txn_context);
    stellar_txn_context = NULL;
  }

  return;
}