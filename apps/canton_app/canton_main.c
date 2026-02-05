/**
 * @file    canton_main.c
 * @author  Cypherock X1 Team
 * @brief   Common entry point for canton
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

#include "canton_main.h"

#include <stddef.h>
#include <stdint.h>

#include "app_registry.h"
#include "canton/core.pb.h"
#include "canton_api.h"
#include "canton_priv.h"
#include "core.pb.h"
#include "error.pb.h"
#include "status_api.h"
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
 * @brief Entry poont for the canton application of x1 vault. Invoked by x1
 * vault firmware when a USB request is recieved for canton application.
 *
 * @param usb_event The USB event which triggered invocation
 * @param canton_app_config Additional canton application configuration
 */
void canton_main(usb_event_t usb_event, const void *canton_app_config);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

static const cy_app_desc_t canton_app_desc = {
    .id = 26,
    .version = {.major = 1, .minor = 0, .patch = 0},
    .app = canton_main,
    .app_config = NULL};

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

void canton_main(usb_event_t usb_event, const void *canton_app_config) {
  canton_query_t query = CANTON_QUERY_INIT_DEFAULT;
  if (!decode_canton_query(usb_event.p_msg, usb_event.msg_size, &query)) {
    return;
  }

  /* set device status to CORE_DEVICE_IDLE_STATE_USB to indicate host that we
   * are now servicing a usb initiated command */
  core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_USB);

  switch ((uint8_t)query.which_request) {
    case CANTON_QUERY_GET_PUBLIC_KEYS_TAG:
    case CANTON_QUERY_GET_USER_VERIFIED_PUBLIC_KEY_TAG: {
      canton_get_pub_keys(&query);
      break;
    }
    case CANTON_QUERY_SIGN_TXN_TAG: {
      canton_sign_transaction(&query);
      break;
    }

    case CANTON_QUERY_SIGN_TOPOLOGY_TXN_TAG: {
      canton_sign_topology_transaction(&query);
      break;
    }

    /* Incase we encounter an invalid query */
    default: {
      canton_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_QUERY);
      break;
    }
  }
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

const cy_app_desc_t *get_canton_app_desc() {
  return &canton_app_desc;
}
