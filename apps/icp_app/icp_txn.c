/**
 * @file    icp_txn.c
 * @author  Cypherock X1 Team
 * @brief   Source file to handle transaction signing logic for ICP protocol
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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "base58.h"
#include "composable_app_queue.h"
#include "constant_texts.h"
#include "exchange_main.h"
#include "icp/sign_txn.pb.h"
#include "icp_api.h"
#include "icp_context.h"
#include "icp_helpers.h"
#include "icp_priv.h"
#include "icp_txn_helpers.h"
#include "icrc_canisters.h"
#include "reconstruct_wallet_flow.h"
#include "sha2.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "utils.h"
#include "wallet_list.h"

/*****************************************************************************
 * EXTERN AND GLOBAL VARIABLES
 *****************************************************************************/
#define ICP_DOMAIN_SEPARATOR_LEN 11
// "\x0Aic-request"
uint8_t domain_separator[ICP_DOMAIN_SEPARATOR_LEN] =
    {10, 105, 99, 45, 114, 101, 113, 117, 101, 115, 116};
/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef icp_sign_txn_signature_response_t sig_t;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host bitcoin app and return false.
 *
 * @param query Reference to an instance of icp_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const icp_query_t *query,
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
 * @param request Reference to an instance of icp_sign_txn_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(const icp_sign_txn_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * ICP_SIGN_TXN_REQUEST_INITIATE_TAG type of request. Additionally, the
 * wallet-id is validated for sanity and the derivation path for the account is
 * also validated. After the validations, user is prompted about the action for
 * confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the icp_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const icp_query_t *query);

/**
 * @brief Receives unsigned txn from the host. If reception is successful, it
 * also parses the txn to ensure it's validity.
 * @note In case of any failure, a corresponding message is conveyed to the host
 *
 * @param query Reference to buffer of type icp_query_t
 * @return true If the txn is received in the internal buffers and is valid
 * @return false If the txn could not be received or it's validation failed
 */
static bool fetch_valid_input(icp_query_t *query);

/**
 * @brief This function executes user verification flow of the unsigned txn
 * received from the host.
 * @details The user verification flow is different for different type of
 * transactions.
 * @note This function expected that the unsigned txn is parsed using the helper
 * function.
 *
 * @return true If the user accepted the transaction display
 * @return false If any user rejection occured or P0 event occured
 */
static bool get_user_verification(void);

/**
 * @brief This function executes user verification flow of the unsigned coin txn
 * received from the host.
 * @note This function expected that the unsigned txn is parsed using the helper
 * function.
 *
 * @return true If the user accepted the transaction display
 * @return false If any user rejection occured or P0 event occured
 */
static bool get_user_verification_for_coin_txn(void);

/**
 * @brief This function executes user verification flow of the unsigned token
 * txn received from the host.
 * @note This function expected that the unsigned txn is parsed using the helper
 * function.
 *
 * @return true If the user accepted the transaction display
 * @return false If any user rejection occured or P0 event occured
 */
static bool get_user_verification_for_token_txn(void);

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
static bool sign_txn(sig_t *signature);

/**
 * @brief Sends signature of the ICP unsigned txn to the host
 * @details The function waits for the host to send a request of type
 * ICP_SIGN_TXN_REQUEST_SIGNATURE_TAG and sends the response
 *
 * @param query Reference to buffer of type icp_query_t
 * @param signature Reference to signature to be sent to the host
 * @return true If the signature was sent successfully
 * @return false If the signature could not be sent - maybe due to and P0 event
 * or invalid request received from the host
 */
