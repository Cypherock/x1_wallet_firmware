/**
 * @file    status_api.c
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
#include "status_api.h"

#include "usb_api_priv.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define CORE_STATUS_MASK 0xFF
#define CORE_STATUS_SHIFT 8

#define APP_STATUS_MASK 0xFF
#define APP_STATUS_SHIFT 0

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
core_status_t core_status = CORE_STATUS_INIT_ZERO;

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
void core_status_set_idle_state(core_device_idle_state_t idle_state) {
  core_status.device_idle_state = idle_state;

  /**
   * If app state is idle, active interface is cleared to allow recieving
   * commands from any interface. If app state is not idle, it means an
   * application/flow is already in progress and command from a new interface
   * shouldn't be allowed before tasks of the app have been completed or app
   * is closed.
   */
  if (CORE_DEVICE_IDLE_STATE_IDLE == core_status.device_idle_state)
    comm_reset_interface();

  if (CORE_DEVICE_IDLE_STATE_USB == core_status.device_idle_state) {
    core_status.abort_disabled = false;
  } else {
    core_status.abort_disabled = true;
  }
  return;
}

void set_core_flow_status(uint32_t status) {
  core_status.flow_status &= ~(CORE_STATUS_MASK << CORE_STATUS_SHIFT);
  core_status.flow_status |= ((status & CORE_STATUS_MASK) << CORE_STATUS_SHIFT);
  return;
}

void set_app_flow_status(uint32_t status) {
  core_status.flow_status &= ~(APP_STATUS_MASK << APP_STATUS_SHIFT);
  core_status.flow_status |= ((status & APP_STATUS_MASK) << APP_STATUS_SHIFT);
  return;
}

void core_status_set_device_waiting_on(core_device_waiting_on_t waiting_on) {
  core_status.device_waiting_on = waiting_on;
  return;
}

bool core_status_get_abort_disabled(void) {
  if (CORE_DEVICE_IDLE_STATE_USB == core_status.device_idle_state) {
    return true;
  }
  return false;
}

core_status_t get_core_status(void) {
  return core_status;
}
