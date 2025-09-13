/**
 * @file    canton_txn.c
 * @author  Cypherock X1 Team
 * @brief   Canton internal apis related transaction signing
 *
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bip32.h"
#include "canton/core.pb.h"
#include "canton/sign_txn.pb.h"
#include "canton_api.h"
#include "canton_context.h"
#include "canton_priv.h"
#include "coin_utils.h"
#include "constant_texts.h"
#include "curves.h"
#include "ecdsa.h"
#include "ed25519.h"
#include "error.pb.h"
#include "memzero.h"
#include "pb.h"
#include "reconstruct_wallet_flow.h"
#include "sha2.h"
#include "status_api.h"
#include "ui_common.h"
#include "ui_core_confirm.h"
#include "ui_delay.h"
#include "ui_screens.h"
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

typedef canton_sign_txn_signature_response_signature_t signature_t;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Verifies if the provided query contains expected request
 *
 * @param query Reference to @ref canton_query_t obj
 * @param which_request Expected request type enum variant
 *
 * @returns bool Indicating whether check succeeded or failed
 */
static bool is_query_type(const canton_query_t *query, pb_size_t which_request);

/**
 * @brief Validates the derivation path recieved in the request from the host.
 *
 * @param request Reference to @ref canton_sign_txn_request_t obj
 * @return bool Indicating whether verification succeeded or failed
 */
static bool is_valid_request_data(const canton_sign_txn_request_t *request);

/**
 * @brief Sends response to device of given tag
 *
 * @param which_response Tag of the response type to be sent
 */
static void send_response(pb_size_t which_response);


/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

static canton_txn_context_t *canton_txn_context = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool is_query_type(const canton_query_t *query,
                          pb_size_t which_request) {
  if (which_request != query->sign_txn.which_request) {
    canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }
  return true;
}

static bool is_valid_request_data(const canton_sign_txn_request_t *request) {
  // TODO: impl
  return true;
}

static void send_response(const pb_size_t which_response) {
  canton_result_t result = init_canton_result(CANTON_RESULT_SIGN_TXN_TAG);
  result.sign_txn.which_response = which_response;
  canton_send_result(&result);
}


/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void canton_sign_transaction(canton_query_t *query) {
}
