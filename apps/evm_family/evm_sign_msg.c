/**
 * @file    evm_sign_msg.c
 * @author  Cypherock X1 Team
 * @brief   Signs ETH, personal and typed data struct messages for EVM
 *derivations.
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

#include <stddef.h>
#include <stdint.h>

#include "evm_api.h"
#include "evm_helpers.h"
#include "evm_priv.h"
#include "evm_typed_data_helper.h"
#include "pb_decode.h"
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
 * fails, then it will send an error to the host EVM app and return false.
 *
 * @param query Reference to an instance of evm_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const evm_query_t *query,
                                pb_size_t which_request);

/**
 * @brief The function checks if the given `init_req` is a valid request for
 * message signing and returns true if it is, otherwise it returns false and
 * sends an error message to host.
 *
 * @param init_req A pointer to a structure of type
 * `evm_sign_msg_initiate_request_t`.
 *
 * @return a boolean value indicating init_req is valid or not.
 */
static bool validate_initiate_query(evm_sign_msg_initiate_request_t *init_req);

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
static bool handle_initiate_query(evm_query_t *query);

/**
 * @brief This function is responsible for retrieving and assembling message
 * data chunks for signing.
 *
 * @param query Reference to the decoded query struct from the host app
 *
 * @return a boolean value indicating if the message data has been reconstructed
 * from chunks correctly or not.
 */
static bool get_msg_data(evm_query_t *query);

/**
 * @brief This function checks the message type and displays the message data
 * for verification.
 *
 * @return a boolean indicating user verification or the rejection.
 */
static bool get_user_verification();

/**
 * @brief This function generates a signature for a message using a given
 * derivation path and private key.
 *
 * @param sig The parameter `sig` is a pointer to a structure of type
 * `evm_sign_msg_signature_response_t`.
 *
 * @return a boolean value indicating if signautre for msg data is generated
 * correctly or not.
 */
static bool get_msg_data_signature(evm_sign_msg_signature_response_t *sig);

/**
 * @brief This function sends a signature response for a sign message query.
 *
 * @param query A pointer to an evm_query_t struct, which contains information
 * about the query being processed.
 * @param sig The parameter `sig` is of type
 * `evm_sign_msg_signature_response_t`, which is a structure containing the
 * signature response for a sign message request.
 *
 * @return a boolean value indicating if the signature is sent to the host
 * correctly or not.
 */
static bool send_signature(evm_query_t *query,
                           evm_sign_msg_signature_response_t *sig);
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static evm_sign_msg_context_t sign_msg_ctx;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const evm_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->get_public_keys.which_request) {
    evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_initiate_query(evm_sign_msg_initiate_request_t *init_req) {
  uint32_t size_limit = MAX_MSG_DATA_SIZE;

  switch (init_req->message_type) {
    case EVM_SIGN_MSG_TYPE_SIGN_TYPED_DATA:
      size_limit = MAX_MSG_DATA_TYPED_DATA_SIZE;
    case EVM_SIGN_MSG_TYPE_ETH_SIGN:
    case EVM_SIGN_MSG_TYPE_PERSONAL_SIGN:

      if (!evm_derivation_path_guard(init_req->derivation_path,
                                     init_req->derivation_path_count)) {
        break;
      }

      // TODO: Replace macro with EVM_TRANSACTION_SIZE_CAP upon fixing the issue
      // with double allocation of typed data
      if (size_limit < init_req->total_msg_size) {
        evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_DATA);
        break;
      }
      return true;
      break;

    default:
      break;
  }

  evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                 ERROR_DATA_FLOW_INVALID_DATA);
  return false;
}

