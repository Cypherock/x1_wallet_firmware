/**
 * @file    cardano_txn.c
 * @author  Cypherock X1 Team
 * @brief   Implementations related to cardano transactions
 * @copyright Copyright (c) 2026 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2026 by HODL TECH PTE LTD
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bip32.h"
#include "cardano/core.pb.h"
#include "cardano/sign_txn.pb.h"
#include "cardano_api.h"
#include "cardano_context.h"
#include "cardano_helpers.h"
#include "cardano_priv.h"
#include "cbor.h"
#include "cbor/arrays.h"
#include "cbor/common.h"
#include "cbor/data.h"
#include "cbor/ints.h"
#include "cbor/maps.h"
#include "cbor/strings.h"
#include "coin_utils.h"
#include "common.pb.h"
#include "composable_app_queue.h"
#include "constant_texts.h"
#include "ed25519.h"
#include "error.pb.h"
#include "exchange_main.h"
#include "memzero.h"
#include "reconstruct_wallet_flow.h"
#include "segwit_addr.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_delay.h"
#include "wallet.h"
#include "wallet_list.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

cardano_txn_context_t *cardano_txn_context = NULL;

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

#define CBOR_TXN_BODY_OUTPUTS_KEY_PAIR_ID 1
#define CBOR_TXN_BODY_FEES_KEY_PAIR_ID 2

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
 * @param query Reference to an instance of cardano_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const cardano_query_t *query,
                                pb_size_t which_request)
    __attribute__((warn_unused_result));

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
static bool validate_request_data(const cardano_sign_txn_request_t *request);

/**
 * @brief The function prepares and sends empty responses
 *
 * @param which_response Constant value for the response type to be sent
 */
static void send_response(const pb_size_t which_response);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the CARDANO_SIGN_TXN
 * type of request. Additionally, the wallet-id is validated for sanity and the
 * derivation path for the account is also validated. After the validations,
 * user is prompted about the action for confirmation. The function returns true
 * indicating all the validation and user confirmation was a success. The
 * function also duplicates the data from query into the cardano_txn_context for
 * further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
static bool handle_initiate_query(const cardano_query_t *query)
    __attribute__((warn_unused_result));

/**
 * @brief Parses cbor encoded raw cardano transaction
 *
 * @param txn[in]              Raw cbor encoded transaction
 * @param txn_size[in]         Transaction buffer size
 * @param out_parsed_txn[out]  Reference to @ref cardano_parsed_txn where parsed
 * txn will be stored
 *
 * @returns Bool Indicating whether succeeded or failed
 */
static bool cardano_parse_txn_from_cbor(uint8_t *txn,
                                        size_t txn_size,
                                        cardano_txn_context_t *out_txn_context)
    __attribute__((warn_unused_result));

/**
 * @brief Fetches complete raw transaction to be signed for verification
 * @details The function will try to fetch the transaction by referring to the
 * declared size in cardano_txn_context. The function will store complete
 * transaction into cardano_txn_context.transaction.
 *
 * @param query Reference to an instance of cardano_query_t for storing the
 * transient transaction chunks.
 *
 * @return bool Indicating if the whole transaction received and verified
 * @retval true If all the transaction was fetched and verified
 * @retval false If the transaction failed verification or wasn't fetched
 */
static bool fetch_valid_transaction(cardano_query_t *query)
    __attribute__((warn_unused_result));

/**
 * @brief Aggregates user consent for the transaction info
 * @details The function decodes the receiver address along with the
 * corresponding transfer value in TRX.
 *
 *
 * @return bool Indicating if the user confirmed the transaction
 * @retval true If user confirmed the receiver address along with
 * the corresponding value.
 * @retval false Immediate return if any of the confirmation is disapproved
 */
static bool get_user_verification() __attribute__((warn_unused_result));

/**
 * @brief Fetches seed for generating public and private key
 * @details The function internally calls wallet reconstruction sub-flow to get
 * access to the seed.
 *
 * @param query Reference to an instance of cardano_query_t to store transient
 * request from the host
 * @param seed_out seed recontructed from wallet
 * @return bool Indicating if signature for the provided transaction was
 * successfully generated
 * @retval true If all the seed is generated without any error
 * @retval false If seed failed to generate
 */
static bool fetch_seed(cardano_query_t *query, uint8_t *seed_out)
    __attribute__((warn_unused_result));

/**
 * @brief Sends the generated signature to the host
 * @details The function internally updates the unsigned transaction with a
 * recent blockhash and signs the transaction before sending to the host
 *
 * @param query Reference to an instance of cardano_query_t to store transient
 * request from the host
 * @param seed seed reconstructed from wallet
 * @param sig Reference to the struct having the signature components
 * @return bool Indicating if the signature is sent to the host
 * @retval true If the signature was sent to host successfully
 * @retval false If the host responded with unknown/wrong query
 */
