/**
 * @file    ui_events.c
 * @author  Cypherock X1 Team
 * @brief   UI Events module
 *          Provides UI event setters and getters for different ui events.
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
static ui_event_t ui_event;

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
bool ui_get_and_reset_event(ui_event_t *ui_event_os_obj) {
  if (ui_event_os_obj == NULL) {
    return false;
  }

  if (ui_event.event_occured) {
    memcpy(ui_event_os_obj, &ui_event, sizeof(ui_event_t));
    ui_reset_event();
    return true;
  }

  return false;
}

void ui_reset_event() {
  memzero(&ui_event, sizeof(ui_event));
}

void ui_set_confirm_event() {
  ui_event.event_occured = true;
  ui_event.event_type = UI_EVENT_CONFIRM;
  return;
}

void ui_set_cancel_event() {
  ui_event.event_occured = true;
  ui_event.event_type = UI_EVENT_REJECT;
  return;
}

void ui_set_list_event(uint16_t list_selection) {
  ui_event.event_occured = true;
  ui_event.event_type = UI_EVENT_LIST_CHOICE;
  ui_event.list_selection = list_selection;
  return;
}

void ui_set_text_input_event(char *text_ptr) {
  if (text_ptr == NULL) {
    return;
  }

  ui_event.event_occured = true;
  ui_event.event_type = UI_EVENT_TEXT_INPUT;
  ui_event.text_ptr = text_ptr;
  return;
}

void ui_fill_text(const char *text_src_ptr,
                  char *input_text_ptr,
                  const size_t max_text_len) {
  if ((text_src_ptr != NULL) && (input_text_ptr != NULL)) {
    snprintf(input_text_ptr, max_text_len, "%s", text_src_ptr);
  }
  return;
}
