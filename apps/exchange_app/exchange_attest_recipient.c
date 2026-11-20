/**
 * @file    exchange_attest_recipient.c
 * @author  Cypherock X1 Team
 * @brief   Attests (signs) a user-confirmed Private Send recipient
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

#include <stdio.h>
#include <string.h>

#include "composable_app_queue.h"
#include "constant_texts.h"
#include "exchange/attest_recipient.pb.h"
#include "exchange_api.h"
#include "exchange_main.h"
#include "memzero.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_delay.h"

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

#define ATTEST_PREFIX "PS1"
#define ATTEST_CHAIN_MAX 32
#define ATTEST_ADDRESS_MAX 256
// prefix + ':' + chain + ':' + address + NUL
#define ATTEST_MESSAGE_MAX                                                     \
  (sizeof(ATTEST_PREFIX) + ATTEST_CHAIN_MAX + ATTEST_ADDRESS_MAX)

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

static bool check_which_request(const exchange_query_t *query,
                                pb_size_t which_request);

static bool validate_input(
    const exchange_attest_recipient_initiate_request_t *req);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const exchange_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->attest_recipient.which_request) {
    exchange_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  // The attest step must be the next expected step of the queued flow
  caq_node_data_t data = {.applet_id = get_applet_id()};
  memzero(data.params, sizeof(data.params));
  data.params[0] = EXCHANGE_FLOW_TAG_ATTEST;

  if (!exchange_app_validate_caq(data)) {
    exchange_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_input(
    const exchange_attest_recipient_initiate_request_t *req) {
  size_t chain_len = strnlen(req->chain, sizeof(req->chain));
  size_t addr_len = strnlen(req->address, sizeof(req->address));

  if (chain_len == 0 || chain_len >= sizeof(req->chain) || addr_len == 0 ||
      addr_len >= sizeof(req->address) || strchr(req->chain, ':') != NULL) {
    return false;
  }

  // Printable ASCII only
  for (size_t i = 0; i < addr_len; i++) {
    if ((uint8_t)req->address[i] < 0x21 || (uint8_t)req->address[i] > 0x7E) {
      return false;
    }
  }
  for (size_t i = 0; i < chain_len; i++) {
    if ((uint8_t)req->chain[i] < 0x21 || (uint8_t)req->chain[i] > 0x7E) {
      return false;
    }
  }

  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void exchange_attest_recipient(exchange_query_t *query) {
  exchange_result_t result =
      init_exchange_result(EXCHANGE_RESULT_ATTEST_RECIPIENT_TAG);

  if (!check_which_request(query,
                           EXCHANGE_ATTEST_RECIPIENT_REQUEST_INITIATE_TAG)) {
    return;
  }

  const exchange_attest_recipient_initiate_request_t *req =
      &query->attest_recipient.initiate;

  if (!validate_input(req)) {
    exchange_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_DATA);
    return;
  }

  // Show the recipient on the device so the user confirms what is signed
  char title[sizeof(UI_TEXT_PRIVATE_SEND_VERIFY_RECIPIENT) + ATTEST_CHAIN_MAX] =
      "";
  snprintf(
      title, sizeof(title), UI_TEXT_PRIVATE_SEND_VERIFY_RECIPIENT, req->chain);

  if (!core_scroll_page(title, req->address, exchange_send_error)) {
    return;
  }

  char message[ATTEST_MESSAGE_MAX] = "";
  snprintf(message,
           sizeof(message),
           "%s:%s:%s",
           ATTEST_PREFIX,
           req->chain,
           req->address);

  exchange_sign_address(message, sizeof(message));
  memzero(message, sizeof(message));

  result.attest_recipient.which_response =
      EXCHANGE_ATTEST_RECIPIENT_RESPONSE_RESULT_TAG;
  exchange_send_result(&result);
}