static bool send_signature(icp_query_t *query, const sig_t *signature);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static icp_txn_context_t *icp_txn_context = NULL;
static bool use_signature_verification = false;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool check_which_request(const icp_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    icp_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static void send_response(const pb_size_t which_response) {
  icp_result_t result = init_icp_result(ICP_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  icp_send_result(&result);
}

static bool validate_request_data(const icp_sign_txn_request_t *request) {
  bool status = true;

  if (!icp_derivation_path_guard(request->initiate.derivation_path,
                                 request->initiate.derivation_path_count)) {
    // TODO: add proper specific errors
    icp_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
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

static bool handle_initiate_query(const icp_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  // TODO: Handle wallet search failures - eg: Wallet ID not found, Wallet
  // ID found but is invalid/locked wallet
  if (!check_which_request(query, ICP_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             icp_send_error)) {
    return false;
  }

  snprintf(msg, sizeof(msg), UI_TEXT_SIGN_TXN_PROMPT, ICP_NAME, wallet_name);
  // Take user consent to sign transaction for the wallet
  if (!core_confirmation(msg, icp_send_error)) {
    return false;
  }

  set_app_flow_status(ICP_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&icp_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(icp_sign_txn_initiate_request_t));

  send_response(ICP_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);
  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool fetch_valid_input(icp_query_t *query) {
  if (!icp_get_query(query, ICP_QUERY_SIGN_TXN_TAG) &&
      !check_which_request(query, ICP_SIGN_TXN_REQUEST_TXN_DATA_TAG)) {
    return false;
  }

  const icp_sign_txn_data_t *txn_data = &query->sign_txn.txn_data;
  if (txn_data->has_icp_transfer_req == false) {
    return false;
  }

  icp_txn_context->icp_transfer_req = &txn_data->icp_transfer_req;

  // Verify request_type = "call"
  if (memcmp(icp_txn_context->icp_transfer_req->request_type, "call", 4) != 0) {
    return false;
  }

  // Verify method_name = "transfer" or "icrc1_transfer"
  if (memcmp(icp_txn_context->icp_transfer_req->method_name.bytes,
             "transfer",
             8) != 0 &&
      memcmp(icp_txn_context->icp_transfer_req->method_name.bytes,
             "icrc1_transfer",
             14) != 0) {
    return false;
  }

  send_response(ICP_SIGN_TXN_RESPONSE_DATA_ACCEPTED_TAG);

  icp_txn_context->is_token_transfer_txn =
      memcmp(icp_txn_context->icp_transfer_req->method_name.bytes,
             "icrc1_transfer",
             14) == 0;

  if (icp_txn_context->is_token_transfer_txn) {
    icp_txn_context->raw_icp_token_transfer_txn =
        (icp_token_transfer_t *)malloc(sizeof(icp_token_transfer_t));
  } else {
    icp_txn_context->raw_icp_coin_transfer_txn =
        (icp_coin_transfer_t *)malloc(sizeof(icp_coin_transfer_t));
  }

  if (!icp_parse_transfer_txn(icp_txn_context->icp_transfer_req->arg.bytes,
                              icp_txn_context->icp_transfer_req->arg.size,
                              icp_txn_context)) {
    icp_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  return true;
}

static bool is_token_whitelisted(const uint8_t *cansister_id,
                                 size_t canister_id_len,
                                 icrc_token_t *token) {
  const icrc_token_t *match = NULL;
  bool status = false;

  for (int16_t i = 0; i < ICRC_WHITELISTED_TOKEN_COUNT; i++) {
    if (canister_id_len == sizeof(icrc_tokens[i].ledger_canister_id) &&
        memcmp(cansister_id,
               icrc_tokens[i].ledger_canister_id,
               canister_id_len) == 0) {
      match = &icrc_tokens[i];
      status = true;
      break;
    }
  }

  if (NULL != token && NULL != match) {
    memcpy(token, match, sizeof(*match));
  }

  // return unknown empty token if not found in the whitelist
  if (!status) {
    icrc_token_t empty_token = {
        .ledger_canister_id = {},
        .symbol = "",
        .decimal = 0,
    };

    memcpy(token, &empty_token, sizeof(empty_token));
  }

  return status;
}

static bool get_user_verification_for_token_txn(void) {
  const uint8_t *canister_id =
      icp_txn_context->icp_transfer_req->canister_id.bytes;
  size_t canister_id_len = icp_txn_context->icp_transfer_req->canister_id.size;

  icrc_token_t token = {0};
  if (!is_token_whitelisted(canister_id, canister_id_len, &token)) {
    // Token Unverifed, Display warning
    delay_scr_init(ui_text_unverified_token, DELAY_TIME);

    char canister_principal_id[200] = {0};
    get_principal_id_to_display(
        canister_id, canister_id_len, canister_principal_id);

    if (!core_scroll_page(ui_text_verify_token_address,
                          canister_principal_id,
                          icp_send_error)) {
      return false;
    }

  } else {
    char msg[100] = "";
    snprintf(
        msg, sizeof(msg), UI_TEXT_SEND_TOKEN_PROMPT, token.symbol, ICP_NAME);
    if (!core_confirmation(msg, icp_send_error)) {
      return false;
    }
  }

  const icp_token_transfer_t *decoded_utxn =
      icp_txn_context->raw_icp_token_transfer_txn;

  // verify recipient principal id;
  char principal_id[200] = {0};
  get_principal_id_to_display(
      decoded_utxn->to.owner, ICP_PRINCIPAL_LENGTH, principal_id);

  if (use_signature_verification) {
    if (!exchange_validate_stored_signature(principal_id,
                                            sizeof(principal_id))) {
      return false;
    }
  }

  // Now take user verification
  if (!core_scroll_page(
          ui_text_verify_principal_id, principal_id, icp_send_error)) {
    return false;
  }

  // verify recipient amount
  char amount_string[40] = {'\0'}, amount_decimal_string[30] = {'\0'};
  char display_amount[100] = "";

  uint8_t be_amount[8] = {0};
  int amount_index = 8;
  while (amount_index--)
    be_amount[amount_index] = decoded_utxn->amount >> 8 * (7 - amount_index);

  byte_array_to_hex_string(be_amount, 8, amount_string, sizeof(amount_string));
  if (!convert_byte_array_to_decimal_string(16,
                                            token.decimal,
                                            amount_string,
                                            amount_decimal_string,
                                            sizeof(amount_decimal_string))) {
    icp_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    return false;
  }

  snprintf(display_amount,
           sizeof(display_amount),
           UI_TEXT_VERIFY_AMOUNT,
           amount_decimal_string,
           token.symbol);
  if (!core_confirmation(display_amount, icp_send_error)) {
    return false;
  }

  // verify fee
  if (decoded_utxn->has_fee) {
    char fee_string[40] = {'\0'}, fee_decimal_string[30] = {'\0'};
    char display_fee[100] = "";

    uint8_t be_fee[8] = {0};
    int fee_index = 8;
    while (fee_index--)
      be_fee[fee_index] = decoded_utxn->fee >> 8 * (7 - fee_index);

    byte_array_to_hex_string(be_fee, 8, fee_string, sizeof(fee_string));
    if (!convert_byte_array_to_decimal_string(16,
                                              token.decimal,
                                              fee_string,
                                              fee_decimal_string,
                                              sizeof(fee_decimal_string))) {
      icp_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
      return false;
    }

    snprintf(display_fee,
             sizeof(display_fee),
             UI_TEXT_VERIFY_FEE,
             fee_decimal_string,
             token.symbol);

    if (!core_confirmation(display_fee, icp_send_error)) {
      return false;
    }
  }

  // Verfy memo
  if (decoded_utxn->has_memo) {
    char memo_string[decoded_utxn->memo.size * 2 + 1];
    byte_array_to_hex_string(decoded_utxn->memo.bytes,
                             decoded_utxn->memo.size,
                             memo_string,
                             sizeof(memo_string));

    char display_memo[50 + decoded_utxn->memo.size * 2 + 1];
    snprintf(
        display_memo, sizeof(display_memo), UI_TEXT_VERIFY_MEMO, memo_string);

    if (!core_confirmation(display_memo, icp_send_error)) {
      return false;
    }
  }

  set_app_flow_status(ICP_SIGN_TXN_STATUS_VERIFY);
  return true;
}

static bool get_user_verification_for_coin_txn(void) {
  const icp_coin_transfer_t *decoded_utxn =
      icp_txn_context->raw_icp_coin_transfer_txn;

  char to_account_id[ICP_ACCOUNT_ID_LENGTH * 2 + 1] = "";

  byte_array_to_hex_string(decoded_utxn->to,
                           ICP_ACCOUNT_ID_LENGTH,
                           to_account_id,
                           ICP_ACCOUNT_ID_LENGTH * 2 + 1);

  if (use_signature_verification) {
    if (!exchange_validate_stored_signature(to_account_id,
                                            sizeof(to_account_id))) {
      return false;
    }
  }

  if (!core_scroll_page(
          ui_text_verify_account_id, to_account_id, icp_send_error)) {
    return false;
  }

  // verify recipient amount
  uint64_t amount = 0;
  memcpy(&amount, &decoded_utxn->amount.e8s, sizeof(uint64_t));
  char amount_string[30] = {'\0'};
  double decimal_amount = (double)amount;
  decimal_amount *= 1e-8;
  snprintf(amount_string, sizeof(amount_string), "%.*g", 8, decimal_amount);

  char display[100] = {'\0'};
  snprintf(display,
           sizeof(display),
           UI_TEXT_VERIFY_AMOUNT,
           amount_string,
           ICP_LUNIT);

  if (!core_confirmation(display, icp_send_error)) {
    return false;
  }

  // verify transaction fee
  uint64_t fee = 0;
  memcpy(&fee, &decoded_utxn->fee.e8s, sizeof(uint64_t));
  char fee_string[30] = {'\0'};
  double decimal_fee = (double)fee;
  decimal_fee *= 1e-8;
  snprintf(fee_string, sizeof(fee_string), "%.*g", 8, decimal_fee);

  snprintf(display, sizeof(display), UI_TEXT_VERIFY_FEE, fee_string, ICP_LUNIT);

  if (!core_confirmation(display, icp_send_error)) {
    return false;
  }

  // verify memo
  char memo_string[30] = "";
  snprintf(memo_string, sizeof(memo_string), "%llu", decoded_utxn->memo);

  char display_memo[50] = {'\0'};
  snprintf(
      display_memo, sizeof(display_memo), UI_TEXT_VERIFY_MEMO, memo_string);

  if (!core_confirmation(display_memo, icp_send_error)) {
    return false;
  }

  set_app_flow_status(ICP_SIGN_TXN_STATUS_VERIFY);

  return true;
}

static bool get_user_verification(void) {
  if (icp_txn_context->is_token_transfer_txn) {
    return get_user_verification_for_token_txn();
  }
  return get_user_verification_for_coin_txn();
}

static bool sign_txn(sig_t *signature) {
  uint8_t seed[64] = {0};
  if (!reconstruct_seed(
          icp_txn_context->init_info.wallet_id, seed, icp_send_error)) {
    memzero(seed, sizeof(seed));
    // TODO: handle errors of reconstruction flow
    return false;
  }

  set_app_flow_status(ICP_SIGN_TXN_STATUS_SEED_GENERATED);

  uint8_t request_id[SHA256_DIGEST_LENGTH] = {0};
  hash_icp_transfer_request(icp_txn_context->icp_transfer_req, request_id);

  uint8_t result[SHA256_DIGEST_LENGTH + ICP_DOMAIN_SEPARATOR_LEN] = {0};
  memcpy(result, domain_separator, ICP_DOMAIN_SEPARATOR_LEN);
  memcpy(result + ICP_DOMAIN_SEPARATOR_LEN, request_id, SHA256_DIGEST_LENGTH);

  uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw(result, SHA256_DIGEST_LENGTH + ICP_DOMAIN_SEPARATOR_LEN, digest);

  HDNode hdnode = {0};
  derive_hdnode_from_path(icp_txn_context->init_info.derivation_path,
                          icp_txn_context->init_info.derivation_path_count,
                          SECP256K1_NAME,
                          seed,
                          &hdnode);

  ecdsa_sign_digest(&secp256k1,
                    hdnode.private_key,
                    digest,
                    signature->transfer_req_signature,
                    NULL,
                    NULL);

  uint8_t read_state_request_id[SHA256_DIGEST_LENGTH] = {0};
  get_icp_read_state_request_id(read_state_request_id,
                                request_id,
                                sizeof(request_id),
                                icp_txn_context->icp_transfer_req);

  uint8_t read_state_request_result[SHA256_DIGEST_LENGTH +
                                    ICP_DOMAIN_SEPARATOR_LEN] = {0};
  memcpy(read_state_request_result, domain_separator, ICP_DOMAIN_SEPARATOR_LEN);
  memcpy(read_state_request_result + ICP_DOMAIN_SEPARATOR_LEN,
         read_state_request_id,
         SHA256_DIGEST_LENGTH);

  uint8_t read_state_request_digest[SHA256_DIGEST_LENGTH] = {0};
  sha256_Raw(read_state_request_result,
             SHA256_DIGEST_LENGTH + ICP_DOMAIN_SEPARATOR_LEN,
             read_state_request_digest);

  ecdsa_sign_digest(&secp256k1,
                    hdnode.private_key,
                    read_state_request_digest,
                    signature->read_state_req_signature,
                    NULL,
                    NULL);

  memzero(request_id, sizeof(request_id));
  memzero(read_state_request_id, sizeof(read_state_request_id));
  memzero(result, sizeof(result));
  memzero(read_state_request_result, sizeof(read_state_request_result));
  memzero(digest, sizeof(digest));
  memzero(read_state_request_digest, sizeof(read_state_request_digest));
  memzero(seed, sizeof(seed));
  memzero(&hdnode, sizeof(hdnode));

  return true;
}

static bool send_signature(icp_query_t *query, const sig_t *signature) {
  icp_result_t result = init_icp_result(ICP_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = ICP_SIGN_TXN_RESPONSE_SIGNATURE_TAG;

  if (!icp_get_query(query, ICP_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, ICP_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(&result.sign_txn.signature, signature, sizeof(sig_t));

  icp_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void icp_sign_transaction(icp_query_t *query) {
  icp_txn_context = (icp_txn_context_t *)malloc(sizeof(icp_txn_context_t));
  memzero(icp_txn_context, sizeof(icp_txn_context_t));

  sig_t signature = {0};

  if (handle_initiate_query(query) && fetch_valid_input(query) &&
      get_user_verification() && sign_txn(&signature) &&
      send_signature(query, &signature)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  if (icp_txn_context) {
    if (icp_txn_context->raw_icp_coin_transfer_txn) {
      free(icp_txn_context->raw_icp_coin_transfer_txn);
      icp_txn_context->raw_icp_coin_transfer_txn = NULL;
    }
    if (icp_txn_context->raw_icp_token_transfer_txn) {
      free(icp_txn_context->raw_icp_token_transfer_txn);
      icp_txn_context->raw_icp_token_transfer_txn = NULL;
    }
    free(icp_txn_context);
    icp_txn_context = NULL;
  }
}
