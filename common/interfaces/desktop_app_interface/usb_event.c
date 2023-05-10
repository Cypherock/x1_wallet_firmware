/**
 * @file    usb_event.c
 * @author  Cypherock X1 Team
 * @brief   USB Event APIs.
 *          Describes all the logic for interfacing with USB Events.
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
#include <string.h>

#include "memzero.h"
#include "usb_api.h"
#include "usb_api_priv.h"

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
static usb_event_t usb_event;
static const uint8_t *request_meta;
static uint16_t request_meta_size;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Reset the members of usb_event_t to decided default state.
 * @details The instance after reset will represent the absence of any usb
 * event as well as the members will be invalid/meaningless content.
 *
 * @param evt Pass the reference to an instance of usb_event_t to reset
 */
void reset_event_obj(usb_event_t *evt);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
void reset_event_obj(usb_event_t *evt) {
  evt->flag = false;
  evt->msg_size = 0;
  evt->p_msg = NULL;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void usb_clear_event() {
  reset_event_obj(&usb_event);
  usb_free_msg_buffer();
  usb_reset_state();
}

void usb_set_event(const uint16_t proto_msg_size,
                   const uint8_t *proto_buf,
                   const uint16_t raw_msg_size,
                   const uint8_t *raw_msg) {
  usb_event.flag = true;
  usb_event.msg_size = raw_msg_size;
  usb_event.p_msg = raw_msg;
  request_meta = proto_buf;
  request_meta_size = proto_msg_size;
}

bool usb_get_event(usb_event_t *evt) {
  if (evt == NULL) {
    return false;
  }

  reset_event_obj(evt);
  if (usb_event.flag) {
    memcpy(evt, &usb_event, sizeof(usb_event_t));
    usb_set_state_executing();
  }
  return evt->flag;
}