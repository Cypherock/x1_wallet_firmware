/**
 * @file    get_logs.c
 * @author  Cypherock X1 Team
 * @brief   Dumps device logs from flash to the host in ascii
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

#include "common_error.h"
#include "flash_api.h"
#include "manager_api.h"
#include "onboarding.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"

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
 * @brief The function prepares and sends the error for disabled logs situation.
 */
static void log_disabled_error();

/**
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host manager app and return false.
 *
 * @param query Reference to an instance of manager_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const manager_query_t *query,
                                pb_size_t which_request);

/**
 * @brief Checks if the logs can be exported to the host
 * @details The function return true (indicating logs can be exported) if the
 * user on-boarding is pending. If the user training is complete, the logs can
 * be exported only if the user has enabled logs in the settings.
 *
 * @return bool Indicating whether logs can be exported to the host
 * @retval true Logs can be exported in the current state
 * @retval false Logs cannot be exported in the current state
 */
static bool check_logs_export();

/**
 * @brief The function prepares and send logs one flash page at a time
 * @details The uses the logger apis to initialize and fetch the logs from the
 * flash memory. It will listen for USB events from the host requesting the next
 * chunk of logs. The function will return false if either the query was wrong
 * or a P0 event is occurred. In case of wrong query, the function also sends an
 * error to the host app.
 *
 * @param query Reference to an instance of manager_query_t
 * @param result Reference to an instance of manager_result_t
 *
 * @return bool Indicating if the log was exported completely to the host
 * @retval true If the entire logs were exported to the host app
 * @retval false If the export was interrupted by a P0 event or an invalid query
 * was received from the host app.
 */
static bool send_logs(manager_query_t *query, manager_result_t *result);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static void log_disabled_error() {
  manager_result_t result = init_manager_result(MANAGER_RESULT_GET_LOGS_TAG);
  result.get_logs.which_response = MANAGER_GET_LOGS_RESPONSE_ERROR_TAG;
  result.get_logs.error.logs_disabled = true;
  manager_send_result(&result);
}

static bool check_which_request(const manager_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->get_logs.which_request) {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool check_logs_export() {
  if (MANAGER_ONBOARDING_STEP_COMPLETE != onboarding_get_last_step()) {
    return true;
  }

  return is_logging_enabled();
}

static bool send_logs(manager_query_t *query, manager_result_t *result) {
  size_t log_size = 0;
  set_start_log_read();

  while (1) {
    logger_task(result->get_logs.logs.data.bytes, &log_size);
    result->get_logs.logs.data.size = log_size;
    // the logger state is LOG_READ_FINISH when it sends the last packet
    result->get_logs.logs.has_more = (LOG_READ_FINISH != get_log_read_status());
    manager_send_result(result);

    if (false == result->get_logs.logs.has_more) {
      // all logs sent, exit now
      break;
    }

    if (!manager_get_query(query, MANAGER_QUERY_GET_LOGS_TAG) ||
        !check_which_request(query, MANAGER_GET_LOGS_REQUEST_FETCH_NEXT_TAG)) {
      return false;
    }
  }

  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void manager_get_logs(manager_query_t *query) {
  /** Validate if exporting logs is allowed.
   * Logs should be allowed in restricted mode (unauthenticated device)
   * irrespective of `Log Setting`
   */
  if (DEVICE_AUTHENTICATED == get_auth_state() &&
      false == check_logs_export()) {
    log_disabled_error();
    return;
  }

  manager_result_t result = init_manager_result(MANAGER_RESULT_GET_LOGS_TAG);
  if (!check_which_request(query, MANAGER_GET_LOGS_REQUEST_INITIATE_TAG) ||
      !core_confirmation(ui_text_send_logs_prompt, manager_send_error)) {
    return;
  }

  result.get_logs.which_response = MANAGER_GET_LOGS_RESPONSE_LOGS_TAG;
  delay_scr_init(ui_text_sending_logs, DELAY_SHORT);
  set_app_flow_status(MANAGER_GET_LOGS_STATUS_USER_CONFIRMED);
  if (true == send_logs(query, &result)) {
    // logs sent successfully, display "Logs sent"
    delay_scr_init(ui_text_logs_sent, DELAY_TIME);
  }
}