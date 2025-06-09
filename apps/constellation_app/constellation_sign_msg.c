/**
 * @file    constellation_sign_msg.c
 * @author  Cypherock X1 Team
 * @brief   Signs messages for CONSTELLATION
 *derivations.
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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "atca_helpers.h"
#include "atca_status.h"
#include "coin_utils.h"
#include "constant_texts.h"
#include "constellation/sign_msg.pb.h"
#include "constellation_api.h"
#include "constellation_context.h"
#include "constellation_helpers.h"
#include "constellation_priv.h"
#include "pb_decode.h"
#include "reconstruct_wallet_flow.h"
#include "sha2.h"
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
 * fails, then it will send an error to the host CONSTELLATION app and return
 * false.
 *
 * @param query Reference to an instance of constellation_query_t containing
 * query received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const constellation_query_t *query,
                                pb_size_t which_request);

/**
 * @brief The function checks if the given `init_req` is a valid request for
 * message signing and returns true if it is, otherwise it returns false and
 * sends an error message to host.
 *
 * @param init_req A pointer to a structure of type
 * `constellation_sign_msg_initiate_request_t`.
 *
 * @return a boolean value indicating init_req is valid or not.
 */
static bool validate_initiate_query(
    constellation_sign_msg_initiate_request_t *init_req);

/**
 * @brief The function handles the initiation of a query to sign a message for a
 * specific wallet.
 * @details It performs the following tasks in response to a valid initiate
 * request.
 * - Get user confirmation on the wallet and coin for which flow is requested.
 * - Copy the init request data to sign_msg_ctx.
 *
 * @param query Reference to the decoded query struct from the host app
 *
 * @return a boolean value.
 */
static bool handle_initiate_query(constellation_query_t *query);

/**
 * @brief This function is responsible for retrieving and assembling message
 * data chunks for signing.
 *
 * @param query Reference to the decoded query struct from the host app
 *
 * @return a boolean value indicating if the message data has been reconstructed
 * from chunks correctly or not.
 */
static bool get_msg_data(constellation_query_t *query);

/**
 * @brief This function displays the message for verification in case of
 * verified sign.
 *
 * @return a boolean indicating user verification or the rejection.
 */
static bool get_user_verification();

/**
 * @brief This function generates a signature for a message using a given
 * derivation path and private key.
 *
 * @param sig The parameter `sig` is a pointer to a structure of type
 * `constellation_sign_msg_signature_response_t`.
 *
 * @return a boolean value indicating if signautre for msg data is generated
 * correctly or not.
 */
static bool get_msg_data_signature(
    constellation_sign_msg_signature_response_t *sig);

/**
 * @brief This function sends a signature response for a sign message query.
 *
 * @param query A pointer to an constellation_query_t struct, which contains
 * information about the query being processed.
 * @param sig The parameter `sig` is of type
 * `constellation_sign_msg_signature_response_t`, which is a structure
 * containing the signature response for a sign message request.
 *
 * @return a boolean value indicating if the signature is sent to the host
 * correctly or not.
 */
static bool send_signature(constellation_query_t *query,
                           constellation_sign_msg_signature_response_t *sig);

/**
 * @brief This function generates prefixed message/data for signing.
 *
 * @param ctx A pointer to a constellation_sign_msg_context_t struct, which
 * contains information about the message/data to which prefixed needs to be
 * added.
 * @param prefix The prefix to be added to the message/data.
 * @param prefixed_msg_data The buffer to hold the resultant prefixed
 * message/data.
 *
 * @return The length of the resultant prefixed message/data.
 */

static size_t constellation_get_prefixed_msg_data(
    const constellation_sign_msg_context_t *ctx,
    const char *prefix,
    size_t prefix_len,
    char *prefixed_msg_data);

/**
 * @brief This function generates message/data digest for signing by
 * hashing the prefixed message/data.
 *
 * @param ctx A pointer to a constellation_sign_msg_context_t struct, which
 * contains information about the message/data which needs to be hashed.
 * @param digest The buffer to hold the hash digest.
 *
 * @return A boolean whether msg data digest was generated successfully or not.
 */

