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

#include "app_registry.h"
#include "core.pb.h"
#include "core_api.h"
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
// TODO: Following will be replaced when core starts maintaining it
static uint32_t applet_id = 0;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * Thid function decodes a USB core message and returns the type of the message
 * and any associated errors.
 *
 * @param msg The parameter `msg` is of type `usb_core_msg_t`, which is a
 * structure containing a buffer and its size. It is used to hold the message
 * data that needs to be decoded.
 * @param request_type A pointer to a variable of type `size_t` where the
 * request type will be stored.
 *
 * @return a value of type `core_error_type_t`.
 */
static core_error_type_t get_core_req_type(usb_core_msg_t msg,
                                           size_t *request_type);

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

/**
 * @brief This function populates a response structure with the versions of all
 * registered apps.
 *
 * @param response A pointer to a structure of type
 * `core_app_version_result_response_t`. This structure contains an array
 * `app_versions` of type `core_app_version_t`, which represents the list of app
 * versions. The structure also contains an integer `app_versions_count` to
 * store the count of app versions.
 */
static void populate_version_list(core_app_version_result_response_t *response);

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

static core_error_type_t get_core_req_type(usb_core_msg_t msg,
                                           size_t *request_type) {
  core_msg_t core_msg_p = CORE_MSG_INIT_ZERO;
  pb_istream_t stream = pb_istream_from_buffer(msg.buffer, msg.size);
  core_error_type_t status = CORE_INVALID_MSG;
  // invalid buffer ref, 0 size & decode failure are error situation
  if (false == pb_decode(&stream, CORE_MSG_FIELDS, &core_msg_p) ||
      NULL == msg.buffer || 0 == msg.size) {
    return status;
  }

  switch (core_msg_p.which_type) {
    case CORE_MSG_CMD_TAG: {
      // store applet id
      applet_id = core_msg_p.cmd.applet_id;
      status = CORE_NO_ERROR;
    } break;

    case CORE_MSG_APP_VERSION_TAG: {
      if (CORE_APP_VERSION_CMD_REQUEST_TAG ==
          core_msg_p.app_version.which_cmd) {
        status = CORE_NO_ERROR;
      }
    } break;

    default:
      break;
  }

  if (NULL != request_type) {
    *request_type = core_msg_p.which_type;
  }

  return status;
}

static void populate_version_list(
    core_app_version_result_response_t *response) {
  const cy_app_desc_t **desc_list = registry_get_app_list();
  uint8_t app_count = 0;

  for (int i = 0; i < REGISTRY_MAX_APPS; i++) {
    if (NULL == desc_list[i]) {
      continue;
    }

    response->app_versions[app_count].id = desc_list[i]->id;
    response->app_versions[app_count].has_version = true;
    memcpy(&(response->app_versions[app_count].version),
           &(desc_list[i]->version),
           sizeof(common_version_t));
    app_count++;
  }

  response->app_versions_count = app_count;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

uint32_t get_applet_id(void) {
  return applet_id;
}

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

  size_t request_type = 0;
  reset_event_obj(evt);

  if (usb_event.flag) {
    core_error_type_t status = get_core_req_type(core_msg, &request_type);
    if (CORE_NO_ERROR != status) {
      // now clear event as it is not supposed to reach the app
      usb_clear_event();
      send_core_error_msg_to_host(status);
    } else {
      if (CORE_MSG_CMD_TAG == request_type) {
        memcpy(evt, &usb_event, sizeof(usb_event_t));
        usb_set_state_executing();
      } else if (CORE_MSG_APP_VERSION_TAG == request_type) {
        /**
         * The applet verison list request is processed only when @ref
         * usb_get_event api is called, limitation of this implementation is
         * that if device is not actively looking for usb events then the host
         * will not be able to receive version response until @ref usb_get_event
         * is called. An alternate approach is to process the version request
         * when calling the @ref usb_set_event api, so it is processed through
         * within the interrupt thread and the version request is processed at
         * any time, regardless of device operation or state.
         */

        core_app_version_result_response_t resp;
        populate_version_list(&resp);
        send_app_version_list_to_host(&resp);
        reset_event_obj(&usb_event);
      }
    }
  }
  return evt->flag;
}