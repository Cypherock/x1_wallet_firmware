/**
 * @file    ui_events.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) ${YEAR} HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
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
#include "ui_events.h"

#include <string.h>

#include "memzero.h"

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
static ui_event_status_t event_status;
static ui_event_t event;

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
void ui_status_mark_ready_for_events() {
  memzero(&event, sizeof(event));
  event_status = UI_EVENT_SCR_RENDERED;
}

void ui_get_events(ui_event_t *ui_event_os_obj) {
  memcpy(ui_event_os_obj, &event, sizeof(ui_event_t));
}

void ui_status_reset_event_state() {
  memzero(&event, sizeof(event));
  event_status = UI_EVENT_NONE;
}

bool ui_set_confirm_event() {
  if (event_status != UI_EVENT_SCR_RENDERED)
    return false;    // Invalid state return

  event.event_occured = SEC_TRUE;
  event.event_type = UI_EVENT_CONFIRM;
}

bool ui_set_cancel_event() {
  if (event_status != UI_EVENT_SCR_RENDERED)
    return false;    // Invalid state return

  event.event_occured = SEC_TRUE;
  event.event_type = UI_EVENT_REJECT;
}

bool ui_set_list_event(uint8_t list_selection) {
  if (event_status != UI_EVENT_SCR_RENDERED)
    return false;    // Invalid state return

  event.event_occured = SEC_TRUE;
  event.event_type = UI_EVENT_LIST_CHOICE;
  event.list_selection = list_selection;
}

bool ui_set_text_input_event(char *text_ptr) {
  ASSERT(text_ptr != NULL);
  if (event_status != UI_EVENT_SCR_RENDERED)
    return false;    // Invalid state return

  event.event_occured = SEC_TRUE;
  event.event_type = UI_EVENT_TEXT_INPUT;
  event.text_ptr = text_ptr;
}