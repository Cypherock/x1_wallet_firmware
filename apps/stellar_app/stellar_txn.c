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

#include "base32.h"
#include "bip32.h"
#include "composable_app_queue.h"
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
typedef stellar_sign_txn_signature_response_signature_t der_sig_t;

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
                           const der_sig_t *der_signature);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static stellar_txn_context_t *stellar_txn_context = NULL;
static bool use_signature_verification = false;
static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char envelope_b64[2048];
static uint8_t signature_base[1024];

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

  // TODO: Handle wallet search failures - eg: Wallet ID not found, Wallet
  // ID found but is invalid/locked wallet
  if (!check_which_request(query, STELLAR_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             stellar_send_error)) {
    return false;
  }

  snprintf(
      msg, sizeof(msg), UI_TEXT_SIGN_TXN_PROMPT, STELLAR_NAME, wallet_name);
  // Take user consent to sign transaction for the wallet
  if (!core_confirmation(msg, stellar_send_error)) {
    return false;
  }

  set_app_flow_status(STELLAR_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&stellar_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(stellar_sign_txn_initiate_request_t));

  send_response(STELLAR_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
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
  stellar_txn_context->payment =
      (stellar_payment_t *)malloc(sizeof(stellar_payment_t));

  if (stellar_parse_transaction(stellar_txn_context->transaction,
                                total_size,
                                stellar_txn_context->txn,
                                stellar_txn_context->payment) != 0) {
    stellar_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  return true;
}

// Add this helper function before get_user_verification()
static bool show_memo_details(const stellar_transaction_t *decoded_txn) {
  if (decoded_txn->memo_type == MEMO_TEXT) {
    char memo_display[100] = {'\0'};
    snprintf(memo_display,
             sizeof(memo_display),
             "Memo: \"%s\"",
             decoded_txn->memo.text);
    return core_confirmation(memo_display, stellar_send_error);
  } else if (decoded_txn->memo_type == MEMO_ID) {
    char memo_display[50] = {'\0'};
    snprintf(memo_display,
             sizeof(memo_display),
             "Memo ID: %llu",
             decoded_txn->memo.id);
    return core_confirmation(memo_display, stellar_send_error);
  } else if (decoded_txn->memo_type == MEMO_NONE) {
    return core_confirmation("Memo: (none)", stellar_send_error);
  } else if (decoded_txn->memo_type == MEMO_HASH ||
             decoded_txn->memo_type == MEMO_RETURN) {
    char memo_hash[80] = {'\0'};
    char temp[3];
    strcpy(memo_hash, "Memo Hash: ");
    for (int i = 0; i < 32; i++) {
      snprintf(temp, sizeof(temp), "%02x", decoded_txn->memo.hash[i]);
      strcat(memo_hash, temp);
    }
    return core_confirmation(memo_hash, stellar_send_error);
  } else {
    char memo_display[50] = {'\0'};
    snprintf(memo_display,
             sizeof(memo_display),
             "Memo: (unknown type %u)",
             decoded_txn->memo_type);
    return core_confirmation(memo_display, stellar_send_error);
  }
}

// Updated main function
static bool get_user_verification(void) {
  const stellar_transaction_t *decoded_txn = stellar_txn_context->txn;
  const stellar_payment_t *payment = stellar_txn_context->payment;

  char to_address[STELLAR_ADDRESS_LENGTH] = "";
  char from_address[STELLAR_ADDRESS_LENGTH] = "";

  // Generate addresses for display
  if (!stellar_generate_address(payment->destination, to_address)) {
    stellar_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  if (!stellar_generate_address(decoded_txn->source_account, from_address)) {
    stellar_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  // Exchange validation
  if (use_signature_verification) {
    if (!exchange_validate_stored_signature(to_address, sizeof(to_address))) {
      return false;
    }
  }

  // Show operation type
  char operation_display[50] = {'\0'};
  snprintf(operation_display,
           sizeof(operation_display),
           "Operation: %s",
           decoded_txn->operation_type == 0 ? "CREATE_ACCOUNT" : "PAYMENT");
  if (!core_confirmation(operation_display, stellar_send_error)) {
    return false;
  }

  // Show from address
  if (!core_scroll_page("From:", from_address, stellar_send_error)) {
    return false;
  }

  // Show destination address
  if (!core_scroll_page(
          ui_text_verify_address, to_address, stellar_send_error)) {
    return false;
  }

  // Show amount
  char amount_string[30] = {'\0'};
  double decimal_amount = (double)payment->amount;
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

  // Show fee
  char fee_display[50] = {'\0'};
  snprintf(
      fee_display, sizeof(fee_display), "Fee: %lu stroops", decoded_txn->fee);
  if (!core_confirmation(fee_display, stellar_send_error)) {
    return false;
  }

  // Show sequence number
  char seq_display[50] = {'\0'};
  snprintf(seq_display,
           sizeof(seq_display),
           "Sequence: %llu",
           decoded_txn->sequence_number);
  if (!core_confirmation(seq_display, stellar_send_error)) {
    return false;
  }

  // Handle memo display
  if (!show_memo_details(decoded_txn)) {
    return false;
  }

  set_app_flow_status(STELLAR_SIGN_TXN_STATUS_VERIFY);
  return true;
}

// See https://developers.stellar.org/docs/learn/encyclopedia/data-format/xdr
// Envelope type (4 bytes) - ENVELOPE_TYPE_TX = 2
// See https://github.com/stellar/stellar-protocol/blob/master/core/cap-0015.md
static void write_uint32_be(uint8_t *buffer, uint32_t value) {
  buffer[0] = (value >> 24) & 0xFF;
  buffer[1] = (value >> 16) & 0xFF;
  buffer[2] = (value >> 8) & 0xFF;
  buffer[3] = value & 0xFF;
}

static void write_uint64_be(uint8_t *buffer, uint64_t value) {
  write_uint32_be(buffer, (uint32_t)(value >> 32));
  write_uint32_be(buffer + 4, (uint32_t)(value & 0xFFFFFFFF));
}

static void base64_encode(const uint8_t *data, int len, char *output) {
  int i = 0, j = 0;
  uint8_t char_array_3[3];
  uint8_t char_array_4[4];

  while (len--) {
    char_array_3[i++] = *(data++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] =
          ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] =
          ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; i < 4; i++) {
        output[j++] = base64_chars[char_array_4[i]];
      }
      i = 0;
    }
  }

  if (i) {
    for (int k = i; k < 3; k++) {
      char_array_3[k] = '\0';
    }

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (int k = 0; k < i + 1; k++) {
      output[j++] = base64_chars[char_array_4[k]];
    }

    while (i++ < 3) {
      output[j++] = '=';
    }
  }

  output[j] = '\0';
}

