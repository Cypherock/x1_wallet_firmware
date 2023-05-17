/**
 * @file    get_device_info.c
 * @author  Cypherock X1 Team
 * @brief   Populates device info fields at runtime requests.
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

#include "controller_level_four.h"
#include "flash_api.h"
#include "manager_api.h"
#include "manager_app.h"
#include "onboarding.h"

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
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Returns the formatted semantic versioning.
 *
 * @param firmware_version
 * @return bool Status indicating the
 */
static bool get_firmware_version(common_version_t *firmware_version);

/**
 * @brief Return a filled instance of get_device_info_response_t.
 */
static manager_get_device_info_response_t get_device_info(void);

/**
 * @brief Sets the necessary field identifiers along with setting the error code
 *
 * @param device_info Reference to manager_get_device_info_response_t to fill
 * @param error_code The error code to be set in error struct
 */
static void fill_device_info_unknown_error(
    manager_get_device_info_response_t *device_info,
    uint32_t error_code);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool get_firmware_version(common_version_t *firmware_version) {
  uint32_t version = get_fwVer();

  if (NULL == firmware_version || 0 == version || 0xFFFFFFFF == version) {
    return false;
  }

  firmware_version->major = (version >> 24) & 0xFF;
  firmware_version->minor = (version >> 16) & 0xFF;
  firmware_version->patch = version & 0xFFFF;
  return true;
}

static manager_get_device_info_response_t get_device_info(void) {
  manager_get_device_info_response_t device_info =
      MANAGER_GET_DEVICE_INFO_RESPONSE_INIT_ZERO;
  uint32_t status = get_device_serial();

  device_info.which_response = MANAGER_GET_DEVICE_INFO_RESPONSE_RESULT_TAG;
  if (status != ATCA_SUCCESS) {
    fill_device_info_unknown_error(&device_info, status);
    LOG_CRITICAL("serial %d", status);
  }

  if (device_info.which_response ==
      MANAGER_GET_DEVICE_INFO_RESPONSE_RESULT_TAG) {
    manager_get_device_info_result_response_t *result = &device_info.result;
    result->has_firmware_version =
        get_firmware_version(&result->firmware_version);
    memcpy(result->device_serial, atecc_data.device_serial, DEVICE_SERIAL_SIZE);
    result->is_authenticated = (is_device_authenticated() == 1);
    result->is_initial = true;            // TODO: Get from memory
    result->has_initial_states = true;    // TODO: Get from memory
    // TODO: populate applet list (result->applet_list)
  }

  return device_info;
}

static void fill_device_info_unknown_error(
    manager_get_device_info_response_t *device_info,
    uint32_t error_code) {
  device_info->which_response = MANAGER_GET_DEVICE_INFO_RESPONSE_CORE_ERROR_TAG;
  device_info->core_error.which_error = ERROR_CORE_ERROR_UNKNOWN_ERROR_TAG;
  device_info->core_error.unknown_error = error_code;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void get_device_info_flow(const manager_query_t *query) {
  size_t msg_size = 0;
  // TODO: replace MANAGER_GET_DEVICE_INFO_RESULT_RESPONSE_SIZE with relevant
  // macro/value
  uint8_t response[MANAGER_GET_DEVICE_INFO_RESULT_RESPONSE_SIZE] = {0};
  manager_result_t result = MANAGER_RESULT_INIT_ZERO;

  if (MANAGER_QUERY_GET_DEVICE_INFO_TAG != query->which_request ||
      MANAGER_GET_DEVICE_INFO_REQUEST_INITIATE_TAG !=
          query->get_device_info.which_request) {
    // set the relevant tags for error
    result.which_response = MANAGER_RESULT_GET_DEVICE_INFO_TAG;
    fill_device_info_unknown_error(&result.get_device_info, 1);
  } else {
    result.which_response = MANAGER_RESULT_GET_DEVICE_INFO_TAG;
    result.get_device_info = get_device_info();
  }

  ASSERT(encode_manager_result(&result, response, sizeof(response), &msg_size));
  usb_send_msg(response, msg_size);
}