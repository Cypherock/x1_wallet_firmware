/**
 * @file    firmware_update.c
 * @author  Cypherock X1 Team
 * @brief   Manager app function to get user confirmation before going to
 *          bootloader mode for firmware update
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
#include "board.h"
#include "common_error.h"
#include "manager_api.h"
#include "sec_flash.h"
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
 * @brief Checks if the provided request has valid data
 * @details It checks if the target firmware version is strictly greater than
 * the currently installed firmware version
 *
 * @param request Reference to the request received from the host
 * @return true Indicating that the request is valid
 * @return false Indicating that the request is not valid
 */
static bool validate_query(const manager_firmware_update_request_t *request);
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool check_which_request(const manager_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->firmware_update.which_request) {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_query(const manager_firmware_update_request_t *request) {
  if (false == request->initiate.has_version) {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_FIELD_MISSING);
    return false;
  }
  uint32_t current_version = get_fwVer();

  const common_version_t *target = &request->initiate.version;
  uint32_t target_version =
      (target->major << 24) | (target->minor << 16) | (target->patch);

  // Query is invalid if the target version is equal or less than the current
  // firmware version installed
  if (target_version <= current_version) {
    manager_result_t result =
        init_manager_result(MANAGER_RESULT_FIRMWARE_UPDATE_TAG);
    manager_firmware_update_response_t *resp = &result.firmware_update;
    resp->which_response = MANAGER_FIRMWARE_UPDATE_RESPONSE_ERROR_TAG;
    resp->error.error = MANAGER_FIRMWARE_UPDATE_ERROR_VERSION_NOT_ALLOWED;
    manager_send_result(&result);
    return false;
  }

  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void manager_confirm_firmware_update(manager_query_t *query) {
  if ((!check_which_request(
          query, MANAGER_FIRMWARE_UPDATE_INITIATE_REQUEST_VERSION_TAG)) ||
      (!validate_query(&query->firmware_update))) {
    return;
  }

  char msg[100];
  snprintf(msg,
           sizeof(msg),
           FIRMWARE_UPDATE_CONFIRMATION,
           query->firmware_update.initiate.version.major,
           query->firmware_update.initiate.version.minor,
           query->firmware_update.initiate.version.patch);
  if (!core_confirmation(msg, manager_send_error)) {
    return;
  }

  manager_result_t result =
      init_manager_result(MANAGER_RESULT_FIRMWARE_UPDATE_TAG);
  manager_firmware_update_response_t *resp = &result.firmware_update;
  resp->which_response = MANAGER_FIRMWARE_UPDATE_RESPONSE_CONFIRMED_TAG;
  resp->confirmed.dummy_field = '\0';
  manager_send_result(&result);

  // NOTE: This is a USB initiated flow, however, device will go in bootloader
  // mode after blocking delay of 500ms without serving any events. So in case
  // any abort event is triggered by the host, it will NOT be served!
  // NOTE: Wait for status pull to desktop (which requests at 200ms)
  instruction_scr_init(ui_text_processing, NULL);
  BSP_DelayMs(500);
  FW_enter_DFU();
  BSP_reset();

  return;
}
