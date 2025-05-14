/**
 * @file    solana_sign_txn.c
 * @author  Cypherock X1 Team
 * @brief   Solana chain transaction signing logic
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

#include <ed25519-donna.h>

#include "composable_app_queue.h"
#include "exchange_main.h"
#include "int-util.h"
#include "reconstruct_wallet_flow.h"
#include "solana_api.h"
#include "solana_contracts.h"
#include "solana_helpers.h"
#include "solana_priv.h"
#include "solana_txn_helpers.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "wallet_list.h"
/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/
/**
 * @brief Whitelisted contracts with respective token symbol
 * @details A map of Solana Token addresses with their token symbols. These
 * will enable the device to verify the token transaction in a
 * user-friendly manner.
 *
 * @see solana_token_program_t
 */
extern const solana_token_program_t solana_token_program[];
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
 * @param query Reference to an instance of solana_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const solana_query_t *query,
                                pb_size_t which_request);

/**
 * @brief Validates the derivation path received in the request from host
 * @details The function validates the provided account derivation path in the
 * request. If invalid path is detected, the function will send an error to the
 * host and return false.
 *
 * @param request Reference to an instance of solana_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(const solana_sign_txn_request_t *request);

/**
 * @brief The function prepares and sends empty responses
 *
 * @param which_response Constant value for the response type to be sent
 */
static void send_response(pb_size_t which_response);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the SOLANA_SIGN_TXN
 * type of request. Additionally, the wallet-id is validated for sanity and the
 * derivation path for the account is also validated. After the validations,
 * user is prompted about the action for confirmation. The function returns true
 * indicating all the validation and user confirmation was a success. The
 * function also duplicates the data from query into the solana_txn_context  for
 * further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
STATIC bool solana_handle_initiate_query(const solana_query_t *query);

/**
 * @brief Fetches complete raw transaction to be signed for verification
 * @details The function will try to fetch the transaction by referring to the
 * declared size in solana_txn_context. The function will store complete
 * transaction into solana_txn_context.transaction.
 *
 * @param query Reference to an instance of solana_query_t for storing the
 * transient transaction chunks.
 *
 * @return bool Indicating if the whole transaction received and verified
 * @retval true If all the transaction was fetched and verified
 * @retval false If the transaction failed verification or wasn't fetched
 */
STATIC bool solana_fetch_valid_transaction(solana_query_t *query);

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
STATIC bool solana_get_user_verification();

/**
 * @brief Fetches seed for generating public and private key
 * @details The function internally calls wallet reconstruction sub-flow to get
 * access to the seed.
 *
 * @param query Reference to an instance of solana_query_t to store transient
 * request from the host
 * @param seed_out seed recontructed from wallet
 * @return bool Indicating if signature for the provided transaction was
 * successfully generated
 * @retval true If all the seed is generated without any error
 * @retval false If seed failed to generate
 */
STATIC bool fetch_seed(solana_query_t *query, uint8_t *seed_out);

/**
 * @brief Sends the generated signature to the host
 * @details The function internally updates the unsigned transaction with a
 * recent blockhash and signs the transaction before sending to the host
 *
 * @param query Reference to an instance of solana_query_t to store transient
 * request from the host
 * @param seed seed reconstructed from wallet
 * @param sig Reference to the struct having the signature components
 * @return bool Indicating if the signature is sent to the host
 * @retval true If the signature was sent to host successfully
 * @retval false If the host responded with unknown/wrong query
 */
static bool send_signature(solana_query_t *query,
                           uint8_t *seed,
                           solana_sign_txn_signature_response_t *sig);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

