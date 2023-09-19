/**
 * @file    core_api.c
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the application layer
 *
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
#include "core_api.h"

#include <core.pb.h>

#include "assert_conf.h"
#include "pb_encode.h"
#include "status_api.h"
#include "usb_api.h"

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
/**
 * The function encodes a `core_msg_t` structure using Protocol Buffers and
 * sends it over USB along with a message.
 *
 * @param core_msg A pointer to a structure of type `core_msg_t`. This structure
 * contains the data that needs to be encoded and sent.
 * @param msg The `msg` parameter is a pointer to an array of `uint8_t` type,
 * which represents the message data to be sent.
 * @param msg_size Size of the messsage to be sent.
 */
static void send_core_msg(core_msg_t *core_msg,
                          const uint8_t *msg,
                          uint32_t msg_size);
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void send_core_msg(core_msg_t *core_msg,
                          const uint8_t *msg,
                          uint32_t msg_size) {
  uint8_t encoded_buffer[CORE_MSG_SIZE] = {0};
  pb_ostream_t stream =
      pb_ostream_from_buffer(encoded_buffer, sizeof(encoded_buffer));
  ASSERT(pb_encode(&stream, CORE_MSG_FIELDS, core_msg));

  usb_send_msg(encoded_buffer, stream.bytes_written, msg, msg_size);
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void send_response_to_host(const uint8_t *msg, const uint32_t size) {
  core_msg_t core_msg = CORE_MSG_INIT_ZERO;
  core_msg.which_type = CORE_MSG_CMD_TAG;

  // TODO: Move applet_id management to core
  core_msg.cmd.applet_id = get_applet_id();

  send_core_msg(&core_msg, msg, size);
  return;
}

void send_core_error_msg_to_host(uint32_t core_error_type) {
  core_msg_t core_msg = CORE_MSG_INIT_ZERO;
  core_msg.which_type = CORE_MSG_ERROR_TAG;
  core_msg.error.type = (core_error_type_t)core_error_type;

  send_core_msg(&core_msg, NULL, 0);
  return;
}

void send_app_version_list_to_host(
    const core_app_version_result_response_t *version_resp) {
  core_msg_t core_msg = CORE_MSG_INIT_ZERO;
  core_msg.which_type = CORE_MSG_APP_VERSION_TAG;
  core_msg.app_version.which_cmd = CORE_APP_VERSION_CMD_RESPONSE_TAG;
  core_msg.app_version.response.which_response =
      CORE_APP_VERSION_RESPONSE_RESULT_TAG;
  memcpy(&(core_msg.app_version.response.result),
         version_resp,
         sizeof(core_app_version_result_response_t));

  send_core_msg(&core_msg, NULL, 0);
  return;
}