static bool send_signature(cardano_query_t *query,
                           uint8_t *seed,
                           cardano_sign_txn_signature_response_t *sig)
    __attribute__((warn_unused_result));

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

bool use_signature_verification = false;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const cardano_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    cardano_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request_data(const cardano_sign_txn_request_t *request) {
  bool status = true;

  if (!cardano_derivation_path_guard(request->initiate.derivation_path,
                                     request->initiate.derivation_path_count)) {
    cardano_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
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
  cardano_result_t result = init_cardano_result(CARDANO_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  cardano_send_result(&result);
}

static bool handle_initiate_query(const cardano_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  /*
   * TODO: handle wallet search failure conditions like wallet not found, locked
   * wallet.
   */
  if (!check_which_request(query, CARDANO_SIGN_TXN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->sign_txn) ||
      !get_wallet_name_by_id(query->sign_txn.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             cardano_send_error)) {
    return false;
  }

  int32_t ignored = snprintf(
      msg, sizeof(msg), UI_TEXT_SIGN_TXN_PROMPT, CARDANO_NAME, wallet_name);
  (void)ignored;

  /* take user consent */
  if (!core_confirmation(msg, cardano_send_error)) {
    return false;
  }

  set_app_flow_status(CARDANO_SIGN_TXN_STATUS_CONFIRM);
  memcpy(&cardano_txn_context->init_info,
         &query->sign_txn.initiate,
         sizeof(cardano_sign_txn_initiate_request_t));

  send_response(CARDANO_SIGN_TXN_RESPONSE_CONFIRMATION_TAG);

  /* show processing screen for a short duration */
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool cardano_parse_txn_from_cbor(
    uint8_t *txn,
    size_t txn_size,
    cardano_txn_context_t *out_txn_context) {
  struct cbor_load_result result;
  cbor_item_t *cbor_decoded_raw_txn = cbor_load(txn, txn_size, &result);

  if (CBOR_ERR_NONE != result.error.code) {
    logger("cborerror-1");
    return false;
  }

  /* make sure parent is array and has atleast 1 element */
  if (!cbor_isa_array(cbor_decoded_raw_txn) ||
      cbor_array_size(cbor_decoded_raw_txn) <= 0) {
    logger("cborerror-2");
    return false;
  }

  /* get txn body, the first element */
  cbor_item_t *txn_body_cbor = cbor_array_get(cbor_decoded_raw_txn, 0);

  /* extract and store fields */

  /* should be a map */
  if (!cbor_isa_map(txn_body_cbor)) {
    logger("cborerror-3");
    return false;
  }

  size_t txn_body_key_value_count = cbor_map_size(txn_body_cbor);

  /* should have at least 4 elements */
  if (4 > txn_body_key_value_count) {
    logger("cborerror-4");
    return false;
  }

  /* txn body key value pairs */
  struct cbor_pair *txn_body_key_value = cbor_map_handle(txn_body_cbor);

  for (size_t i = 0; i < txn_body_key_value_count; i++) {
    /* get key value pair */
    struct cbor_pair *key_pair = &txn_body_key_value[i];

    /* key is always a uint */
    if (!cbor_isa_uint(key_pair->key)) {
      logger("cborerror-5");
      return false;
    }

    uint64_t key_value_pair_id = cbor_get_int(key_pair->key);
    switch (key_value_pair_id) {
        /* outputs */
      case CBOR_TXN_BODY_OUTPUTS_KEY_PAIR_ID: {
        /* should be an array of outputs */
        if (!cbor_isa_array(key_pair->value)) {
          logger("cborerror-6");
          return false;
        }

        size_t outputs_size = cbor_array_size(key_pair->value);

        /* there should be atleast 1 output */
        if (0 >= outputs_size) {
          logger("cborerror-7");
          return false;
        }

        /* for each output in outputs */
        for (size_t output_i = 0; output_i < outputs_size; output_i++) {
          cbor_item_t *output = cbor_array_get(key_pair->value, output_i);

          /* each output is a array of 2 elements */
          if (!cbor_isa_array(output) || cbor_array_size(output) != 2) {
            logger("cborerror-8");
            return false;
          }

          /* first element is a byte string of raw address of not more than
           * length 29 */
          cbor_item_t *raw_addr_wrapped = cbor_array_get(output, 0);
          if (!cbor_isa_bytestring(raw_addr_wrapped) ||
              29 < cbor_string_length(raw_addr_wrapped)) {
            logger("cborerror-9");
            return false;
          }

          uint8_t *raw_addr = cbor_string_handle(raw_addr_wrapped);

          /* blake2b hash the raw address */
          uint8_t address[60] = {0};
          blake2b(raw_addr, 29, address, 28);

          /* convert to 5 bit symbols */
          uint8_t raw_addr_5bit[64];
          size_t raw_addr_5bit_len = 0;
          if (!convert_bits_bech32(
                  raw_addr_5bit, &raw_addr_5bit_len, raw_addr, 29)) {
            logger("cborerror-9.1");
            return false;
          }

          char display[30] = {0};
          snprintf(display, 30, "len = %d", raw_addr_5bit_len);
          logger("%s", display);

          /* bech32 encode */
          if (!bech32_encode((char *)&out_txn_context->parsed_txn.receiver_addr,
                             PAYMENT_BECH32_PREFIX,
                             raw_addr_5bit,
                             raw_addr_5bit_len)) {
            logger("cborerror-10");
            return false;
          }

          /* amount */
          cbor_item_t *sending_amount = cbor_array_get(output, 1);
          if (!cbor_isa_uint(sending_amount)) {
            logger("cborerror-11");
            return false;
          }

          out_txn_context->parsed_txn.receive_amount =
              cbor_get_int(sending_amount);

          memzero(display, sizeof(display));
          snprintf(display,
                   30,
                   "amount = %llu",
                   out_txn_context->parsed_txn.receive_amount);
          logger("%s", display);
        }
        break;
      }

        /* fees  */
      case CBOR_TXN_BODY_FEES_KEY_PAIR_ID: {
        if (!cbor_isa_uint(key_pair->value)) {
          logger("cborerror-12");
          return false;
        }

        char display[30] = {0};
        snprintf(display, 30, "fees = %llu", cbor_get_int(key_pair->value));
        logger("%s", display);
        out_txn_context->parsed_txn.fees = cbor_get_int(key_pair->value);
        break;
      }
      default:
        logger("id = %ld", key_value_pair_id);
        break;
    }
  }

  /* canonical reencode */
  unsigned char *serialized_txn_body = 0;
  size_t serialized_txn_body_len = 0;
  cbor_serialize_alloc(
      txn_body_cbor, &serialized_txn_body, &serialized_txn_body_len);

  /* libcbor failed to allocate */
  if (0 >= serialized_txn_body_len) {
    logger("cborerror-13");
    return false;
  }

  /* hash */
  blake2b(serialized_txn_body,
          serialized_txn_body_len,
          out_txn_context->transaction_hash,
          CARDANO_HASHED_TXN_SIZE);

  /* free serialized transaction body */
  free(serialized_txn_body);
  return true;
}

static bool fetch_valid_transaction(cardano_query_t *query) {
  uint32_t total_txn_size = cardano_txn_context->init_info.transaction_size;
  const cardano_sign_txn_data_t *txn_data = &query->sign_txn.txn_data;
  const common_chunk_payload_t *payload = &txn_data->chunk_payload;
  const common_chunk_payload_chunk_t *chunk = &txn_data->chunk_payload.chunk;

  uint32_t read_size = 0;
  cardano_result_t response = init_cardano_result(CARDANO_RESULT_SIGN_TXN_TAG);
  /* allocate mem for storing entire trnsaction */
  cardano_txn_context->transaction = (uint8_t *)malloc(total_txn_size);
  logger("txn size = %d\n", total_txn_size);

  for (;;) {
    /* invalid request? */
    if (!cardano_get_query(query, CARDANO_QUERY_SIGN_TXN_TAG) ||
        !check_which_request(query, CARDANO_SIGN_TXN_REQUEST_TXN_DATA_TAG)) {
      return false;
    }

    /* data probably got corrupt */
    if (!txn_data->has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        read_size + payload->chunk.size > total_txn_size) {
      cardano_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    /* copy to internal buffer */
    memcpy(&cardano_txn_context->transaction[read_size],
           chunk->bytes,
           chunk->size);
    read_size += chunk->size;

    /* send ack */
    response.sign_txn.which_response =
        CARDANO_SIGN_TXN_RESPONSE_DATA_ACCEPTED_TAG;
    response.sign_txn.data_accepted.has_chunk_ack = true;
    response.sign_txn.data_accepted.chunk_ack.chunk_index =
        payload->chunk_index;
    cardano_send_result(&response);

    /* reached the end of payload */
    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }

  /* if read size is not total size, data probably got corrupt */
  if (read_size != total_txn_size) {
    cardano_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_DATA);

    return false;
  }

  logger("first = %d\n", cardano_txn_context->transaction[0]);
  logger("last = %d\n", cardano_txn_context->transaction[total_txn_size - 1]);
  if (!cardano_parse_txn_from_cbor(cardano_txn_context->transaction,
                                   total_txn_size,
                                   cardano_txn_context)) {
    logger("unique2-error4");
    return false;
  }

  // TODO: validate transaction
  return true;
}

static bool get_user_verification() {
  logger("unique3");
  /* verify amount, addr for each receipt */
  char to_address[CARDANO_PAYMENT_ADDR_LENGTH + 1] = {0};
  memcpy(to_address,
         cardano_txn_context->parsed_txn.receiver_addr,
         CARDANO_PAYMENT_ADDR_LENGTH);

  if (!core_scroll_page(
          ui_text_verify_address, to_address, cardano_send_error)) {
    return false;
  }

  char amount_string[30] = {'\0'};
  double amount_ada =
      ada_from_lovelace(cardano_txn_context->parsed_txn.receive_amount);
  int32_t ignored =
      snprintf(amount_string, sizeof(amount_string), "%.6f", amount_ada);

  char display[100] = {'\0'};
  ignored = snprintf(display,
                     sizeof(display),
                     UI_TEXT_VERIFY_AMOUNT,
                     amount_string,
                     CARDANO_LUNIT);
  (void)ignored;

  if (!core_confirmation(display, cardano_send_error)) {
    return false;
  }

  /* verify fees */
  char fees_string[30] = {0};
  double fees_ada = ada_from_lovelace(cardano_txn_context->parsed_txn.fees);
  memzero(display, sizeof(display));
  ignored = snprintf(fees_string, sizeof(fees_string), "%.6f", fees_ada);
  (void)ignored;
  ignored = snprintf(display, sizeof(display), "Verify fees\n%s", fees_string);
  (void)ignored;

  if (!core_confirmation(display, cardano_send_error)) {
    return false;
  }

  set_app_flow_status(CARDANO_SIGN_TXN_STATUS_VERIFY);
  return true;
}

static bool fetch_seed(cardano_query_t *query, uint8_t *seed_out) {
  logger("unique4");
  if (!cardano_get_query(query, CARDANO_QUERY_SIGN_TXN_TAG)) {
    return false;
  }

  if (!reconstruct_seed(cardano_txn_context->init_info.wallet_id,
                        seed_out,
                        cardano_send_error)) {
    memzero(seed_out, sizeof(seed_out));
    return false;
  }

  set_app_flow_status(CARDANO_SIGN_TXN_STATUS_SEED_GENERATED);
  return true;
}

static bool send_signature(cardano_query_t *query,
                           uint8_t *seed,
                           cardano_sign_txn_signature_response_t *sig) {
  logger("unique5");
  HDNode hdnode = {0};
  const size_t depth = cardano_txn_context->init_info.derivation_path_count;
  const uint32_t *hd_path = cardano_txn_context->init_info.derivation_path;

  cardano_result_t result = init_cardano_result(CARDANO_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = CARDANO_SIGN_TXN_RESPONSE_SIGNATURE_TAG;
  if (!cardano_get_query(query, CARDANO_QUERY_SIGN_TXN_TAG) ||
      !check_which_request(query, CARDANO_SIGN_TXN_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  /* derive keys */
  if (!derive_hdnode_from_path_cardano(hd_path, depth, seed, &hdnode)) {
    return false;
  }

  /* sign */
  hdnode_fill_public_key(&hdnode);
  ed25519_sign(cardano_txn_context->transaction_hash,
               CARDANO_HASHED_TXN_SIZE,
               hdnode.private_key,
               hdnode.public_key,
               sig->signature);

  memzero(&hdnode, sizeof(hdnode));
  memzero(seed, sizeof(seed));

  memcpy(&result.sign_txn.signature,
         sig,
         sizeof(cardano_sign_txn_signature_response_t));

  cardano_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void cardano_sign_transaction(cardano_query_t *query) {
  cardano_txn_context =
      (cardano_txn_context_t *)malloc(sizeof(cardano_txn_context_t));
  memzero(cardano_txn_context, sizeof(cardano_txn_context_t));

  cardano_sign_txn_signature_response_t sig = {0};
  uint8_t seed[64] = {0};

  if (handle_initiate_query(query) && fetch_valid_transaction(query) &&
      get_user_verification() && fetch_seed(query, seed) &&
      send_signature(query, seed, &sig)) {
    logger("unique6");
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  memzero(seed, sizeof(seed));

  if (NULL != cardano_txn_context->transaction) {
    free(cardano_txn_context->transaction);
    cardano_txn_context->transaction = NULL;
  }

  if (NULL != cardano_txn_context) {
    free(cardano_txn_context);
    cardano_txn_context = NULL;
  }
}
