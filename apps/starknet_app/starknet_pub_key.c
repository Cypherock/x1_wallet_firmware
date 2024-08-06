/**
 * @file    starknet_pub_key.c
 * @author  Cypherock X1 Team
 * @brief   Generates public key for Starknet derivations.
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

#include "reconstruct_wallet_flow.h"
#include "starknet_api.h"
#include "starknet_context.h"
#include "starknet_helpers.h"
#include "starknet_priv.h"
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

#define STARKNET_PUB_KEY_SIZE 65

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * static FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host STARKNET app and return false.
 *
 * @param query Reference to an instance of starknet_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const starknet_query_t *query,
                                pb_size_t which_request);
/**
 * @brief Validates all the derivation paths received in the request from host
 * @details The function validates each path entry in the request. If any
 * invalid path is detected, the function will send an error to the host and
 * return false.
 *
 * @param request Reference to an instance of starknet_get_public_keys_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(starknet_get_public_keys_request_t *request,
                                  const pb_size_t which_request);

/**
 * @brief Derives a list of public key corresponding to the provided list of
 * derivation paths.
 * @details The function expects the size of list for derivation paths and
 * location for storing derived public keys to be a match with provided count.
 *
 * @param paths Reference to the list of
 * starknet_get_public_keys_derivation_path_t
 * @param count Number of derivation paths in the list and consequently,
 * sufficient space in memory for storing derived public keys.
 * @param seed Reference to a const array containing the seed
 * @param public_keys Reference to the location to store all the public keys to
 * be derived
 *
 * @return bool Indicating if the complete public keys list was derived
 * @retval true If all the requested public keys were derived.
 * @retval false If the public key derivation failed. This could be due to
 * invalid derivation path.
 */
static bool fill_public_keys(
    const starknet_get_public_keys_derivation_path_t *paths,
    const uint8_t *seed,
    uint8_t public_keys[][STARKNET_PUB_KEY_SIZE],
    pb_size_t count);

/**
 * @brief The function sends public keys for the requested batch
 * @details The function determines the batch size from the static struct
 * member declaration of nanopb options. The function batches the result based
 * on the definition and sends the result. The function expects that the entire
 * list of public keys requested is already derived and provided to this
 * function as public_keys. The function will return false if either the query
 * was wrong or a P0 event is occurred. In case of wrong query, the function
 * also sends an error to the host app.
 *
 * @param query Reference to an instance of starknet_query_t
 * @param public_keys Reference to list of derived public keys to be sent to the
 * host
 * @param count Number of public key entries in the list of public keys
 *
 * @return bool Indicating if the public keys were exported completely to the
 * host
 * @retval true If all the requested public keys were exported to the host app
 * @retval false If the export was interrupted by a P0 event or an invalid query
 * was received from the host app.
 */
