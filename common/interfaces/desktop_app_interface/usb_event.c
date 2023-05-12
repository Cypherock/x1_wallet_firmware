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

#include "core.pb.h"
#include "memzero.h"
#include "pb_decode.h"
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

typedef struct usb_core_msg {
  const uint8_t *buffer;
  uint16_t size;
} usb_core_msg_t;

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

static usb_event_t usb_event;
static usb_core_msg_t core_msg;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Validates that the received usb event is valid in the current context
 * of the application/OS.
 *
 * @param evt The captured usb event usb_event_t
 * @param msg The received encoded context buffer
 */
static core_error_type_t validate_msg_context(usb_core_msg_t msg);

/**
 * @brief Clear the fields of core_msg by setting default values
 */
static void clear_msg_context();

/**
 * @brief Reset the members of usb_event_t to decided default state.
 * @details The instance after reset will represent the absence of any usb
 * event as well as the members will be invalid/meaningless content.
 *
 * @param evt Pass the reference to an instance of usb_event_t to reset
 */
static void reset_event_obj(usb_event_t *evt);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void reset_event_obj(usb_event_t *evt) {
  evt->flag = false;
  evt->msg_size = 0;
  evt->p_msg = NULL;
}

static void clear_msg_context() {
  core_msg.size = 0;
  core_msg.buffer = NULL;
}

static core_error_type_t validate_msg_context(usb_core_msg_t msg) {
  core_msg_t core_msg_p = CORE_MSG_INIT_ZERO;
  pb_istream_t stream = pb_istream_from_buffer(msg.buffer, msg.size);

  // invalid buffer ref, 0 size & decode failure are error situation
  if (false == pb_decode(&stream, CORE_MSG_FIELDS, &core_msg_p) ||
      NULL == msg.buffer || 0 == msg.size ||
      CORE_MSG_CMD_TAG != core_msg_p.which_type) {
    return CORE_ERROR_TYPE_INVALID_MSG;
  }

  // TODO: verify if the core_msg_p.type.cmd.applet_id is valid one ref PR #235
  // const cy_app_desc_t *app_desc =
  // registry_get_app_desc(core_msg_p.type.cmd.applet_id);

  // if (NULL == app_desc) {
  //   return CORE_ERROR_TYPE_UNKNOWN_APP;
  // }

  // TODO: verify with core context, if applet is invokable/active
  return CORE_ERROR_TYPE_NO_ERROR;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void usb_clear_event() {
  reset_event_obj(&usb_event);
  clear_msg_context();
  usb_free_msg_buffer();
  usb_reset_state();
}

void usb_set_event(const uint16_t core_msg_size,
                   const uint8_t *core_msg_buffer,
                   const uint16_t app_msg_size,
                   const uint8_t *app_msg) {
  usb_event.flag = true;
  usb_event.msg_size = app_msg_size;
  usb_event.p_msg = app_msg;

  core_msg.buffer = core_msg_buffer;
  core_msg.size = core_msg_size;
}

bool usb_get_event(usb_event_t *evt) {
  if (evt == NULL) {
    return false;
  }

  reset_event_obj(evt);

  if (usb_event.flag) {
    core_error_type_t status = validate_msg_context(core_msg);
    if (CORE_ERROR_TYPE_NO_ERROR != status) {
      // now clear event as it is not supposed to reach the app
      usb_clear_event();
      // TODO: send an error to host
    } else {
      memcpy(evt, &usb_event, sizeof(usb_event_t));
      usb_set_state_executing();
    }
  }
  return evt->flag;
}