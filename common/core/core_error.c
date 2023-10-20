/**
 * @file    core_error.c
 * @author  Cypherock X1 Team
 * @brief   Display and return core errors.
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

#include "core_error.h"

#include "buzzer.h"
#include "constant_texts.h"
#include "core_api.h"
#include "events.h"
#include "p0_events.h"
#include "status_api.h"
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
typedef struct {
  char core_error_msg[60]; /**< Buffer to store the error message that needs to
                              be displayed */
  bool ring_buzzer; /**< Configuration parameter to record if buzzer is required
                       while the error is being displayed */
} error_screen_t;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Display error message to user set by core operations.
 *
 * @details When a core operation faces a fatal error, it can set an error
 * message using using @ref mark_core_error_screen before exiting the flow. This
 * API displays that error message and sets the core device idle status to
 * CORE_DEVICE_IDLE_STATE_DEVICE.
 *
 * NOTE: P0 events are ignored in this API, as this could also be used to
 * display P0 errors
 */
static void display_core_error();

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static error_screen_t error_screen = {0};

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void display_core_error() {
  if (0 ==
      strnlen(error_screen.core_error_msg, sizeof(error_screen.core_error_msg)))
    return;

  evt_status_t status = {0};
  message_scr_init(error_screen.core_error_msg);
  core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_DEVICE);

  if (error_screen.ring_buzzer) {
    buzzer_start(BUZZER_DURATION);
  }

  do {
    status = get_events(EVENT_CONFIG_UI, INFINITE_WAIT_TIMEOUT);
    p0_reset_evt();
  } while (true != status.ui_event.event_occured);

  clear_core_error_screen();
  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void mark_core_error_screen(const char *error_msg, bool ring_buzzer) {
  if (NULL == error_msg) {
    return;
  }

  // Return if an error message is already set
  if (0 != strnlen(error_screen.core_error_msg,
                   sizeof(error_screen.core_error_msg))) {
    return;
  }

  snprintf(error_screen.core_error_msg,
           sizeof(error_screen.core_error_msg),
           "%s",
           error_msg);

  error_screen.ring_buzzer = ring_buzzer;
  return;
}

void handle_core_errors() {
  /* Check P0 events */
  p0_evt_t evt = {0};
  p0_get_evt(&evt);

  if (true == evt.inactivity_evt) {
    // Send response to the host before proceeding further
    if (CORE_DEVICE_IDLE_STATE_USB == get_core_status().device_idle_state) {
      send_core_error_msg_to_host(CORE_APP_TIMEOUT_OCCURRED);
    }

    mark_core_error_screen(ui_text_process_reset_due_to_inactivity, false);
    p0_reset_evt();
  }

  if (true == evt.abort_evt) {
    usb_clear_event();
    p0_reset_evt();
  }

  display_core_error();
  return;
}

bool show_errors_if_p0_not_occured() {
  /* Check P0 events */
  p0_evt_t evt = {0};
  p0_get_evt(&evt);

  // Display errors if there's no p0 event, else it'll be handled by
  // handle_core_errors
  if (false == evt.flag) {
    display_core_error();
  }

  return !evt.flag;
}

void ignore_p0_event() {
  p0_reset_evt();
}

void clear_core_error_screen(void) {
  memzero(&error_screen, sizeof(error_screen_t));
  return;
}