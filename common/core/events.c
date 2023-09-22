/**
 * @file    events.c
 * @author  Cypherock X1 Team
 * @brief   Source file for the event getter module
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
#include "events.h"

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
evt_status_t get_events(uint8_t event_config, uint32_t timeout) {
  evt_status_t status = {0};

  p0_ctx_init(timeout);

  bool p0_evt_occurred = false;
  bool p1_evt_occurred = false;

  /* Poll for the selected events, until atleast one event is captured. */
  while (1) {
    p0_evt_occurred = p0_get_evt(&(status.p0_event));

    /* As soon as a p0 event is registered, break the loop */
    if (p0_evt_occurred) {
      break;
    }

    if (EVENT_CONFIG_UI == (event_config & EVENT_CONFIG_UI)) {
      // Check user activity on joystick before it is cleared by lv_task_handler
      if (keypad_get_key()) {
        // Refresh the timeout as a user activity is detected on the joystick
        p0_ctx_init(timeout);
      }

      lv_task_handler();
      p1_evt_occurred |= ui_get_and_reset_event(&(status.ui_event));
    }

    if (EVENT_CONFIG_USB == (event_config & EVENT_CONFIG_USB)) {
      p1_evt_occurred |= usb_get_event(&(status.usb_event));
    }

    if (EVENT_CONFIG_NFC == (event_config & EVENT_CONFIG_NFC)) {
      nfc_task_handler();
      p1_evt_occurred |= nfc_get_event(&(status.nfc_event));
    }

    /* In each loop, provide 50ms delay for things to stabilize, for example USB
     * interrupts, OLED display, etc */
    BSP_DelayMs(50);

    /* As soon as an event is registered, break the loop */
    if (p1_evt_occurred) {
      break;
    }
  }

  /* Any post cleanup required */
  p0_ctx_destroy();

  if (EVENT_CONFIG_NFC == (event_config & EVENT_CONFIG_NFC)) {
    nfc_ctx_destroy();
  }

  return status;
}