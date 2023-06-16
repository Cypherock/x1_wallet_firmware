/**
 * @file    get_public_key.c
 * @author  Cypherock X1 Team
 * @brief   Generates public key for bitcoin derivations.
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

#include "bip32.h"
#include "btc.h"
#include "btc/core.pb.h"
#include "btc_api.h"
#include "events.h"
#include "pb_encode.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "usb_api.h"

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
 * @brief Validates the derivation paths received in the request from host
 * @details The function validates each path index in the request. If any
 * invalid index is detected, the function will send an error to the host and
 * return false.
 *
 * @param request Reference to an instance of btc_get_public_key_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(btc_get_public_key_request_t *request);

/**
 * @brief Derives public address from the provided public key
 *
 * @param node Reference to the address node
 * @param purpose_index Purpose index of the provided address node path
 * @param coin_index Coin index of the provided address node path
 * @param address Storage location for encoded public address
 *
 * @return int length of the derived address
 * @retval 0 If derivation failed
 */
static int btc_get_address(const uint8_t *seed,
                           const uint32_t *path,
                           uint32_t path_length,
                           uint8_t *public_key,
                           char *address);

/**
 * @brief Derives public key from seed and takes user confirmation prior sending
 * to host
 * @details The function derives the HDNode at specified derivation path and
 * sends the uncompressed public key to the host. Prior to sending the public
 * key, the function derives and verifies public address in the respective
 * encoding format.
 *
 * @param query Reference to the query received from the host
 * @param seed The wallet seed generated from X1 Card
 *
 * @return bool Indicating if the public key was sent successfully.
 * @retval true The user confirmed his/her intention to export address
 * @retval false The user either rejected the prompt or a P0 event occurred
 */
static bool confirm_and_send(const btc_query_t *query, const uint8_t *seed);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

static bool check_which_request(const btc_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->get_public_key.which_request) {
    btc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request_data(btc_get_public_key_request_t *request) {
  bool status = true;

  // TODO: Enable btc/coin specific check
  if (false ==
      verify_receive_derivation_path(request->initiate.derivation_path,
                                     request->initiate.derivation_path_count)) {
    btc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                   ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }
  return status;
}

static int btc_get_address(const uint8_t *seed,
                           const uint32_t *path,
                           uint32_t path_length,
                           uint8_t *public_key,
                           char *address) {
  HDNode node = {0};
  uint8_t address_version = 0;
  uint32_t purpose_index = path[0];
  uint32_t coin_index = path[1];
  int address_length = 0;

  if (false ==
      derive_hdnode_from_path(path, path_length, SECP256K1_NAME, seed, &node)) {
    // send unknown error; unknown failure reason
    btc_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    memzero(&node, sizeof(HDNode));
    return address_length;
  }

  switch (purpose_index) {
    case NATIVE_SEGWIT:
      address_length = get_segwit_address(
          node.public_key, sizeof(node.public_key), coin_index, address);
      break;
    case NON_SEGWIT:
      get_version(purpose_index, coin_index, &address_version, NULL);
      hdnode_get_address(&node, address_version, address, 35);
      break;
    // TODO: add support for taproot and segwit
    default:
      address_length = 0;
      break;
  }

  if (NULL != public_key) {
    memcpy(public_key, node.public_key, ECDSA_PUB_KEY_SIZE);
  }
  memzero(&node, sizeof(HDNode));
  return address_length;
}

static bool confirm_and_send(const btc_query_t *query, const uint8_t *seed) {
  const uint32_t *path = query->get_public_key.initiate.derivation_path;
  uint32_t path_length = query->get_public_key.initiate.derivation_path_count;
  uint8_t public_key[65] = {0};
  char address[50] = "";
  bool status = false;

  int address_length =
      btc_get_address(seed, path, path_length, public_key, address);

  if (0 < address_length) {
    // wait for user confirmation to send address to desktop
    ui_scrollable_page(
        ui_text_receive_on, address, MENU_SCROLL_HORIZONTAL, false);
    evt_status_t events = get_events(EVENT_CONFIG_UI, MAX_INACTIVITY_TIMEOUT);
    if (true == events.p0_event.flag) {
      // core will handle p0 events, exit now
      status = false;
    }
    if (UI_EVENT_REJECT == events.ui_event.event_type) {
      // user rejected sending of address, send error and exit
      btc_send_error(ERROR_COMMON_ERROR_USER_REJECTION_TAG,
                     ERROR_USER_REJECTION_ADDRESS);
      status = false;
    } else if (UI_EVENT_CONFIRM == events.ui_event.event_type) {
      // send response
      status = true;
      btc_result_t response = init_btc_result(BTC_RESULT_GET_PUBLIC_KEY_TAG);
      response.get_public_key.which_response =
          BTC_GET_PUBLIC_KEY_RESPONSE_RESULT_TAG;
      ecdsa_uncompress_pubkey(get_curve_by_name(SECP256K1_NAME)->params,
                              public_key,
                              response.get_public_key.result.public_key);
      btc_send_result(&response);
    }
  }
  return status;
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

void btc_get_public_key(btc_query_t *query) {
  const char *wallet_name = "";
  char msg[100] = "";
  uint8_t wallet_index = 0;

  if (!check_which_request(query, BTC_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->get_public_key)) {
    return;
  }

  // this will always succeed; can skip checking return value
  get_first_matching_index_by_id(query->get_public_key.initiate.wallet_id,
                                 &wallet_index);
  wallet_name = (const char *)get_wallet_name(wallet_index);
  snprintf(msg, sizeof(msg), "Receive Bitcoin in %s", wallet_name);
  // Take user consent to export address for the wallet
  if (!core_user_confirmation(msg, btc_send_error)) {
    return;
  }

  core_status_set_flow_status(BTC_GET_PUBLIC_KEY_STATUS_CONFIRM);

  // TODO: call the reconstruct flow and get seed from the core
  uint8_t seed[64] = {0};    // = generate_seed();

  core_status_set_flow_status(BTC_GET_PUBLIC_KEY_STATUS_CARD);
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  if (true == confirm_and_send(query, seed)) {
    delay_scr_init(ui_text_check_cysync_app, DELAY_TIME);
  }
  memzero(seed, sizeof(seed));
}