static int create_signature_base(const char *network_passphrase,
                                 stellar_transaction_t *tx,
                                 stellar_payment_t *payment,
                                 uint8_t *signature_base,
                                 int *base_len) {
  int pos = 0;

  // 1. Network passphrase hash (32 bytes)
  uint8_t network_hash[32];
  sha256_Raw((const uint8_t *)network_passphrase,
             strlen(network_passphrase),
             network_hash);
  memcpy(signature_base + pos, network_hash, 32);
  pos += 32;

  // 2. TX_TYPE (4 bytes) - ENVELOPE_TYPE_TX = 2
  write_uint32_be(signature_base + pos, 2);
  pos += 4;

  // 3. Source account type (4 bytes) + public key (32 bytes)
  write_uint32_be(signature_base + pos, 0);    // KEY_TYPE_ED25519 = 0
  pos += 4;
  memcpy(signature_base + pos, tx->source_account, 32);
  pos += 32;

  // 4. Fee (4 bytes, big-endian)
  write_uint32_be(signature_base + pos, tx->fee);
  pos += 4;

  // 5. Sequence number (8 bytes, big-endian)
  write_uint64_be(signature_base + pos, tx->sequence_number);
  pos += 8;

  // 6. Time bounds (1 + 8 + 8 = 17 bytes)
  write_uint32_be(signature_base + pos, 1);    // has time bounds = true
  pos += 4;
  write_uint64_be(signature_base + pos, 0);    // min_time = 0
  pos += 8;
  write_uint64_be(signature_base + pos, 0);    // max_time = 0
  pos += 8;

  // 7. Memo
  write_uint32_be(signature_base + pos, tx->memo_type);
  pos += 4;

  if (tx->memo_type == MEMO_TEXT) {
    int memo_len = strlen(tx->memo.text);
    write_uint32_be(signature_base + pos, memo_len);
    pos += 4;
    memcpy(signature_base + pos, tx->memo.text, memo_len);
    pos += memo_len;
    // Add padding to 4-byte boundary
    int padding = (4 - (memo_len % 4)) % 4;
    for (int i = 0; i < padding; i++) {
      signature_base[pos++] = 0;
    }
  } else if (tx->memo_type == MEMO_ID) {
    write_uint64_be(signature_base + pos, tx->memo.id);
    pos += 8;
  } else if (tx->memo_type == MEMO_HASH || tx->memo_type == MEMO_RETURN) {
    memcpy(signature_base + pos, tx->memo.hash, 32);
    pos += 32;
  }

  // 8. Operations count (4 bytes)
  write_uint32_be(signature_base + pos, 1);
  pos += 4;

  // 9. Operation: has_source_account = false
  write_uint32_be(signature_base + pos, 0);
  pos += 4;

  // 10. Operation type = PAYMENT OR CREATE_ACCOUNT
  write_uint32_be(signature_base + pos, tx->operation_type);
  pos += 4;

  // 11. Payment operation
  // Destination (type + pubkey)
  write_uint32_be(signature_base + pos, 0);    // KEY_TYPE_ED25519
  pos += 4;
  memcpy(signature_base + pos, payment->destination, 32);
  pos += 32;
  // Asset (native)
  if (tx->operation_type) {
    write_uint32_be(signature_base + pos, 0);    // ASSET_TYPE_NATIVE
    pos += 4;
  }
  // Amount
  write_uint64_be(signature_base + pos, payment->amount);
  pos += 8;

  // 12. Extension
  write_uint32_be(signature_base + pos, 0);
  pos += 4;

  *base_len = pos;

  return 0;
}

