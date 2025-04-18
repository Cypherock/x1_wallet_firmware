/**
 * @file    near_pub_key.c
 * @author  Cypherock X1 Team
 * @brief   Generates public key for NEAR derivations.
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

#include "composable_app_queue.h"
#include "exchange_main.h"
#include "near_api.h"
#include "near_context.h"
#include "near_helpers.h"
#include "near_priv.h"
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
#define NEAR_PUB_KEY_SIZE sizeof(ed25519_public_key)

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
 * @param query Reference to an instance of near_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const near_query_t *query,
                                pb_size_t which_request);

/**
 * @brief Validates the derivation paths received in the request from host
 * @details The function validates each path index in the request. If any
 * invalid index is detected, the function will send an error to the host and
 * return false.
 *
 * @param req Reference to an instance of near_get_public_keys_intiate_request_t
 * @param which_request The type of request received from the host.
 * @return bool Indicating if the verification passed or failed
 * @retval true If the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request(const near_get_public_keys_intiate_request_t *req,
                             const pb_size_t which_request);

/**
 * @brief Fills the list of public keys corresponding to the provided list of
 * derivation paths in the buffer
 * @details The function expects the size of list for derivation paths and
 * location for storing derived public keys to be a match with provided count.
 *
 * @param path Reference to the list of near_get_public_keys_derivation_path_t
 * @param seed Reference to a const array containing the seed
 * @param public_key Reference to the location to store all the public keys to
 * be derived
 * @param count Number of derivation paths in the list and consequently,
 * sufficient space in memory for storing derived public keys.
 *
 * @retval true If all the requested public keys were derived successfully
 * @retval false If there is any issue occurred during the key derivation
 */
static bool fill_public_keys(const near_get_public_keys_derivation_path_t *path,
                             const uint8_t *seed,
                             uint8_t public_key_list[][NEAR_PUB_KEY_SIZE],
                             pb_size_t count);

/**
 * @brief The function sends public keys for the requested batch
 * @details The function determines the batch size from the static struct
 * member declaration of nanopb options. The function batches the result based
 * on the definition and sends the result. The function expects that the entire
 * list of public keys requested is already derived and provided to this
 * function as pubkey_list. The function will return false if either the query
 * was wrong or a P0 event is occurred. In case of wrong query, the function
 * also sends an error to the host app.
 *
 * @param query Reference to an instance of near_query_t
 * @param pubkey_list Reference to list of derived public key to be sent to the
 * host
 * @param count Number of public keys entries in the list of public keys
 * @param which_request The type of request to be expected from the host
 * @param which_response The type of response to be sent to the host
 *
 * @return bool Indicating if the public keys was exported completely to the
 * host
 * @retval true If all the requested public keys were exported to the host app
 * @retval false If the export was interrupted by a P0 event or an invalid query
 * was received from the host app.
 */
static bool send_public_keys(near_query_t *query,
                             const uint8_t pubkey_list[][NEAR_PUB_KEY_SIZE],
                             const pb_size_t count,
                             const pb_size_t which_request,
                             const pb_size_t which_response);

/**
 * @details The function provides an ED25519 public key for NEAR. It accepts
 * NULL for output parameter and handles accordingly. The function also manages
 * all the terminal errors during derivation/encoding, in which case it will
 * return false and send a relevant error to the host closing the
 * request-response pair All the errors/invalid cases are conveyed to the host
 * as unknown_error = 1 because we expect the data validation was success.
 * TODO: Make this a common utility function
 *
 * @param seed Reference to the wallet seed generated
 * @param path Derivation path of the node to be derived
 * @param path_length Expected length of the provided derivation path
 * @param public_key Storage location for raw uncompressed public key
 *
 * @retval false If derivation failed
 */
static bool get_public_key(const uint8_t *seed,
                           const uint32_t *path,
                           uint32_t path_length,
                           uint8_t *public_key);

/**
 * @brief Helper function to take user consent before exporting public keys to
 * the host. Uses an appropriate message template based on the query request
 * received from the host.
 *
 * @param which_request The type of request received from host
 * @param wallet_name The name of the wallet on which the request needs to be
 * performed
 * @return true If the user accepted the request
 * @return false If the user rejected or any P0 event occurred during the
 * confirmation.
 */
static bool get_user_consent(const pb_size_t which_request,
                             const char *wallet_name);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