static bool constellation_get_msg_data_digest(
    const constellation_sign_msg_context_t *ctx,
    uint8_t *digest);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static constellation_sign_msg_context_t sign_msg_ctx;

static const char *sign_message_prefix = "\031Constellation Signed Message:\n";
static const char *sign_data_prefix = "\031Constellation Signed Data:\n";

// for atcab_base64decode_ defined in atca_helpers.c as the original default is
// conditionally defined
uint8_t atcab_b64rules[4] = {'+', '/', '=', 64};
/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const constellation_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->get_public_keys.which_request) {
    constellation_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_initiate_query(
    constellation_sign_msg_initiate_request_t *init_req) {
  bool status = false;

  status = constellation_derivation_path_guard(init_req->derivation_path,
                                               init_req->derivation_path_count);

  uint32_t size_limit = MAX_ALLOWED_SIZE;
  if (init_req->message_type == CONSTELLATION_SIGN_MSG_TYPE_SIGN_TYPED_MSG ||
      init_req->message_type ==
          CONSTELLATION_SIGN_MSG_TYPE_SIGN_ARBITRARY_DATA) {
    size_limit = MAX_MSG_DATA_SIZE;
  }

  status = size_limit >= init_req->message_size;

  if (!status) {
    constellation_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  return true;
}

static bool handle_initiate_query(constellation_query_t *query) {
  uint8_t wallet_name[NAME_SIZE] = {0};
  constellation_result_t response =
      init_constellation_result(CONSTELLATION_RESULT_SIGN_MSG_TAG);
  if (!check_which_request(query,
                           CONSTELLATION_SIGN_MSG_REQUEST_INITIATE_TAG) ||
      !validate_initiate_query(&query->sign_msg.initiate) ||
      !get_wallet_name_by_id(query->sign_msg.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             constellation_send_error)) {
    return false;
  }

  char *ui_text_prompt = UI_TEXT_SIGN_MSG_PROMPT;
  if (query->sign_msg.initiate.message_type ==
      CONSTELLATION_SIGN_MSG_TYPE_SIGN_ARBITRARY_DATA) {
    ui_text_prompt = UI_TEXT_SIGN_DATA_PROMPT;
  }

  char msg[200] = "";
  snprintf(msg, sizeof(msg), ui_text_prompt, CONSTELLATION_NAME, wallet_name);

  if (!core_confirmation(msg, constellation_send_error)) {
    return false;
  }

  set_app_flow_status(CONSTELLATION_SIGN_MSG_STATUS_CONFIRM);

  memcpy(&(sign_msg_ctx.init),
         &(query->sign_msg.initiate),
         sizeof(query->sign_msg.initiate));

  response.sign_msg.which_response =
      CONSTELLATION_SIGN_MSG_RESPONSE_CONFIRMATION_TAG;
  response.sign_msg.confirmation.dummy_field = 0;
  constellation_send_result(&response);

  // show processing screen for a minimum duration (additional time will add due
  // to actual processing)
  delay_scr_init(ui_text_processing, DELAY_SHORT);

  return true;
}

static bool get_msg_data(constellation_query_t *query) {
  constellation_result_t response =
      init_constellation_result(CONSTELLATION_RESULT_SIGN_MSG_TAG);
  uint32_t total_size = sign_msg_ctx.init.message_size;
  const constellation_sign_msg_data_t *msg_data = &query->sign_msg.msg_data;
  const common_chunk_payload_t *payload = &(msg_data->chunk_payload);
  const common_chunk_payload_chunk_t *chunk = &(payload->chunk);

  uint32_t size = 0;

  /**
   * Allocate required memory for message size +1. Extra byte is used to add a
   * NULL character at the end of the msg data in case it'll be used as a string
   */
  sign_msg_ctx.msg_data = malloc(total_size + 1);
  ASSERT(NULL != sign_msg_ctx.msg_data);
  sign_msg_ctx.msg_data[total_size] = '\0';

  while (1) {
    // Get next data chunk from host
    if (!constellation_get_query(query, CONSTELLATION_QUERY_SIGN_MSG_TAG) ||
        !check_which_request(query,
                             CONSTELLATION_SIGN_MSG_REQUEST_MSG_DATA_TAG)) {
      return false;
    }

    if (false == query->sign_msg.msg_data.has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        size + chunk->size > total_size) {
      constellation_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                               ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    memcpy(sign_msg_ctx.msg_data + size, chunk->bytes, chunk->size);
    size += chunk->size;

    // Send chunk ack to host
    response.sign_msg.which_response =
        CONSTELLATION_SIGN_MSG_RESPONSE_DATA_ACCEPTED_TAG;
    response.sign_msg.data_accepted.has_chunk_ack = true;
    response.sign_msg.data_accepted.chunk_ack.chunk_index =
        payload->chunk_index;
    constellation_send_result(&response);

    // If no data remaining to be received from the host, then exit
    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }

  if (total_size != size) {
    constellation_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  return true;
}

static bool get_user_verification() {
  bool result = false;

  char *ui_text_title = UI_TEXT_VERIFY_DATA;
  switch (sign_msg_ctx.init.message_type) {
    case CONSTELLATION_SIGN_MSG_TYPE_SIGN_TYPED_MSG: {
      ui_text_title = UI_TEXT_VERIFY_MESSAGE;
    }
    case CONSTELLATION_SIGN_MSG_TYPE_SIGN_ARBITRARY_DATA: {
      uint8_t decoded_json[sign_msg_ctx.init.message_size];
      size_t decoded_json_len = sign_msg_ctx.init.message_size;
      ATCA_STATUS decoding_status =
          atcab_base64decode_((const char *)sign_msg_ctx.msg_data,
                              sign_msg_ctx.init.message_size,
                              decoded_json,
                              &decoded_json_len,
                              atcab_b64rules);

      if (decoding_status == ATCA_SUCCESS) {
        // TODO: Add a limit on size of data per confirmation based on LVGL
        // buffer and split message into multiple confirmations accordingly
        result = core_scroll_page(
            ui_text_title, (char *)decoded_json, constellation_send_error);
      }
    } break;

    default: {
      result = core_confirmation(UI_TEXT_BLIND_SIGNING_WARNING,
                                 constellation_send_error);
    } break;
  }

  if (result) {
    set_app_flow_status(CONSTELLATION_SIGN_MSG_STATUS_VERIFY);
  }

  return result;
}

static size_t constellation_get_prefixed_msg_data(
    const constellation_sign_msg_context_t *ctx,
    const char *prefix,
    size_t prefix_len,
    char *prefixed_msg_data) {
  size_t msg_len = ctx->init.message_size;

  char length_string[20] = "";
  size_t length_string_len = sprintf(length_string, "%u\n", msg_len);

  size_t total_len = prefix_len + length_string_len + msg_len;

  snprintf(prefixed_msg_data,
           total_len + 1,    // +1 for \0
           "%s%s%s",
           prefix,
           length_string,
           (const char *)ctx->msg_data);

  return total_len;
}

static bool constellation_get_msg_data_digest(
    const constellation_sign_msg_context_t *ctx,
    uint8_t *digest) {
  switch (ctx->init.message_type) {
    case CONSTELLATION_SIGN_MSG_TYPE_SIGN_TYPED_MSG: {
      char prefixed_message[ctx->init.message_size + 100];
      size_t prefixed_message_len = constellation_get_prefixed_msg_data(
          ctx,
          sign_message_prefix,
          strnlen(sign_message_prefix, CONSTELLATION_SIGN_MSG_PREFIX_LENGTH),
          prefixed_message);

      sha512_Raw(
          (const uint8_t *)prefixed_message, prefixed_message_len, digest);
    } break;

    case CONSTELLATION_SIGN_MSG_TYPE_SIGN_ARBITRARY_DATA: {
      char prefixed_data[ctx->init.message_size + 100];
      size_t prefixed_data_len = constellation_get_prefixed_msg_data(
          ctx,
          sign_data_prefix,
          strnlen(sign_data_prefix, CONSTELLATION_SIGN_DATA_PREFIX_LENGTH),
          prefixed_data);

      uint8_t sha256_digest[SHA256_DIGEST_LENGTH] = {0};
      sha256_Raw(
          (const uint8_t *)prefixed_data, prefixed_data_len, sha256_digest);

      char sha256_hex_str[SHA256_DIGEST_LENGTH * 2 + 1] = "";
      byte_array_to_hex_string(sha256_digest,
                               sizeof(sha256_digest),
                               sha256_hex_str,
                               sizeof(sha256_hex_str));

      sha512_Raw((uint8_t *)sha256_hex_str, strlen(sha256_hex_str), digest);
    } break;

    default: {
      sha512_Raw(ctx->msg_data, ctx->init.message_size, digest);
    } break;
  }

  return true;
}

static bool get_msg_data_signature(
    constellation_sign_msg_signature_response_t *sig) {
  bool status = false;
  HDNode node = {0};
  uint8_t seed[64] = {0};
  const size_t depth = sign_msg_ctx.init.derivation_path_count;
  const uint32_t *hd_path = sign_msg_ctx.init.derivation_path;
  const ecdsa_curve *curve = get_curve_by_name(SECP256K1_NAME)->params;

  if (!reconstruct_seed(
          sign_msg_ctx.init.wallet_id, seed, constellation_send_error)) {
    memzero(seed, sizeof(seed));
    return status;
  }

  set_app_flow_status(CONSTELLATION_SIGN_MSG_STATUS_SEED_GENERATED);
  delay_scr_init(ui_text_processing, DELAY_SHORT);

  status = derive_hdnode_from_path(hd_path, depth, SECP256K1_NAME, seed, &node);

  // zeroise the seed
  memzero(seed, sizeof(seed));

  if (!status) {
    constellation_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_DATA);
  } else {
    status = true;
    uint8_t digest[SHA512_DIGEST_LENGTH] = {0};
    uint8_t signature[64] = {0};
    if (!constellation_get_msg_data_digest(&sign_msg_ctx, digest) ||
        (0 != ecdsa_sign_digest(
                  curve, node.private_key, digest, signature, NULL, NULL))) {
      constellation_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
      status = false;
    }
    sig->signature.size = ecdsa_sig_to_der(signature, sig->signature.bytes);
  }
  memzero(&node, sizeof(HDNode));
  return status;
}

static bool send_signature(constellation_query_t *query,
                           constellation_sign_msg_signature_response_t *sig) {
  constellation_result_t result =
      init_constellation_result(CONSTELLATION_RESULT_SIGN_MSG_TAG);
  result.sign_msg.which_response =
      CONSTELLATION_SIGN_MSG_RESPONSE_SIGNATURE_TAG;
  if (!constellation_get_query(query, CONSTELLATION_QUERY_SIGN_MSG_TAG) ||
      !check_which_request(query,
                           CONSTELLATION_SIGN_MSG_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(&result.sign_msg.signature,
         sig,
         sizeof(constellation_sign_msg_signature_response_t));
  constellation_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void constellation_sign_msg(constellation_query_t *query) {
  constellation_sign_msg_signature_response_t sig_resp =
      CONSTELLATION_SIGN_MSG_SIGNATURE_RESPONSE_INIT_DEFAULT;
  memzero(&sign_msg_ctx, sizeof(sign_msg_ctx));

  if (handle_initiate_query(query) && get_msg_data(query) &&
      get_user_verification() && get_msg_data_signature(&sig_resp) &&
      send_signature(query, &(sig_resp))) {
    delay_scr_init(ui_text_check_software_wallet_app, DELAY_TIME);
  }

  if (NULL != sign_msg_ctx.msg_data) {
    memzero(sign_msg_ctx.msg_data, sign_msg_ctx.init.message_size);
    free(sign_msg_ctx.msg_data);
    sign_msg_ctx.msg_data = NULL;
  }

  sign_msg_ctx.init.message_size = 0;

  // Clear the dynamic allocation done for UI purposes using cy_malloc
  cy_free();
}
