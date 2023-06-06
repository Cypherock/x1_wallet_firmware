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

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void manager_app_main(usb_event_t usb_evt) {
  manager_query_t query = MANAGER_QUERY_INIT_ZERO;

  if (false == decode_manager_query(usb_evt.p_msg, usb_evt.msg_size, &query)) {
    return;
  }

  /* Set status to CORE_DEVICE_IDLE_STATE_USB to indicate host that we are now
   * servicing a USB initiated command */
  core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_USB);

  LOG_SWV("%s (%d) - Query:%d\n", __func__, __LINE__, query.which_request);

  // TODO: Add calls to flows/ functions based on query type decoded from the
  // protobuf
  switch ((uint8_t)query.which_request) {
    case MANAGER_QUERY_GET_DEVICE_INFO_TAG: {
      get_device_info_flow(&query);
      break;
    }
    case MANAGER_QUERY_GET_WALLETS_TAG: {
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
    default: {
      /* In case we ever encounter invalid query, the USB event should be
       * cleared manually */
      usb_clear_event();
      break;
    }
  }

  // TODO: Check if on-boarding default screen is to be rendered
  onboarding_set_static_screen();

  return;
}