static bool send_public_keys(starknet_query_t *query,
                             const uint8_t public_keys[][STARKNET_PUB_KEY_SIZE],
                             const size_t count,
                             const pb_size_t which_request,
                             const pb_size_t which_response);
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
 * static VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * static FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const starknet_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->get_public_keys.which_request) {
    starknet_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request_data(starknet_get_public_keys_request_t *request,
                                  const pb_size_t which_request) {
  bool status = true;

  if (0 == request->initiate.derivation_paths_count) {
    // request does not have any derivation paths, invalid request
    starknet_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  if (STARKNET_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG == which_request &&
      1 < request->initiate.derivation_paths_count) {
    // `STARKNET_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG` request contains more
    // than one derivation path which is not expected
    starknet_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  starknet_get_public_keys_derivation_path_t *path = NULL;
  pb_size_t count = request->initiate.derivation_paths_count;
  for (pb_size_t index = 0; index < count; index++) {
    path = &request->initiate.derivation_paths[index];
    if (!starknet_derivation_path_guard(path->path, path->path_count)) {
      starknet_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                          ERROR_DATA_FLOW_INVALID_DATA);
      status = false;
      break;
    }
  }

  return status;
}

static bool fill_public_keys(
    const starknet_get_public_keys_derivation_path_t *path,
    const uint8_t *seed,
    uint8_t public_key_list[][STARKNET_PUB_KEY_SIZE],
    pb_size_t count) {
  uint8_t hdkey1[32] = {0};

  for (pb_size_t index = 0; index < count; index++) {
    const starknet_get_public_keys_derivation_path_t *current = &path[index];
    if (!starknet_derive_bip32_node(seed, hdkey1) ||
        !starknet_derive_key_from_seed(hdkey1,
                                       current->path,
                                       current->path_count,
                                       public_key_list[index])) {
      // TODO: Generate public key from private key
      return false;
    }
  }

  return true;
}

static bool send_public_keys(starknet_query_t *query,
                             const uint8_t public_keys[][STARKNET_PUB_KEY_SIZE],
                             const size_t count,
                             const pb_size_t which_request,
                             const pb_size_t which_response) {
  starknet_result_t response = init_starknet_result(which_response);
  starknet_get_public_keys_result_response_t *result =
      &response.get_public_keys.result;
  static const size_t batch_limit =
      sizeof(response.get_public_keys.result.public_keys) /
      STARKNET_PUB_KEY_SIZE;
  size_t remaining = count;

  response.get_public_keys.which_response =
      STARKNET_GET_PUBLIC_KEYS_RESPONSE_RESULT_TAG;

  while (true) {
    // send response as batched list of public keys
    size_t batch_size = CY_MIN(batch_limit, remaining);
    result->public_keys_count = batch_size;

    memcpy(response.get_public_keys.result.public_keys,
           public_keys[count - remaining],
           batch_size * STARKNET_PUB_KEY_SIZE);

    starknet_send_result(&response);
    remaining -= batch_size;
    if (0 == remaining) {
      break;
    }

    if (!starknet_get_query(query, which_request) ||
        !check_which_request(query,
                             STARKNET_GET_PUBLIC_KEYS_REQUEST_FETCH_NEXT_TAG)) {
      return false;
    }
  }
  return true;
}

static bool get_user_consent(const pb_size_t which_request,
                             const char *wallet_name) {
  char msg[100] = "";

  if (STARKNET_QUERY_GET_PUBLIC_KEYS_TAG == which_request) {
    snprintf(msg,
             sizeof(msg),
             UI_TEXT_ADD_ACCOUNT_PROMPT,
             starknet_app.name,
             wallet_name);
  } else {
    snprintf(msg,
             sizeof(msg),
             UI_TEXT_RECEIVE_TOKEN_PROMPT,
             starknet_app.lunit_name,
             starknet_app.name,
             wallet_name);
  }

  return core_scroll_page(NULL, msg, starknet_send_error);
}

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * static FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void starknet_get_pub_keys(starknet_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  uint8_t seed[64] = {0};

  const pb_size_t which_request = query->which_request;
  starknet_get_public_keys_intiate_request_t *init_req = NULL;
  pb_size_t which_response = STARKNET_RESULT_COMMON_ERROR_TAG;

  if (STARKNET_QUERY_GET_PUBLIC_KEYS_TAG == which_request) {
    which_response = STARKNET_RESULT_GET_PUBLIC_KEYS_TAG;
    init_req = &query->get_public_keys.initiate;
  } else {
    which_response = STARKNET_RESULT_GET_USER_VERIFIED_PUBLIC_KEY_TAG;
    init_req = &query->get_user_verified_public_key.initiate;
  }

  uint8_t public_keys[sizeof(init_req->derivation_paths) /
                      sizeof(starknet_get_public_keys_derivation_path_t)]
                     [STARKNET_PUB_KEY_SIZE] = {0};

  if (!check_which_request(query,
                           STARKNET_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->get_public_keys, which_request) ||
      !get_wallet_name_by_id(query->get_public_keys.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             starknet_send_error)) {
    return;
  }

  // Take user consent to export public key for the wallet
  if (!get_user_consent(which_request, wallet_name)) {
    return;
  }

  set_app_flow_status(STARKNET_GET_PUBLIC_KEYS_STATUS_CONFIRM);

  if (!reconstruct_seed(query->get_public_keys.initiate.wallet_id,
                        &seed[0],
                        starknet_send_error)) {
    memzero(seed, sizeof(seed));
    return;
  }

  set_app_flow_status(STARKNET_GET_PUBLIC_KEYS_STATUS_SEED_GENERATED);
  delay_scr_init(ui_text_processing, DELAY_SHORT);

  bool status = fill_public_keys(init_req->derivation_paths,
                                 seed,
                                 public_keys,
                                 init_req->derivation_paths_count);

  // Clear seed as soon as it is not needed
  memzero(seed, sizeof(seed));

  if (!status) {
    // send unknown error; do not know failure reason
    starknet_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return;
  }

  if (STARKNET_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG == which_request) {
    char address[100] = "";

    // TODO: Derive address from public key
    // size_t public_key_size = sizeof(address);
    if (!true) {
      starknet_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 3);
      return;
    };

    if (!core_scroll_page(ui_text_receive_on, address, starknet_send_error)) {
      return;
    }

    set_app_flow_status(STARKNET_GET_PUBLIC_KEYS_STATUS_VERIFY);
  }

  if (!send_public_keys(query,
                        public_keys,
                        init_req->derivation_paths_count,
                        which_request,
                        which_response)) {
    return;
  }

  delay_scr_init(ui_text_check_cysync_app, DELAY_TIME);
}