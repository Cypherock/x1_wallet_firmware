/**
 * @file    cardano_pub_key.c
 * @author  Cypherock X1 Team
 * @brief   Cardano public key derivation
 * @copyright Copyright (c) 2026 HODL TECH PTE LTD <br/> You may obtain a copy
 *of license at <a href="https://mitcc.org/"
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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bip32.h"
#include "cardano/core.pb.h"
#include "cardano/get_public_key.pb.h"
#include "cardano_api.h"
#include "cardano_context.h"
#include "cardano_helpers.h"
#include "cardano_priv.h"
#include "coin_utils.h"
#include "constant_texts.h"
#include "error.pb.h"
#include "memzero.h"
#include "pb.h"
#include "reconstruct_wallet_flow.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_delay.h"
#include "utils.h"
#include "wallet.h"
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

/*****************************************************************************
 * STATIC VARIABLES
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
 * @brief Validates the derivation paths received in the request from host
 * @details The function validates each path index in the request. If any
 * invalid index is detected, the function will send an error to the host and
 * return false.
 *
 * @param req Reference to an instance of
 * cardano_get_public_keys_intiate_request_t
 * @param which_request The type of request received from the host.
 * @return bool Indicating if the verification passed or failed
 * @retval true If the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request(
    const cardano_get_public_keys_intiate_request_t *req,
    pb_size_t which_request) __attribute__((warn_unused_result));

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
static bool get_user_consent(pb_size_t which_request, const char *wallet_name)
    __attribute__((warn_unused_result));

/**
 * @details The function provides an Ed25519-BIP32 public key for cardano. It
 * accepts NULL for output parameter and handles accordingly. The function also
 * manages all the terminal errors during derivation/encoding, in which case it
 * will return false and send a relevant error to the host closing the
 * request-response pair All the errors/invalid cases are conveyed to the host
 * as unknown_error = 1 because we expect the data validation was success.
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
                           uint8_t public_key[CARDANO_PUBLIC_KEY_SIZE])
    __attribute__((warn_unused_result));

/**
 * @brief Fills the list of public keys corresponding to the provided list of
 * derivation paths in the buffer
 * @details The function expects the size of list for derivation paths and
 * location for storing derived public keys to be a match with provided count.
 *
 * @attention: since host only ever sends payment address derivation path
 * and never stake address, verified by @ref cardano_derivation_path_guard
 *
 * @param[in] path Reference to the list of
 * cardano_get_public_keys_derivation_path_t
 * @param[in] seed Reference to a const array containing the seed
 * @param[out] out_payment_public_key_list Reference to the location to store
 * all the public keys to be derived from payment derivation
 * @param[out] out_stake_public_key_list Reference to the location to store all
 * the public keys to be derived from stake derivation
 * @param[in] count Number of derivation paths in the list and consequently,
 * sufficient space in memory for storing derived public keys.
 *
 * @retval true If all the requested public keys were derived successfully
 * @retval false If there is any issue occurred during the key derivation
 */
static bool fill_public_keys(
    const cardano_get_public_keys_derivation_path_t *path,
    const uint8_t *seed,
    uint8_t out_public_key_list[][CARDANO_PUBLIC_KEY_SIZE],
    uint8_t out_stake_public_key_list[][CARDANO_PUBLIC_KEY_SIZE],
    pb_size_t count) __attribute__((warn_unused_result));

/**
 * @brief Sends public keys, both stake and payment, to host.
 * @details
 *
 * @param[in] query
 * @param[in] stake_pubkey_list
 * @param[in] payment_pubkey_list
 * @param[in] count
 *
 * @return bool Indicating if the public keys were sent successfully.
 */
static bool send_public_keys(
    cardano_query_t *query,
    const uint8_t stake_pubkey_list[][CARDANO_PUBLIC_KEY_SIZE],
    const uint8_t payment_pubkey_list[][CARDANO_PUBLIC_KEY_SIZE],
    const pb_size_t count,
    const pb_size_t which_request,
    const pb_size_t which_response) __attribute__((warn_unused_result));

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const cardano_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->get_public_keys.which_request) {
    cardano_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request(
    const cardano_get_public_keys_intiate_request_t *req,
    const pb_size_t which_request) {
  bool status = true;
  const pb_size_t count = req->derivation_paths_count;

  /* request does not have any derivation path */
  if (0 == count) {
    cardano_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  if (CARDANO_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG == which_request &&
      1 < count) {
    /*    user verified keys with more than 1 derivation path is not expected */
    cardano_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_DATA);
    status = false;
  }

  const cardano_get_public_keys_derivation_path_t *path = NULL;
  for (pb_size_t index = 0; index < count; index++) {
    path = &req->derivation_paths[index];
    if (!cardano_derivation_path_guard(path->path, path->path_count)) {
      cardano_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_DATA);
      status = false;
      break;
    }
  }

  return status;
}