STATIC solana_txn_context_t *solana_txn_context = NULL;
static bool use_signature_verification = false;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const solana_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    solana_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request_data(const solana_sign_txn_request_t *request) {
  bool status = true;

  if (!solana_derivation_path_guard(request->initiate.derivation_path,
                                    request->initiate.derivation_path_count)) {
    solana_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
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

static void send_response(const pb_size_t which_response) {
  solana_result_t result = init_solana_result(SOLANA_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  solana_send_result(&result);
}

STATIC bool solana_handle_initiate_query(const solana_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(query, SOLANA_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             solana_send_error)) {
    return false;
  }

  snprintf(msg, sizeof(msg), UI_TEXT_SIGN_TXN_PROMPT, SOLANA_NAME, wallet_name);

  // Take user consent to sign the transaction for the wallet
  if (!core_confirmation(msg, solana_send_error)) {
    return false;
  }

  solana_txn_context->is_token_transfer_transaction =
      query->sign_txn.initiate.has_token_data;

  if (solana_txn_context->is_token_transfer_transaction) {
    // if it is a token transfer transaction, store the token data
    memcpy(&solana_txn_context->token_data,
           &query->sign_txn.initiate.token_data,
           sizeof(solana_sign_txn_initiate_token_data_t));
  }

  set_app_flow_status(SOLANA_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&solana_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(solana_sign_txn_initiate_request_t));

  send_response(SOLANA_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

STATIC bool solana_fetch_valid_transaction(solana_query_t *query) {
  uint32_t size = 0;
  solana_result_t response = init_solana_result(SOLANA_RESULT_SIGN_TXN_TAG);
  uint32_t total_size = solana_txn_context->init_info.transaction_size;
  const solana_sign_txn_data_t *txn_data = &query->sign_txn.txn_data;
  const common_chunk_payload_t *payload = &txn_data->chunk_payload;
  const common_chunk_payload_chunk_t *chunk = &txn_data->chunk_payload.chunk;

  // allocate memory for storing transaction
  solana_txn_context->transaction = (uint8_t *)malloc(total_size);
  while (1) {
    if (!solana_get_query(query, SOLANA_QUERY_SIGN_TXN_TAG) ||
        !check_which_request(query, SOLANA_SIGN_TXN_REQUEST_TXN_DATA_TAG)) {
      return false;
    }

    if (!txn_data->has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        size + payload->chunk.size > total_size) {
      solana_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    memcpy(&solana_txn_context->transaction[size], chunk->bytes, chunk->size);
    size += chunk->size;
    // Send chunk ack to host
    response.sign_txn.which_response =
        SOLANA_SIGN_TXN_RESPONSE_DATA_ACCEPTED_TAG;
    response.sign_txn.data_accepted.has_chunk_ack = true;
    response.sign_txn.data_accepted.chunk_ack.chunk_index =
        payload->chunk_index;
    solana_send_result(&response);

    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }

  // make sure all chunks were received
  if (size != total_size) {
    solana_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  // decode and verify the received transaction
  if (SOL_OK != solana_byte_array_to_unsigned_txn(
                    solana_txn_context->transaction,
                    total_size,
                    &solana_txn_context->transaction_info,
                    &solana_txn_context->extra_data) ||
      SOL_OK !=
          solana_validate_unsigned_txn(&solana_txn_context->transaction_info)) {
    return false;
  }

  return true;
}

static bool verify_priority_fee() {
  if (solana_txn_context->extra_data.compute_unit_price_micro_lamports > 0) {
    // verify priority fee
    uint64_t priority_fee, carry;

    // Capacity to multiply 2 numbers upto 8-byte value and store the result in
    // 2 separate 8-byte variables
    priority_fee =
        mul128(solana_txn_context->extra_data.compute_unit_price_micro_lamports,
               solana_txn_context->extra_data.compute_unit_limit,
               &carry);

    // prepare the whole 128-bit little-endian representation of priority fee
    uint8_t be_micro_lamports[16] = {0};
    memcpy(be_micro_lamports, &priority_fee, sizeof(priority_fee));
    memcpy(be_micro_lamports + sizeof(priority_fee), &carry, sizeof(carry));

    // outputs 128-bit (16-byte) big-endian representation of priority fee
    cy_reverse_byte_array(be_micro_lamports, sizeof(be_micro_lamports));

    char priority_fee_string[33] = {'\0'},
         priority_fee_decimal_string[34] = {'\0'};

    byte_array_to_hex_string(be_micro_lamports,
                             sizeof(be_micro_lamports),
                             priority_fee_string,
                             sizeof(priority_fee_string));
    if (!convert_byte_array_to_decimal_string(
            sizeof(priority_fee_string) - 1,
            solana_get_decimal() + 6,    // +6 for micro
            priority_fee_string,
            priority_fee_decimal_string,
            sizeof(priority_fee_decimal_string))) {
      solana_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
      return false;
    }

    char display[100] = "";
    snprintf(display,
             sizeof(display),
             UI_TEXT_VERIFY_PRIORITY_FEE,
             priority_fee_decimal_string,
             SOLANA_LUNIT);
    if (!core_confirmation(display, solana_send_error)) {
      return false;
    }
  }
  return true;
}

static bool verify_solana_transfer_sol_transaction() {
  char address[45] = {0};
  size_t address_size = sizeof(address);

  const uint8_t transfer_instruction_index =
      solana_txn_context->extra_data.transfer_instruction_index;
  // verify recipient address;
  if (!b58enc(address,
              &address_size,
              solana_txn_context->transaction_info
                  .instruction[transfer_instruction_index]
                  .program.transfer.recipient_account,
              SOLANA_ACCOUNT_ADDRESS_LENGTH)) {
    solana_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  if (use_signature_verification) {
    if (!exchange_validate_stored_signature(address, sizeof(address))) {
      return false;
    }
  }

  if (!core_scroll_page(ui_text_verify_address, address, solana_send_error)) {
    return false;
  }

  // verify recipient amount
  char amount_string[40] = {'\0'}, amount_decimal_string[30] = {'\0'};
  char display[100] = "";

  uint8_t be_lamports[8] = {0};
  int i = 8;
  while (i--)
    be_lamports[i] = solana_txn_context->transaction_info
                         .instruction[transfer_instruction_index]
                         .program.transfer.lamports >>
                     8 * (7 - i);

  byte_array_to_hex_string(
      be_lamports, 8, amount_string, sizeof(amount_string));
  if (!convert_byte_array_to_decimal_string(16,
                                            solana_get_decimal(),
                                            amount_string,
                                            amount_decimal_string,
                                            sizeof(amount_decimal_string))) {
    solana_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    return false;
  }

  snprintf(display,
           sizeof(display),
           UI_TEXT_VERIFY_AMOUNT,
           amount_decimal_string,
           SOLANA_LUNIT);
  if (!core_confirmation(display, solana_send_error)) {
    return false;
  }

  if (!verify_priority_fee())
    return false;

  set_app_flow_status(SOLANA_SIGN_TXN_STATUS_VERIFY);
  return true;
}

static bool create_program_address(
    const uint8_t seed[][SOLANA_ACCOUNT_ADDRESS_LENGTH],
    const uint8_t seeds_size[],
    const uint8_t count,
    const uint8_t *program_address,
    uint8_t *pub_key) {
  uint8_t buffer[300] = {0};
  uint8_t buffer_size = 0;
  for (uint8_t i = 0; i < count; i++) {
    memcpy(buffer + buffer_size, seed[i], seeds_size[i]);
    buffer_size += seeds_size[i];
  }

  // Append program id and pda_string
  memcpy(buffer + buffer_size, program_address, SOLANA_ACCOUNT_ADDRESS_LENGTH);
  buffer_size += SOLANA_ACCOUNT_ADDRESS_LENGTH;

  const char *pda_string = "ProgramDerivedAddress";
  memcpy(buffer + buffer_size, pda_string, strlen(pda_string));
  buffer_size += strlen(pda_string);

  uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw(buffer, buffer_size, hash);

  // check if point on curve
  ge25519 r;
  if (!ge25519_unpack_vartime(&r, hash)) {
    // point is off curve; return hash as public key
    memcpy(pub_key, hash, SOLANA_ACCOUNT_ADDRESS_LENGTH);
    return true;
  }

  if (!ge25519_check(&r)) {
    // point is off curve; return hash as public key
    memcpy(pub_key, hash, SOLANA_ACCOUNT_ADDRESS_LENGTH);
    return true;
  }

  return false;
}

static bool find_program_address(
    const uint8_t seed[][SOLANA_ACCOUNT_ADDRESS_LENGTH],
    const uint8_t count,
    const uint8_t *program_address,
    uint8_t *address) {
  uint8_t nonce = 255;

  // append nonce to seed
  uint8_t seed_with_nonce[count + 1][SOLANA_ACCOUNT_ADDRESS_LENGTH];
  uint8_t seeds_size[count + 1];
  // copy initial seeds
  for (int i = 0; i < count; i++) {
    memcpy(seed_with_nonce[i], seed[i], SOLANA_ACCOUNT_ADDRESS_LENGTH);
    seeds_size[i] = SOLANA_ACCOUNT_ADDRESS_LENGTH;
  }
  seeds_size[count] = 1;    // nonce size

  while (nonce != 0) {
    seed_with_nonce[count][0] = nonce;
    if (create_program_address(
            seed_with_nonce, seeds_size, count + 1, program_address, address)) {
      return true;
      break;
    }
    nonce -= 1;
  }
  return false;
}

static bool get_associated_token_address(const uint8_t *mint,
                                         const uint8_t *owner,
                                         uint8_t *address) {
  const uint8_t count = 3;    ///< owner + token_program_id + mint
  uint8_t seed[count][SOLANA_ACCOUNT_ADDRESS_LENGTH];

  memcpy(seed[0], owner, SOLANA_ACCOUNT_ADDRESS_LENGTH);
  hex_string_to_byte_array(
      SOLANA_TOKEN_PROGRAM_ADDRESS, SOLANA_ACCOUNT_ADDRESS_LENGTH * 2, seed[1]);
  memcpy(seed[2], mint, SOLANA_ACCOUNT_ADDRESS_LENGTH);

  uint8_t prog_addr[SOLANA_ACCOUNT_ADDRESS_LENGTH] = {0};
  hex_string_to_byte_array(
      SOLANA_ASSOCIATED_TOKEN_PROGRAM_ADDRESS, 64, prog_addr);
  if (!find_program_address(seed, count, prog_addr, address)) {
    return false;
  }

  return true;
}

static bool is_token_whitelisted(const uint8_t *address,
                                 const solana_token_program_t **contract) {
  const solana_token_program_t *match = NULL;
  bool status = false;
  for (int16_t i = 0; i < SOLANA_WHITELISTED_TOKEN_PROGRAM_COUNT; i++) {
    if (memcmp(address,
               solana_token_program[i].address,
               SOLANA_ACCOUNT_ADDRESS_LENGTH) == 0) {
      match = &solana_token_program[i];
      status = true;
      break;
    }
  }

  if (NULL != contract) {
    *contract = match;
  }

  // return empty contract if not found in the whitelist
  if (!status) {
    static solana_token_program_t empty_contract = {
        .symbol = "",
        .decimal = 0,
    };
    *contract = &empty_contract;
  }

  return status;
}

static bool verify_solana_transfer_token_transaction() {
  const uint8_t transfer_instruction_index =
      solana_txn_context->extra_data.transfer_instruction_index;

  const uint8_t *token_mint = solana_txn_context->transaction_info
                                  .instruction[transfer_instruction_index]
                                  .program.transfer_checked.token_mint;
  solana_token_program_t contract = {0};
  const solana_token_program_t *contract_pointer = &contract;
  if (!is_token_whitelisted(token_mint, &contract_pointer)) {
    // Contract Unverifed, Display warning
    delay_scr_init(ui_text_unverified_token, DELAY_TIME);

    char mint_address[45] = {0};
    size_t mint_address_size = sizeof(mint_address);
    // verify mint address
    if (!b58enc(mint_address,
                &mint_address_size,
                token_mint,
                SOLANA_ACCOUNT_ADDRESS_LENGTH)) {
      solana_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
      return false;
    }

    if (!core_scroll_page(
            ui_text_verify_token_address, mint_address, solana_send_error)) {
      return false;
    }

    const uint8_t token_decimals = solana_txn_context->transaction_info
                                       .instruction[transfer_instruction_index]
                                       .program.transfer_checked.decimals;

    solana_token_program_t empty_contract = {
        .symbol = "",
        .decimal = token_decimals,
    };

    memcpy(&contract, &empty_contract, sizeof(empty_contract));

  } else {
    memcpy(&contract, contract_pointer, sizeof(contract));

    char msg[100] = "";
    snprintf(msg,
             sizeof(msg),
             UI_TEXT_SEND_TOKEN_PROMPT,
             contract.symbol,
             SOLANA_NAME);
    if (!core_confirmation(msg, solana_send_error)) {
      return false;
    }
  }

  // verify recipient address;
  char address[45] = {0};
  size_t address_size = sizeof(address);
  if (!b58enc(address,
              &address_size,
              solana_txn_context->token_data.recipient_address,
              SOLANA_ACCOUNT_ADDRESS_LENGTH)) {
    solana_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  // Calculate associated token address and compare with utxn's value
  uint8_t associated_token_address[SOLANA_ACCOUNT_ADDRESS_LENGTH] = {0};
  if (!get_associated_token_address(
          token_mint,
          solana_txn_context->token_data.recipient_address,
          associated_token_address)) {
    solana_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  if (memcmp(associated_token_address,
             solana_txn_context->transaction_info
                 .instruction[transfer_instruction_index]
                 .program.transfer_checked.destination,
             SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0) {
    solana_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG, 2);
    return false;
  }

  if (use_signature_verification) {
    if (!exchange_validate_stored_signature(address, sizeof(address))) {
      return false;
    }
  }

  // Now take user verification
  if (!core_scroll_page(ui_text_verify_address, address, solana_send_error)) {
    return false;
  }

  // verify recipient amount
  char amount_string[40] = {'\0'}, amount_decimal_string[30] = {'\0'};
  char display[100] = "";

  uint8_t be_units[8] = {0};
  int i = 8;
  while (i--)
    be_units[i] = solana_txn_context->transaction_info
                      .instruction[transfer_instruction_index]
                      .program.transfer_checked.amount >>
                  8 * (7 - i);

  byte_array_to_hex_string(be_units, 8, amount_string, sizeof(amount_string));
  if (!convert_byte_array_to_decimal_string(16,
                                            contract.decimal,
                                            amount_string,
                                            amount_decimal_string,
                                            sizeof(amount_decimal_string))) {
    solana_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    return false;
  }

  snprintf(display,
           sizeof(display),
           UI_TEXT_VERIFY_AMOUNT,
           amount_decimal_string,
           contract.symbol);
  if (!core_confirmation(display, solana_send_error)) {
    return false;
  }

  if (!verify_priority_fee())
    return false;

  set_app_flow_status(SOLANA_SIGN_TXN_STATUS_VERIFY);
  return true;
}

STATIC bool solana_get_user_verification() {
  if (solana_txn_context->is_token_transfer_transaction == true) {
    return verify_solana_transfer_token_transaction();
  } else
    return verify_solana_transfer_sol_transaction();
}

STATIC bool fetch_seed(solana_query_t *query, uint8_t *seed_out) {
  if (!solana_get_query(query, SOLANA_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, SOLANA_SIGN_TXN_REQUEST_VERIFY_TAG)) {
    return false;
  }

  if (!reconstruct_seed(solana_txn_context->init_info.wallet_id,
                        seed_out,
                        solana_send_error)) {
    memzero(seed_out, sizeof(seed_out));
    return false;
  }
  set_app_flow_status(SOLANA_SIGN_TXN_STATUS_SEED_GENERATED);
  send_response(SOLANA_SIGN_TXN_RESPONSE_VERIFY_TAG);
  return true;
}

static bool send_signature(solana_query_t *query,
                           uint8_t *seed,
                           solana_sign_txn_signature_response_t *sig) {
  HDNode hdnode = {0};
  const size_t depth = solana_txn_context->init_info.derivation_path_count;
  const uint32_t *hd_path = solana_txn_context->init_info.derivation_path;

  solana_result_t result = init_solana_result(SOLANA_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = SOLANA_SIGN_TXN_RESPONSE_SIGNATURE_TAG;
  if (!solana_get_query(query, SOLANA_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, SOLANA_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  // recieve latest blockhash
  uint8_t solana_latest_blockhash[SOLANA_BLOCKHASH_LENGTH] = {0};
  memcpy(solana_latest_blockhash,
         query->sign_txn.signature.blockhash,
         SOLANA_BLOCKHASH_LENGTH);

  // update unsigned transaction with latest blockhash
  int update_status = solana_update_blockhash_in_byte_array(
      solana_txn_context->transaction, solana_latest_blockhash);
  if (update_status != SOL_OK)
    return false;

  // sign updated transaction
  if (!derive_hdnode_from_path(hd_path, depth, ED25519_NAME, seed, &hdnode))
    return false;

  ed25519_sign(solana_txn_context->transaction,
               solana_txn_context->init_info.transaction_size,
               hdnode.private_key,
               hdnode.public_key + 1,
               sig->signature);

  memzero(&hdnode, sizeof(hdnode));
  memzero(seed, sizeof(seed));

  memcpy(&result.sign_txn.signature,
         sig,
         sizeof(solana_sign_txn_signature_response_t));

  solana_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void solana_sign_transaction(solana_query_t *query) {
  solana_txn_context =
      (solana_txn_context_t *)malloc(sizeof(solana_txn_context_t));
  memzero(solana_txn_context, sizeof(solana_txn_context_t));
  solana_sign_txn_signature_response_t sig = {0};
  uint8_t seed[64] = {0};

  if (solana_handle_initiate_query(query) &&
      solana_fetch_valid_transaction(query) && solana_get_user_verification() &&
      fetch_seed(query, seed) && send_signature(query, seed, &sig)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  memzero(seed, sizeof(seed));

  if (NULL != solana_txn_context->transaction) {
    free(solana_txn_context->transaction);
  }
  if (NULL != solana_txn_context) {
    free(solana_txn_context);
    solana_txn_context = NULL;
  }
}