static bool handle_initiate_query(evm_query_t *query) {
  char msg[100] = "";
  uint8_t wallet_name[NAME_SIZE] = {0};
  evm_result_t response = init_evm_result(EVM_RESULT_SIGN_MSG_TAG);
  if (!check_which_request(query, EVM_SIGN_MSG_REQUEST_INITIATE_TAG) ||
      !validate_initiate_query(&query->sign_msg.initiate) ||
      !get_wallet_name_by_id(query->sign_msg.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             evm_send_error)) {
    return false;
  }

  snprintf(msg,
           sizeof(msg),
           UI_TEXT_SIGN_PROMPT,
           g_evm_app->lunit_name,
           g_evm_app->name,
           wallet_name);

  // Take user consent to sign message for the wallet
  if (!core_confirmation(msg, evm_send_error)) {
    return false;
  }

  set_app_flow_status(EVM_SIGN_MSG_STATUS_CONFIRM);

  memcpy(&(sign_msg_ctx.init),
         &(query->sign_msg.initiate),
         sizeof(query->sign_msg.initiate));

  response.sign_msg.which_response = EVM_SIGN_MSG_RESPONSE_CONFIRMATION_TAG;
  response.sign_msg.confirmation.dummy_field = 0;
  evm_send_result(&response);

  return true;
}