static bool get_user_consent(const pb_size_t which_request,
                             const char *wallet_name) {
  char msg[100] = "";

  if (CARDANO_QUERY_GET_PUBLIC_KEYS_TAG == which_request) {
    int ignored = snprintf(msg,
                           sizeof(msg),
                           UI_TEXT_ADD_ACCOUNT_PROMPT,
                           CARDANO_NAME,
                           wallet_name);
    (void)ignored;
  } else {
    int ignored = snprintf(
        msg, sizeof(msg), UI_TEXT_RECEIVE_PROMPT, CARDANO_NAME, wallet_name);
    (void)ignored;
  }

  return core_scroll_page(NULL, msg, cardano_send_error);
}

static bool get_public_key(const uint8_t *seed,
                           const uint32_t *path,
                           uint32_t path_length,
                           uint8_t public_key[CARDANO_PUBLIC_KEY_SIZE]) {
  HDNode node = {0};
  if (!derive_hdnode_from_path_cardano(path, path_length, seed, &node)) {
    /* unknown error */
    cardano_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    memzero(&node, sizeof(HDNode));
    return false;
  }

  if (NULL != public_key) {
    hdnode_fill_public_key(&node);
    // WARN: MAKE SURE TO SKIP THE HEADER
    //                                    this -> +1
    memcpy(public_key, node.public_key + 1, CARDANO_PUBLIC_KEY_SIZE);
  }
  memzero(&node, sizeof(HDNode));
  return true;
}

static bool fill_public_keys(
    const cardano_get_public_keys_derivation_path_t *path,
    const uint8_t *seed,
    uint8_t out_payment_public_key_list[][CARDANO_PUBLIC_KEY_SIZE],
    uint8_t out_stake_public_key_list[][CARDANO_PUBLIC_KEY_SIZE],
    pb_size_t count) {
  cardano_get_public_keys_derivation_path_t current_stake =
      CARDANO_GET_PUBLIC_KEYS_DERIVATION_PATH_INIT_ZERO;

  for (pb_size_t index = 0; index < count; index++) {
    const cardano_get_public_keys_derivation_path_t *current_payment =
        &path[index];

    /* we need to compute this just once */
    if (0 == index) {
      stake_derv_from_payment(current_payment, &current_stake);
    }

    /* fill payment derivation public key */
    if (!get_public_key(seed,
                        current_payment->path,
                        current_payment->path_count,
                        out_payment_public_key_list[index])) {
      return false;
    }

    /* compute stake only for first derivation path */
    if (0 == index) {
      /* fill stake derivation public key */
      if (!get_public_key(seed,
                          current_stake.path,
                          current_stake.path_count,
                          out_stake_public_key_list[0])) {
        return false;
      }
    } else {
      /* we can simply copy for the rest of the stake address */
      memcpy(out_stake_public_key_list[index],
             out_stake_public_key_list[0],
             CARDANO_PUBLIC_KEY_SIZE);
    }
  }
  return true;
}

