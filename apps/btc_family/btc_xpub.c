/**
 * @file    btc_xpub.c
 * @author  Cypherock X1 Team
 * @brief   Apis for exporting account level xpub to host for bitcoin
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

#include "btc_api.h"
#include "btc_helpers.h"
#include "btc_priv.h"
#include "curves.h"
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
 * fails, then it will send an error to the host bitcoin app and return false.
 *
 * @param query Reference to an instance of btc_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const btc_query_t *query,
                                pb_size_t which_request);

/**
 * @brief Validates all the derivation paths received in the request from host
 * @details The function validates each path entry in the request. If any
 * invalid path is detected, the function will send an error to the host and
 * return false.
 *
 * @param request Reference to an instance of btc_get_xpubs_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(btc_get_xpubs_request_t *request);

/**
 * @brief Derives a list of xpubs corresponding to the provided list of
 * derivation paths.
 * @details The function expects the size of list for derivation paths and
 * location for storing derived xpubs to be a match with provided count.
 *
 * @param paths Reference to the list of btc_get_xpub_derivation_path_t
 * @param seed Reference to a const array containing the seed
 * @param xpubs Reference to the location to store all the xpubs to be derived
 * @param count Number of derivation paths in the list and consequently,
 * sufficient space in memory for storing derived xpubs.
 *
 * @return bool Indicating if the complete xpub list was derived
 * @retval true If all the requested xpubs were derived.
 * @retval false If the xpub derivation failed. This could be due to invalid
 * derivation path.
 */
static bool one_shot_xpub_generate(const btc_get_xpub_derivation_path_t *paths,
                                   const uint8_t *seed,
                                   char xpubs[][XPUB_SIZE],
                                   pb_size_t count);

/**
 * @brief The function sends xpubs for the requested batch
 * @details The function determines the batch size from the static struct
 * member declaration of nanopb options. The function batches the result based
 * on the definition and sends the result. The function expects that the entire
 * list of xpubs requested is already derived and provided to this function as
 * xpub_list. The function will return false if either the query was wrong
 * or a P0 event is occurred. In case of wrong query, the function also sends an
 * error to the host app.
 *
 * @param query Reference to an instance of btc_query_t
 * @param xpub_list Reference to list of derived xpubs to be sent to the host
 * @param count Number of xpub entries in the list of xpubs
 *
 * @return bool Indicating if the xpub was exported completely to the host
 * @retval true If all the requested xpubs were exported to the host app
 * @retval false If the export was interrupted by a P0 event or an invalid query
 * was received from the host app.
 */
static bool send_xpubs(btc_query_t *query,
                       const char xpub_list[][XPUB_SIZE],
                       pb_size_t count);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const btc_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->get_xpubs.which_request) {
    btc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request_data(btc_get_xpubs_request_t *request) {
  bool status = true;

  if (0 == request->initiate.derivation_paths_count) {
    // request does not have any derivation paths, invalid request
    btc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  btc_get_xpub_derivation_path_t *path = NULL;
  pb_size_t count = request->initiate.derivation_paths_count;
  for (pb_size_t index = 0; index < count; index++) {
    path = &request->initiate.derivation_paths[index];
    if (!btc_derivation_path_guard(path->path, path->path_count)) {
      btc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                     ERROR_DATA_FLOW_INVALID_DATA);
      status = false;
      break;
    }
  }
  return status;
}

static bool one_shot_xpub_generate(const btc_get_xpub_derivation_path_t *paths,
                                   const uint8_t *seed,
                                   char xpubs[][XPUB_SIZE],
                                   pb_size_t count) {
  for (pb_size_t index = 0; index < count; index++) {
    const btc_get_xpub_derivation_path_t *path = &paths[index];
    uint32_t xpub_ver = 0;
    if (!btc_get_version(path->path[0], &xpub_ver) ||
        !btc_generate_xpub(path->path,
                           path->path_count,
                           SECP256K1_NAME,
                           seed,
                           xpub_ver,
                           xpubs[index])) {
      return false;
    }
  }
  return true;
}

static bool send_xpubs(btc_query_t *query,
                       const char xpub_list[][XPUB_SIZE],
                       pb_size_t count) {
  btc_result_t response = init_btc_result(BTC_RESULT_GET_XPUBS_TAG);
  btc_get_xpubs_result_response_t *result = &response.get_xpubs.result;
  size_t batch_limit = sizeof(response.get_xpubs.result.xpubs) / XPUB_SIZE;
  size_t remaining = count;

  response.get_xpubs.which_response = BTC_GET_XPUBS_RESPONSE_RESULT_TAG;
  while (true) {
    // send response as batched list of xpubs
    size_t batch_size = CY_MIN(batch_limit, remaining);
    result->xpubs_count = batch_size;
    memcpy(
        result->xpubs, &xpub_list[count - remaining], batch_size * XPUB_SIZE);
    btc_send_result(&response);
    remaining -= batch_size;
    if (0 == remaining) {
      break;
    }

    if (!btc_get_query(query, BTC_QUERY_GET_XPUBS_TAG) ||
        !check_which_request(query, BTC_GET_XPUBS_REQUEST_FETCH_NEXT_TAG)) {
      return false;
    }
  }
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void btc_get_xpub(btc_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";
  uint8_t seed[64] = {0};
  const btc_get_xpubs_intiate_request_t *init_req = &query->get_xpubs.initiate;
  char xpub_list[sizeof(init_req->derivation_paths) /
                 sizeof(btc_get_xpub_derivation_path_t)][XPUB_SIZE] = {0};

  if (!check_which_request(query, BTC_GET_XPUBS_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->get_xpubs) ||
      !get_wallet_name_by_id(query->get_xpubs.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             btc_send_error)) {
    return;
  }

  snprintf(msg,
           sizeof(msg),
           UI_TEXT_ADD_ACCOUNT_PROMPT,
           g_btc_app->name,
           wallet_name);
  // Take user consent to export xpub for the wallet
  if (!core_confirmation(msg, btc_send_error)) {
    return;
  }

  set_app_flow_status(BTC_GET_XPUBS_STATUS_CONFIRM);

  if (!reconstruct_seed(
          query->get_xpubs.initiate.wallet_id, &seed[0], btc_send_error)) {
    memzero(seed, sizeof(seed));
    return;
  }

  set_app_flow_status(BTC_GET_XPUBS_STATUS_SEED_GENERATED);
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  bool status = one_shot_xpub_generate(init_req->derivation_paths,
                                       seed,
                                       xpub_list,
                                       init_req->derivation_paths_count);
  memzero(seed, sizeof(seed));
  if (true == status) {
    status = send_xpubs(query, xpub_list, init_req->derivation_paths_count);
  } else {
    // send unknown error; do not know failure reason
    btc_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
  }

  if (status) {
    delay_scr_init(ui_text_check_cysync_app, DELAY_TIME);
  }

  return;
}