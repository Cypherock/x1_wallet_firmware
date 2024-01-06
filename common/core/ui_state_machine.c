/**
 * @file    ui_state_machine.c
 * @author  Cypherock X1 Team
 * @brief   Helpers to listen for UI events and advance the state of a flow
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
#include "ui_state_machine.h"

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

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
#include <stdint.h>

#include "events.h"

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
uint32_t get_state_on_confirm_scr(uint32_t state_on_confirmation,
                                  uint32_t state_on_rejection,
                                  uint32_t state_on_p0_event) {
  uint32_t next_state = state_on_rejection;
  evt_status_t event = get_events(EVENT_CONFIG_UI, MAX_INACTIVITY_TIMEOUT);

  if (true == event.p0_event.flag) {
    next_state = state_on_p0_event;
  } else {
    ui_event_types_t ui_event = event.ui_event.event_type;
    if (UI_EVENT_REJECT == ui_event) {
      next_state = state_on_rejection;
    } else if (UI_EVENT_CONFIRM == ui_event) {
      next_state = state_on_confirmation;
    }
  }

  return next_state;
}

uint32_t get_state_on_input_scr(uint32_t state_on_text_input,
                                uint32_t state_on_rejection,
                                uint32_t state_on_p0_event) {
  uint32_t next_state = state_on_rejection;
  evt_status_t event = get_events(EVENT_CONFIG_UI, MAX_INACTIVITY_TIMEOUT);

  if (true == event.p0_event.flag) {
    next_state = state_on_p0_event;
  } else {
    ui_event_types_t ui_event = event.ui_event.event_type;
    if (UI_EVENT_REJECT == ui_event) {
      next_state = state_on_rejection;
    } else if (UI_EVENT_TEXT_INPUT == ui_event) {
      next_state = state_on_text_input;
    }
  }

  return next_state;
}

uint32_t get_state_on_list_scr(uint32_t state_on_menu_input,
                               uint32_t state_on_rejection,
                               uint32_t state_on_p0_event,
                               uint16_t *list_choice) {
  uint32_t next_state = state_on_rejection;
  evt_status_t event = get_events(EVENT_CONFIG_UI, MAX_INACTIVITY_TIMEOUT);

  if (true == event.p0_event.flag) {
    next_state = state_on_p0_event;
  } else {
    ui_event_types_t ui_event = event.ui_event.event_type;
    if (UI_EVENT_REJECT == ui_event) {
      next_state = state_on_rejection;
    } else if (UI_EVENT_LIST_CHOICE == ui_event) {
      next_state = state_on_menu_input;
      if (NULL != list_choice) {
        *list_choice = event.ui_event.list_selection;
      }
    }
  }

  return next_state;
}