static int stellar_create_signature(stellar_transaction_t *tx,
                                    stellar_payment_t *payment,
                                    const uint8_t *private_key,
                                    const uint8_t *public_key,
                                    stellar_network_t network,
                                    uint8_t *signature) {
  // uint8_t signature_base[1024];
  int base_len;

  // 1. Create signature base
  const char *passphrase = (network == STELLAR_NETWORK_TESTNET)
                               ? TESTNET_PASSPHRASE
                               : MAINNET_PASSPHRASE;

  int result =
      create_signature_base(passphrase, tx, payment, signature_base, &base_len);
  if (result != 0) {
    return result;
  }

  // 2. Hash the signature base (this is what we actually sign)
  uint8_t transaction_hash[32];
  sha256_Raw(signature_base, base_len, transaction_hash);

  // 3. Sign the hash (32 bytes), not the raw signature base
  ed25519_signature sig;
  ed25519_sign(transaction_hash, 32, private_key, public_key, sig);
  memcpy(signature, sig, 64);

  return 0;
}

static int create_signed_transaction_envelope(stellar_transaction_t *tx,
                                              stellar_payment_t *payment,
                                              const uint8_t *private_key,
                                              const uint8_t *public_key,
                                              stellar_network_t network,
                                              char *signed_envelope_b64) {
  uint8_t signature[64];
  uint8_t signed_envelope[1024];
  int pos = 0;

  // 1. Sign the transaction first
  int result = stellar_create_signature(
      tx, payment, private_key, public_key, network, signature);
  if (result != 0) {
    return result;
  }

  // 2. Create TransactionEnvelope XDR structure

  // Envelope type (4 bytes) - ENVELOPE_TYPE_TX = 2
  write_uint32_be(signed_envelope + pos, 2);
  pos += 4;

  // Transaction structure
  // Source account
  write_uint32_be(signed_envelope + pos, 0);    // KEY_TYPE_ED25519
  pos += 4;
  memcpy(signed_envelope + pos, tx->source_account, 32);
  pos += 32;

  // Fee
  write_uint32_be(signed_envelope + pos, tx->fee);
  pos += 4;

  // Sequence number
  write_uint64_be(signed_envelope + pos, tx->sequence_number);
  pos += 8;

  // Preconditions (time bounds)
  write_uint32_be(signed_envelope + pos, 1);    // PRECOND_TIME
  pos += 4;
  write_uint64_be(signed_envelope + pos, 0);    // min_time
  pos += 8;
  write_uint64_be(signed_envelope + pos, 0);    // max_time
  pos += 8;

  // Memo
  write_uint32_be(signed_envelope + pos, tx->memo_type);
  pos += 4;

  if (tx->memo_type == MEMO_TEXT) {
    int memo_len = strlen(tx->memo.text);
    write_uint32_be(signed_envelope + pos, memo_len);
    pos += 4;
    memcpy(signed_envelope + pos, tx->memo.text, memo_len);
    pos += memo_len;
    // Add padding
    int padding = (4 - (memo_len % 4)) % 4;
    for (int i = 0; i < padding; i++) {
      signed_envelope[pos++] = 0;
    }
  } else if (tx->memo_type == MEMO_ID) {
    write_uint64_be(signed_envelope + pos, tx->memo.id);
    pos += 8;
  } else if (tx->memo_type == MEMO_HASH || tx->memo_type == MEMO_RETURN) {
    memcpy(signed_envelope + pos, tx->memo.hash, 32);
    pos += 32;
  }

  // Operations
  write_uint32_be(signed_envelope + pos, 1);    // operation count
  pos += 4;

  // Operation: source account (optional)
  write_uint32_be(signed_envelope + pos, 0);    // no source account
  pos += 4;

  // Operation type
  write_uint32_be(signed_envelope + pos, tx->operation_type);    // PAYMENT
  pos += 4;

  // Payment operation
  write_uint32_be(signed_envelope + pos, 0);    // KEY_TYPE_ED25519
  pos += 4;
  memcpy(signed_envelope + pos, payment->destination, 32);
  pos += 32;
  if (tx->operation_type) {
    write_uint32_be(signed_envelope + pos, 0);    // ASSET_TYPE_NATIVE
    pos += 4;
  }
  write_uint64_be(signed_envelope + pos, payment->amount);
  pos += 8;

  // Transaction extension
  write_uint32_be(signed_envelope + pos, 0);
  pos += 4;

  // 3. Add signatures array
  write_uint32_be(signed_envelope + pos, 1);    // signature count
  pos += 4;

  // Decorated signature
  // Signature hint (last 4 bytes of public key)
  memcpy(signed_envelope + pos, public_key + 28, 4);
  pos += 4;

  // Signature data
  write_uint32_be(signed_envelope + pos, 64);    // signature length
  pos += 4;
  memcpy(signed_envelope + pos, signature, 64);
  pos += 64;

  // 4. Convert to base64
  base64_encode(signed_envelope, pos, signed_envelope_b64);

  return 0;
}

