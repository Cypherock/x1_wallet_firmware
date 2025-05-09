/**
 * @file    initiate_flow.c
 * @author  Cypherock X1 Team
 * @brief   Initiates the exhchange flow
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

#include <string.h>

#include "composable_app_queue.h"
#include "exchange/initiate_flow.pb.h"
#include "exchange_api.h"
#include "exchange_context.h"
#include "exchange_main.h"
#include "memzero.h"
#include "ui_delay.h"

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
 * fails, then it will send an error to the host exchange app and return false.
 *
 * @param query Reference to an instance of exchange_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const exchange_query_t *query,
                                pb_size_t which_request);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const exchange_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->initiate_flow.which_request) {
    exchange_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void exchange_initiate_flow(exchange_query_t *query) {
  exchange_result_t result =
      init_exchange_result(EXCHANGE_RESULT_INITIATE_FLOW_TAG);
  if (!check_which_request(query,
                           EXCHANGE_INITIATE_FLOW_REQUEST_INITIATE_TAG)) {
    return;
  }

  // Clear existing composable app queue
  caq_clear();

  delay_scr_init("Swap flow initiated", DELAY_TIME);

  // Receive flow
  {
    caq_node_data_t data = {.applet_id =
                                query->initiate_flow.initiate.to.applet_id};

    memzero(data.params, sizeof(data.params));
    memcpy(data.params,
           query->initiate_flow.initiate.to.wallet_id,
           sizeof(query->initiate_flow.initiate.to.wallet_id));
    data.params[32] = EXCHANGE_FLOW_TAG_RECEIVE;

    caq_push(data);
  }

  // Fetch signature for receive address
  {
    caq_node_data_t data = {.applet_id = get_exchange_app_desc()->id};

    memzero(data.params, sizeof(data.params));
    data.params[0] = EXCHANGE_FLOW_TAG_FETCH_SIGNATURE;

    caq_push(data);
  }

  // Store signature for receiver address in send flow
  {
    caq_node_data_t data = {.applet_id = get_exchange_app_desc()->id};

    memzero(data.params, sizeof(data.params));
    data.params[0] = EXCHANGE_FLOW_TAG_STORE_SIGNATURE;

    caq_push(data);
  }

  // Send flow
  {
    caq_node_data_t data = {.applet_id =
                                query->initiate_flow.initiate.from.applet_id};

    memzero(data.params, sizeof(data.params));
    memcpy(data.params,
           query->initiate_flow.initiate.from.wallet_id,
           sizeof(query->initiate_flow.initiate.from.wallet_id));
    data.params[32] = EXCHANGE_FLOW_TAG_SEND;

    caq_push(data);
  }

  result.initiate_flow.which_response =
      EXCHANGE_INITIATE_FLOW_RESPONSE_RESULT_TAG;
  exchange_send_result(&result);
}
