/**
 * @file    exchange_main.c
 * @author  Cypherock X1 Team
 * @brief   A common entry point to various Exchange actions supported.
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

#include "exchange_main.h"

#include "exchange/core.pb.h"
#include "exchange_api.h"
#include "exchange_priv.h"
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
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Entry point for the EXCHANGE application of the X1 vault. It is
 * invoked by the X1 vault firmware, as soon as there is a USB request raised
 * for the Exchange app.
 *
 * @param usb_evt The USB event which triggered invocation of the exchange
 * app
 */
void exchange_main(usb_event_t usb_evt, const void *app_config);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static const cy_app_desc_t exchange_app_desc = {.id = 24,
                                                .version =
                                                    {
                                                        .major = 1,
                                                        .minor = 0,
                                                        .patch = 0,
                                                    },
                                                .app = exchange_main,
                                                .app_config = NULL};

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
void exchange_main(usb_event_t usb_evt, const void *app_config) {
  exchange_query_t query = EXCHANGE_QUERY_INIT_DEFAULT;

  if (false == decode_exchange_query(usb_evt.p_msg, usb_evt.msg_size, &query)) {
    return;
  }

  /* Set status to CORE_DEVICE_IDLE_STATE_USB to indicate host that we are now
   * servicing a USB initiated command */
  core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_USB);

  LOG_SWV("%s (%d) - Query:%d\n", __func__, __LINE__, query.which_request);
  switch ((uint8_t)query.which_request) {
    default: {
      /* In case we ever encounter invalid query, convey to the host app */
      exchange_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                          ERROR_DATA_FLOW_INVALID_QUERY);
    } break;
  }
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
const cy_app_desc_t *get_exchange_app_desc() {
  return &exchange_app_desc;
}