static bool sign_txn(der_sig_t *der_signature) {
  uint8_t seed[64] = {0};
  if (!reconstruct_seed(
          stellar_txn_context->init_info.wallet_id, seed, stellar_send_error)) {
    memzero(seed, sizeof(seed));
    return false;
  }

  set_app_flow_status(STELLAR_SIGN_TXN_STATUS_SEED_GENERATED);

  // Create HDNode from seed and derive private key
  HDNode node = {0};
  if (hdnode_from_seed(seed, 64, "ed25519", &node) != 1) {
    stellar_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    memzero(seed, sizeof(seed));
    memzero(&node, sizeof(HDNode));
    return false;
  }

  // Derive to correct path
  for (uint32_t i = 0; i < stellar_txn_context->init_info.derivation_path_count;
       i++) {
    if (hdnode_private_ckd(
            &node, stellar_txn_context->init_info.derivation_path[i]) != 1) {
      stellar_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
      memzero(seed, sizeof(seed));
      memzero(&node, sizeof(HDNode));
      return false;
    }
  }

  // Fill public key
  hdnode_fill_public_key(&node);

  // Create the full transaction envelope
  // char envelope_b64[2048] = {0};
  int result = create_signed_transaction_envelope(
      stellar_txn_context->txn,
      stellar_txn_context->payment,
      node.private_key,
      node.public_key + 1,    // Skip first byte for Stellar raw public key
      STELLAR_NETWORK_MAINNET,
      envelope_b64);

  if (result != 0) {
    stellar_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    memzero(seed, sizeof(seed));
    memzero(&node, sizeof(HDNode));
    return false;
  }

  // Copy the base64 encoded envelope to the output buffer
  // Check if the envelope fits in the signature buffer
  size_t envelope_len = strlen(envelope_b64);
  if (envelope_len > sizeof(der_signature->bytes)) {
    stellar_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 3);
    memzero(seed, sizeof(seed));
    memzero(&node, sizeof(HDNode));
    return false;
  }

  memcpy(der_signature->bytes, envelope_b64, envelope_len);
  der_signature->size = envelope_len;

  // Clean up sensitive data
  memzero(seed, sizeof(seed));
  memzero(&node, sizeof(HDNode));
  memzero(envelope_b64, sizeof(envelope_b64));

  return true;
}

static bool send_signature(stellar_query_t *query,
                           const der_sig_t *der_signature) {
  stellar_result_t result = init_stellar_result(STELLAR_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = STELLAR_SIGN_TXN_RESPONSE_SIGNATURE_TAG;

  if (!stellar_get_query(query, STELLAR_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, STELLAR_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(
      &result.sign_txn.signature.signature, der_signature, sizeof(der_sig_t));

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

  der_sig_t der_signature = {0};

  if (handle_initiate_query(query) && fetch_valid_input(query) &&
      get_user_verification() && sign_txn(&der_signature) &&
      send_signature(query, &der_signature)) {
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
    if (stellar_txn_context->payment) {
      free(stellar_txn_context->payment);
    }
    free(stellar_txn_context);
    stellar_txn_context = NULL;
  }

  return;
}