static bool send_public_keys(
    cardano_query_t *query,
    const uint8_t stake_pubkey_list[][CARDANO_PUBLIC_KEY_SIZE],
    const uint8_t payment_pubkey_list[][CARDANO_PUBLIC_KEY_SIZE],
    const pb_size_t count,
    const pb_size_t which_request,
    const pb_size_t which_response) {
  cardano_result_t response = init_cardano_result(which_response);
  cardano_get_public_keys_result_response_t *result =
      &response.get_public_keys.result;
  size_t batch_limit =
      sizeof(response.get_public_keys.result.payment_pub_keys) /
      CARDANO_PUBLIC_KEY_SIZE;
  size_t remaining = count;
  response.get_public_keys.which_response =
      CARDANO_GET_PUBLIC_KEYS_RESPONSE_RESULT_TAG;

  for (;;) {
    /* send response in batched list */
    size_t batch_size = CY_MIN(batch_limit, remaining);
    result->stake_pub_keys_count = result->payment_pub_keys_count = batch_size;

    /* copy payment pub keys */
    memcpy(result->payment_pub_keys,
           &payment_pubkey_list[count - remaining],
           batch_size * CARDANO_PUBLIC_KEY_SIZE);

    /* copy stake pub keys */
    memcpy(result->stake_pub_keys,
           &stake_pubkey_list[count - remaining],
           batch_size * CARDANO_PUBLIC_KEY_SIZE);

    cardano_send_result(&response);
    remaining -= batch_size;
    if (0 == remaining) {
      break;
    }

    if (!cardano_get_query(query, which_request) ||
        !check_which_request(query,
                             CARDANO_GET_PUBLIC_KEYS_REQUEST_FETCH_NEXT_TAG)) {
      return false;
    }
  }
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void cardano_get_pub_keys(cardano_query_t *query) {
  const pb_size_t which_request = query->which_request;
  const cardano_get_public_keys_intiate_request_t *init_req = NULL;
  pb_size_t which_response = 0;

  if (CARDANO_QUERY_GET_PUBLIC_KEYS_TAG == which_request) {
    which_response = CARDANO_RESULT_GET_PUBLIC_KEYS_TAG;
    init_req = &query->get_public_keys.initiate;
  } else {
    which_response = CARDANO_RESULT_GET_USER_VERIFIED_PUBLIC_KEY_TAG;
    init_req = &query->get_user_verified_public_key.initiate;
  }

  /* valid request? */
  char wallet_name[NAME_SIZE] = "";
  if (!check_which_request(query,
                           CARDANO_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG) ||
      !validate_request(init_req, which_request) ||
      !get_wallet_name_by_id(
          init_req->wallet_id, (uint8_t *)wallet_name, cardano_send_error)) {
    return;
  }

  /* take user consent */
  if (!get_user_consent(which_request, wallet_name)) {
    return;
  }

  set_app_flow_status(CARDANO_GET_PUBLIC_KEYS_STATUS_CONFIRM);

  /* reconstruct seed */
  uint8_t seed[64] = {0};
  if (!reconstruct_seed(init_req->wallet_id, seed, cardano_send_error)) {
    memzero(seed, sizeof(seed));
    return;
  }

  set_app_flow_status(CARDANO_GET_PUBLIC_KEYS_STATUS_SEED_GENERATED);
  delay_scr_init(ui_text_processing, DELAY_SHORT);

  const pb_size_t count = init_req->derivation_paths_count;

  uint8_t stake_pubkey_list[sizeof(init_req->derivation_paths) /
                            sizeof(cardano_get_public_keys_derivation_path_t)]
                           [CARDANO_PUBLIC_KEY_SIZE];

  uint8_t payment_pubkey_list[sizeof(init_req->derivation_paths) /
                              sizeof(cardano_get_public_keys_derivation_path_t)]
                             [CARDANO_PUBLIC_KEY_SIZE];

  /* fill public keys both payment derivation and stake derivation */
  bool result = fill_public_keys(init_req->derivation_paths,
                                 seed,
                                 payment_pubkey_list,
                                 stake_pubkey_list,
                                 count);

  /* clear seed */
  memzero(seed, sizeof(seed));

  /* unknown error during derivation */
  if (!result) {
    cardano_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    return;
  }

  /* if request is to `CARDANO_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG` then
   * we also verify it from the user. */
  if (CARDANO_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG == which_request) {
    /* first we need to compute stake addr since it is also required in
     * payment address. */

    uint8_t stake_addr[CARDANO_STAKE_ADDR_LENGTH] = {0};
    if (!get_stake_addr(stake_pubkey_list[0], stake_addr)) {
      cardano_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
      return;
    }

    /* verify stake addr */
    if (!core_scroll_page(
            ui_text_stake_address, (char *)stake_addr, cardano_send_error)) {
      return;
    }

    /* now compute payment address */
    uint8_t payment_addr[CARDANO_PAYMENT_ADDR_LENGTH] = {0};
    if (!get_payment_addr(
            stake_pubkey_list[0], payment_pubkey_list[0], payment_addr)) {
      cardano_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
      return;
    }

    /* verify payment addr */
    if (!core_scroll_page(
            ui_text_receive_on, (char *)payment_addr, cardano_send_error)) {
      return;
    }

    set_app_flow_status(CARDANO_GET_PUBLIC_KEYS_STATUS_VERIFY);
  }
  if (!send_public_keys(query,
                        stake_pubkey_list,
                        payment_pubkey_list,
                        count,
                        which_request,
                        which_response)) {
    return;
  }

  /* send public keys */
  delay_scr_init(ui_text_check_cysync_app, DELAY_TIME);
}