static bool get_msg_data(evm_query_t *query) {
  evm_result_t response = init_evm_result(EVM_RESULT_SIGN_MSG_TAG);
  uint32_t total_size = sign_msg_ctx.init.total_msg_size;
  const evm_sign_msg_data_t *msg_data = &query->sign_msg.msg_data;
  const common_chunk_payload_t *payload = &(msg_data->chunk_payload);
  const common_chunk_payload_chunk_t *chunk = &(payload->chunk);

  uint32_t size = 0;

  /**
   * Allocate required memory for message size +1. Extra byte is used to add a
   * NULL character at the end of the msg data in case it'll be used as a string
   */
  sign_msg_ctx.msg_data = malloc(sign_msg_ctx.init.total_msg_size + 1);
  ASSERT(NULL != sign_msg_ctx.msg_data);
  sign_msg_ctx.msg_data[sign_msg_ctx.init.total_msg_size] = '\0';

  while (1) {
    // Get next data chunk from host
    if (!evm_get_query(query, EVM_QUERY_SIGN_MSG_TAG) ||
        !check_which_request(query, EVM_SIGN_MSG_REQUEST_MSG_DATA_TAG)) {
      return false;
    }

    if (false == query->sign_msg.msg_data.has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        size + chunk->size > total_size) {
      evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                     ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    memcpy(sign_msg_ctx.msg_data + size, chunk->bytes, chunk->size);
    size += chunk->size;

    // Send chunk ack to host
    response.sign_msg.which_response = EVM_SIGN_MSG_RESPONSE_DATA_ACCEPTED_TAG;
    response.sign_msg.data_accepted.has_chunk_ack = true;
    response.sign_msg.data_accepted.chunk_ack.chunk_index =
        payload->chunk_index;
    evm_send_result(&response);

    // If no data remaining to be received from the host, then exit
    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }

  if (total_size != size) {
    evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  if (EVM_SIGN_MSG_TYPE_SIGN_TYPED_DATA == sign_msg_ctx.init.message_type) {
    pb_istream_t istream =
        pb_istream_from_buffer(sign_msg_ctx.msg_data, total_size);
    bool result = pb_decode(&istream,
                            EVM_SIGN_TYPED_DATA_STRUCT_FIELDS,
                            &(sign_msg_ctx.typed_data));

    if (!result) {
      evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                     ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }
  }

  return true;
}

static bool get_user_verification() {
  bool result = false;
  switch (sign_msg_ctx.init.message_type) {
    case EVM_SIGN_MSG_TYPE_ETH_SIGN: {
      const size_t array_size = sign_msg_ctx.init.total_msg_size * 2 + 3;
      char *buffer = malloc(array_size);
      memzero(buffer, array_size);
      snprintf(buffer, array_size, "0x");
      byte_array_to_hex_string(sign_msg_ctx.msg_data,
                               sign_msg_ctx.init.total_msg_size,
                               buffer + 2,
                               array_size - 2);
      // TODO: Add a limit on size of data per confirmation based on LVGL buffer
      // and split message into multiple confirmations accordingly
      result = core_scroll_page(
          UI_TEXT_VERIFY_MESSAGE, (const char *)buffer, evm_send_error);
      memzero(buffer, array_size);
      free(buffer);
    } break;

    case EVM_SIGN_MSG_TYPE_PERSONAL_SIGN: {
      // TODO: Add a limit on size of data per confirmation based on LVGL buffer
      // and split message into multiple confirmations accordingly
      result = core_scroll_page(UI_TEXT_VERIFY_MESSAGE,
                                (const char *)sign_msg_ctx.msg_data,
                                evm_send_error);
    } break;

    case EVM_SIGN_MSG_TYPE_SIGN_TYPED_DATA: {
      ui_display_node *display_node = NULL;
      evm_init_typed_data_display_node(&display_node,
                                       &(sign_msg_ctx.typed_data));
      while (NULL != display_node) {
        result = core_scroll_page(
            display_node->title, display_node->value, evm_send_error);
        display_node = display_node->next;

        if (!result) {
          break;
        }
      }
    } break;

    default:
      break;
  }

  if (result) {
    set_app_flow_status(EVM_SIGN_MSG_STATUS_VERIFY);
  }

  return result;
}

static bool get_msg_data_signature(evm_sign_msg_signature_response_t *sig) {
  bool status = false;
  HDNode node = {0};
  uint8_t buffer[64] = {0};
  const size_t depth = sign_msg_ctx.init.derivation_path_count;
  const uint32_t *hd_path = sign_msg_ctx.init.derivation_path;
  const ecdsa_curve *curve = get_curve_by_name(SECP256K1_NAME)->params;

  if (!reconstruct_seed(sign_msg_ctx.init.wallet_id, buffer, evm_send_error)) {
    memzero(buffer, sizeof(buffer));
    return status;
  }

  set_app_flow_status(EVM_SIGN_MSG_STATUS_SEED_GENERATED);
  delay_scr_init(ui_text_processing, DELAY_SHORT);

  status =
      derive_hdnode_from_path(hd_path, depth, SECP256K1_NAME, buffer, &node);

  // zeroise the seed
  memzero(buffer, sizeof(buffer));
  if (!status) {
    evm_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
  } else {
    status = true;
    if (!evm_get_msg_data_digest(&sign_msg_ctx, buffer) ||
        (0 != ecdsa_sign_digest(
                  curve, node.private_key, buffer, sig->r, sig->v, NULL))) {
      evm_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
      status = false;
    }
  }
  memzero(&node, sizeof(HDNode));
  return status;
}

static bool send_signature(evm_query_t *query,
                           evm_sign_msg_signature_response_t *sig) {
  evm_result_t result = init_evm_result(EVM_RESULT_SIGN_MSG_TAG);
  result.sign_msg.which_response = EVM_SIGN_MSG_RESPONSE_SIGNATURE_TAG;
  if (!evm_get_query(query, EVM_QUERY_SIGN_MSG_TAG) ||
      !check_which_request(query, EVM_SIGN_MSG_REQUEST_SIGNATURE_TAG)) {
    return false;
  }

  memcpy(&result.sign_msg.signature,
         sig,
         sizeof(evm_sign_msg_signature_response_t));
  evm_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void evm_sign_msg(evm_query_t *query) {
  evm_sign_msg_signature_response_t sig_resp =
      EVM_SIGN_MSG_SIGNATURE_RESPONSE_INIT_DEFAULT;
  memzero(&sign_msg_ctx, sizeof(sign_msg_ctx));

  if (handle_initiate_query(query) && get_msg_data(query) &&
      get_user_verification() && get_msg_data_signature(&sig_resp) &&
      send_signature(query, &(sig_resp))) {
    delay_scr_init(ui_text_check_cysync_app, DELAY_TIME);
  }

  if (NULL != sign_msg_ctx.msg_data) {
    memzero(sign_msg_ctx.msg_data, sign_msg_ctx.init.total_msg_size);
    free(sign_msg_ctx.msg_data);
    sign_msg_ctx.msg_data = NULL;
  }

  sign_msg_ctx.init.total_msg_size = 0;

  /**
   * The tyepd data struct fields are of FT_POINTER type which means memory for
   * typed data is dynamically allocated. The dynamic allocated data needs to be
   * cleared before we exit the app here.
   */
  pb_release(EVM_SIGN_TYPED_DATA_STRUCT_FIELDS, &(sign_msg_ctx.typed_data));

  // Clear the dynamic allocation done for UI purposes using cy_malloc
  cy_free();
  return;
}