static bool check_which_request(const near_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->get_public_keys.which_request) {
    near_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                    ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request(const near_get_public_keys_intiate_request_t *req,
                             const pb_size_t which_request) {
  bool status = true;
  const pb_size_t count = req->derivation_paths_count;

  if (0 == count) {
    // request does not have any derivation paths, invalid request
    near_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                    ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  if (NEAR_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG == which_request &&
      1 < count) {
    // `NEAR_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG` request contains more than
    // one derivation path which is not expected
    near_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                    ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  const near_get_public_keys_derivation_path_t *path = NULL;

  for (pb_size_t index = 0; index < count; index++) {
    path = &req->derivation_paths[index];
    if (!near_derivation_path_guard(path->path, path->path_count)) {
      near_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_DATA);
      status = false;
      break;
    }
  }

  caq_node_data_t data = {.applet_id = get_applet_id()};

  memzero(data.params, sizeof(data.params));
  memcpy(data.params, req->wallet_id, sizeof(req->wallet_id));
  data.params[32] = EXCHANGE_FLOW_TAG_RECEIVE;

  exchange_app_validate_caq(data);

  return status;
}

static bool get_public_key(const uint8_t *seed,
                           const uint32_t *path,
                           uint32_t path_length,
                           uint8_t *public_key) {
  HDNode node = {0};

  if (!derive_hdnode_from_path(path, path_length, ED25519_NAME, seed, &node)) {
    // send unknown error; unknown failure reason
    near_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    memzero(&node, sizeof(HDNode));
    return false;
  }

  if (NULL != public_key) {
    ed25519_publickey(node.private_key, public_key);
  }

  memzero(&node, sizeof(HDNode));
  return true;
}

static bool fill_public_keys(const near_get_public_keys_derivation_path_t *path,
                             const uint8_t *seed,
                             uint8_t public_key_list[][NEAR_PUB_KEY_SIZE],
                             pb_size_t count) {
  for (pb_size_t index = 0; index < count; index++) {
    const near_get_public_keys_derivation_path_t *current = &path[index];
    if (!get_public_key(
            seed, current->path, current->path_count, public_key_list[index])) {
      return false;
    }
  }

  return true;
}

static bool send_public_keys(near_query_t *query,
                             const uint8_t pubkey_list[][NEAR_PUB_KEY_SIZE],
                             const pb_size_t count,
                             const pb_size_t which_request,
                             const pb_size_t which_response) {
  near_result_t response = init_near_result(which_response);
  near_get_public_keys_result_response_t *result =
      &response.get_public_keys.result;
  size_t batch_limit =
      sizeof(response.get_public_keys.result.public_keys) / NEAR_PUB_KEY_SIZE;
  size_t remaining = count;

  response.get_public_keys.which_response =
      NEAR_GET_PUBLIC_KEYS_RESPONSE_RESULT_TAG;
  while (true) {
    // send response as batched list of public keys
    size_t batch_size = CY_MIN(batch_limit, remaining);
    result->public_keys_count = batch_size;

    memcpy(result->public_keys,
           &pubkey_list[count - remaining],
           batch_size * NEAR_PUB_KEY_SIZE);

    near_send_result(&response);
    remaining -= batch_size;
    if (0 == remaining) {
      break;
    }

    if (!near_get_query(query, which_request) ||
        !check_which_request(query,
                             NEAR_GET_PUBLIC_KEYS_REQUEST_FETCH_NEXT_TAG)) {
      return false;
    }
  }
  return true;
}

static bool get_user_consent(const pb_size_t which_request,
                             const char *wallet_name) {
  char msg[100] = "";

  if (NEAR_QUERY_GET_PUBLIC_KEYS_TAG == which_request) {
    snprintf(msg,
             sizeof(msg),
             UI_TEXT_ADD_ACCOUNT_PROMPT,
             near_app.name,
             wallet_name);
  } else {
    snprintf(msg,
             sizeof(msg),
             UI_TEXT_RECEIVE_TOKEN_PROMPT,
             near_app.lunit_name,
             near_app.name,
             wallet_name);
  }

  return core_scroll_page(NULL, msg, near_send_error);
}

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void near_get_pub_keys(near_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  uint8_t seed[64] = {0};

  const pb_size_t which_request = query->which_request;
  const near_get_public_keys_intiate_request_t *init_req = NULL;
  pb_size_t which_response;

  if (NEAR_QUERY_GET_PUBLIC_KEYS_TAG == which_request) {
    which_response = NEAR_RESULT_GET_PUBLIC_KEYS_TAG;
    init_req = &query->get_public_keys.initiate;
  } else {
    which_response = NEAR_RESULT_GET_USER_VERIFIED_PUBLIC_KEY_TAG;
    init_req = &query->get_user_verified_public_key.initiate;
  }

  const pb_size_t count = init_req->derivation_paths_count;

  uint8_t pubkey_list[sizeof(init_req->derivation_paths) /
                      sizeof(near_get_public_keys_derivation_path_t)]
                     [NEAR_PUB_KEY_SIZE] = {0};

  if (!check_which_request(query, NEAR_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG) ||
      !validate_request(init_req, which_request) ||
      !get_wallet_name_by_id(
          init_req->wallet_id, (uint8_t *)wallet_name, near_send_error)) {
    return;
  }

  // Take user consent to export public key for the wallet
  if (!get_user_consent(which_request, wallet_name)) {
    return;
  }

  set_app_flow_status(NEAR_GET_PUBLIC_KEYS_STATUS_CONFIRM);

  if (!reconstruct_seed(init_req->wallet_id, &seed[0], near_send_error)) {
    memzero(seed, sizeof(seed));
    return;
  }

  set_app_flow_status(NEAR_GET_PUBLIC_KEYS_STATUS_SEED_GENERATED);
  delay_scr_init(ui_text_processing, DELAY_SHORT);

  bool result =
      fill_public_keys(init_req->derivation_paths, seed, pubkey_list, count);

  // Clear seed as soon as it is not needed
  memzero(seed, sizeof(seed));

  if (!result) {
    return;
  }

  // In case the request is to `NEAR_QUERY_GET_PUBLIC_KEY_TAG` type, then wait
  // for user verification of the address
  if (NEAR_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG == which_request) {
    char address[100] = "";
    byte_array_to_hex_string(
        pubkey_list[0], sizeof(pubkey_list[0]), address, sizeof(address));
    if (!core_scroll_page(ui_text_receive_on, address, near_send_error)) {
      return;
    }
    set_app_flow_status(NEAR_GET_PUBLIC_KEYS_STATUS_VERIFY);
  }

  if (!send_public_keys(
          query, pubkey_list, count, which_request, which_response)) {
    return;
  }

  delay_scr_init(ui_text_check_cysync_app, DELAY_TIME);
  return;
}
