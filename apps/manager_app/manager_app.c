/**
 * @file    manager_app.c
 * @author  Cypherock X1 Team
 * @brief
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
#include "manager_app.h"

#include "app_registry.h"
#include "manager_api.h"
#include "manager_app_priv.h"
#include "onboarding.h"
#include "status_api.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
// Macro used to populate manager app descriptor with id and version pre-defined
#define MANAGER_APP_DESCRIPTOR(app, app_config)                                \
  { 1, {1, 0, 0}, app, app_config }
/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Entry point for the manager application of the X1 vault. It is invoked
 * by the X1 vault firmware, as soon as there is a USB request raised for the
 * manager app.
 *
 * @param usb_evt The USB event which triggered invocation of the manager app
 */
void manager_app_main(usb_event_t usb_evt, const void *app_config);

/**
 * @brief Restricted Entry point for the manager application of the X1 vault. It
 * is invoked by the X1 vault firmware, as soon as there is a USB request raised
 * for the manager app.
 * @note It only allows some functionality such as get device info, device
 * authentication and firmware update
 *
 * @param usb_evt The USB event which triggered invocation of the manager app
 */
void manager_app_restricted_main(usb_event_t usb_evt, const void *app_config);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static const cy_app_desc_t manager_desc =
    MANAGER_APP_DESCRIPTOR(manager_app_main, NULL);

static const cy_app_desc_t manager_restricted_desc =
    MANAGER_APP_DESCRIPTOR(manager_app_restricted_main, NULL);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
void manager_app_main(usb_event_t usb_evt, const void *app_config) {
  manager_query_t query = MANAGER_QUERY_INIT_ZERO;

  if (!decode_manager_query(usb_evt.p_msg, usb_evt.msg_size, &query)) {
    return;
  }

  /* Set status to CORE_DEVICE_IDLE_STATE_USB to indicate host that we are now
   * servicing a USB initiated command */
  core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_USB);

  switch ((uint8_t)query.which_request) {
    case MANAGER_QUERY_GET_DEVICE_INFO_TAG: {
      get_device_info_flow(&query);
      break;
    }
    case MANAGER_QUERY_GET_WALLETS_TAG: {
      manager_export_wallets(&query);
      break;
    }
    case MANAGER_QUERY_AUTH_DEVICE_TAG: {
      device_authentication_flow(&query);
      break;
    }
    case MANAGER_QUERY_AUTH_CARD_TAG: {
      card_auth_handler(&query);
      break;
    }
    case MANAGER_QUERY_GET_LOGS_TAG: {
      manager_get_logs(&query);
      break;
    }
    case MANAGER_QUERY_TRAIN_JOYSTICK_TAG: {
      manager_joystick_training(&query);
      break;
    }
    case MANAGER_QUERY_TRAIN_CARD_TAG: {
      manager_card_training(&query);
      break;
    }
    case MANAGER_QUERY_FIRMWARE_UPDATE_TAG: {
      manager_confirm_firmware_update(&query);
      break;
    }
    case MANAGER_QUERY_SELECT_WALLET_TAG: {
      manager_wallet_selector(&query);
      break;
    }
    default: {
      /* In case we ever encounter invalid query, convey to the host app */
      manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_QUERY);
      break;
    }
  }

  onboarding_set_static_screen();

  return;
}

void manager_app_restricted_main(usb_event_t usb_evt, const void *app_config) {
  manager_query_t query = MANAGER_QUERY_INIT_ZERO;

  if (!decode_manager_query(usb_evt.p_msg, usb_evt.msg_size, &query)) {
    return;
  }

  /* Set status to CORE_DEVICE_IDLE_STATE_USB to indicate host that we are now
   * servicing a USB initiated command */
  core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_USB);

  switch ((uint8_t)query.which_request) {
    case MANAGER_QUERY_GET_DEVICE_INFO_TAG: {
      get_device_info_flow(&query);
      break;
    }
    case MANAGER_QUERY_AUTH_DEVICE_TAG: {
      device_authentication_flow(&query);
      break;
    }
    case MANAGER_QUERY_FIRMWARE_UPDATE_TAG: {
      manager_confirm_firmware_update(&query);
      break;
    }
    case MANAGER_QUERY_GET_LOGS_TAG: {
      manager_get_logs(&query);
      break;
    }
    case MANAGER_QUERY_GET_WALLETS_TAG:
    case MANAGER_QUERY_AUTH_CARD_TAG:
    case MANAGER_QUERY_TRAIN_JOYSTICK_TAG:
    case MANAGER_QUERY_TRAIN_CARD_TAG:
    default: {
      /* In case we ever encounter invalid query, convey to the host app */
      manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_QUERY);
      break;
    }
  }

  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
const cy_app_desc_t *get_manager_app_desc() {
  return &manager_desc;
}

const cy_app_desc_t *get_restricted_manager_app_desc() {
  return &manager_restricted_desc;
}