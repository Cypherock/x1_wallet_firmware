/**
 * @file    usb_api.c
 * @author  Cypherock X1 Team
 * @brief   USB communication interface.
 *          Handles all USB communication operations for the application from
 *the ISR context.
 * @copyright Copyright (c) 2022-2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022-2023 by HODL TECH PTE LTD
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

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "board.h"
#include "communication.h"
#if USE_SIMULATOR == 0
#include "libusb.h"
#endif
#include "assert_conf.h"
#include "core.pb.h"
#include "logger.h"
#include "pb_encode.h"
#include "status_api.h"
#include "sys_state.h"
#include "usb_api_priv.h"
#include "utils.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/**
 * @brief struct for raw serialized data received over usb
 */
typedef struct comm_raw_payload {
  uint32_t msg_type;
  uint8_t *msg_data;
} comm_raw_payload_t;

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

uint8_t comm_io_buffer[COMM_BUFFER_SIZE] = {0};
comm_payload_t comm_payload;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

static bool is_there_any_msg_from_app();

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool is_there_any_msg_from_app() {
  return get_comm_status()->curr_cmd_state == CMD_STATE_RECEIVED;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void usb_init() {
#if USE_SIMULATOR == 0
  lusb_register_parserFunction(comm_packet_parser);
#endif
}

uint8_t *get_io_buffer() {
  return comm_io_buffer;
}

comm_payload_t *get_comm_payload() {
  return &comm_payload;
}

void mark_device_state(cy_app_status_t state, uint8_t flow_status) {
  comm_status_t *comm_status = get_comm_status();
  uint8_t usb_irq_enable = NVIC_GetEnableIRQ(OTG_FS_IRQn);
  NVIC_DisableIRQ(OTG_FS_IRQn);
  if (state != CY_UNUSED_STATE) {
    comm_status->app_busy_status = state;

    /**
     * If app state is idle, active interface is cleared to allow recieving
     * commands from any interface. If app state is not idle, it means an
     * application/flow is already in progress and command from a new interface
     * shouldn't be allowed before tasks of the app have been completed or app
     * is closed.
     */
    if (CY_APP_IDLE_TASK == (comm_status->app_busy_status & CY_APP_IDLE_TASK))
      comm_reset_interface();
  }
  if (flow_status != 0xFF)
    comm_status->curr_flow_status = flow_status;
  comm_status->abort_disabled = CY_reset_not_allowed();
  if (usb_irq_enable == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);
}

bool is_device_ready() {
  return get_comm_status()->app_busy_status & CY_APP_IDLE;
}

void comm_reject_request(En_command_type_t command_type, uint8_t byte) {
  comm_status_t *comm_status = get_comm_status();
  uint8_t arr[1] = {byte};
  uint8_t usb_irq_enable = NVIC_GetEnableIRQ(OTG_FS_IRQn);

  NVIC_DisableIRQ(OTG_FS_IRQn);
  // Make sure to set he curr_cmd_state to CMD_STATE_FAILED
  transmit_data_to_app(command_type, arr, 1);
  // Imp: Should be updated after writing to buffer
  comm_status->curr_cmd_state = CMD_STATE_FAILED;
  comm_status->app_busy_status = CY_APP_IDLE | CY_APP_IDLE_TASK;
  // App state is set to idle here, so new command is allowed from any
  // interfaces
  comm_reset_interface();
  if (usb_irq_enable == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);
}

void usb_send_byte(const uint32_t command_type, const uint8_t byte) {
  uint8_t arr[1] = {byte};
  transmit_data_to_app(command_type, arr, 1);
}

void usb_send_data(const uint32_t command_type,
                   const uint8_t *transmit_data,
                   const uint32_t size) {
  return;
}

void usb_send_msg(const uint8_t *core_msg,
                  uint32_t core_msg_size,
                  const uint8_t *app_msg,
                  uint32_t app_msg_size) {
  uint8_t usb_irq_enable = NVIC_GetEnableIRQ(OTG_FS_IRQn);

  NVIC_DisableIRQ(OTG_FS_IRQn);
  usb_clear_event();
  get_comm_status()->curr_cmd_state = CMD_STATE_DONE;

  // catch the buffer overflow situation
  ASSERT((COMM_SZ_RESERVED_SPACE + core_msg_size + app_msg_size) <=
         COMM_BUFFER_SIZE);
  comm_set_payload_struct(core_msg_size, app_msg_size);

  // write stream lengths into payload buffer as follows
  // core_msg_len (2-bytes) : app_msg_len (2-bytes) : core_msg : app_msg
  // write core msg length into payload buffer
  comm_io_buffer[0] = (core_msg_size >> 8) & 0xFF;
  comm_io_buffer[1] = core_msg_size & 0xFF;
  // write app msg length into payload buffer
  comm_io_buffer[2] = (app_msg_size >> 8) & 0xFF;
  comm_io_buffer[3] = app_msg_size & 0xFF;

  if (0 < core_msg_size && NULL != core_msg) {
    // copy core message into payload buffer after COMM_SZ_RESERVED_SPACE
    memcpy(comm_io_buffer + COMM_SZ_RESERVED_SPACE, core_msg, core_msg_size);
  }

  if (0 < app_msg_size && NULL != app_msg) {
    // copy app message into payload buffer after core-msg
    // COMM_SZ_RESERVED_SPACE + core_msg_len
    memcpy(comm_io_buffer + COMM_SZ_RESERVED_SPACE + core_msg_size,
           app_msg,
           app_msg_size);
  }

  if (usb_irq_enable == true)
    NVIC_EnableIRQ(OTG_FS_IRQn);
}

void usb_free_msg_buffer() {
  sys_flow_cntrl_u.bits.usb_buffer_free = true;
  comm_set_payload_struct(0, 0);
  LOG_SWV("%s\n", __func__);
}

void usb_set_state_executing() {
  get_comm_status()->curr_cmd_state = CMD_STATE_EXECUTING;
}

void usb_reset_state() {
  get_comm_status()->curr_cmd_state = CMD_STATE_NONE;
}

bool usb_get_msg(En_command_type_t *command_type,
                 uint8_t **msg_data,
                 uint16_t *msg_len) {
  if ((msg_len == NULL && msg_data != NULL) ||
      (msg_len != NULL && msg_data == NULL))
    return false;
  if (is_there_any_msg_from_app()) {
    // TODO: Handle hybrid (raw and protobuf together) messages here
    if (command_type)
      *command_type = U32_READ_BE_ARRAY(comm_payload.raw_data);
    if (msg_data)
      *msg_data = comm_payload.raw_data + sizeof(uint32_t);
    if (msg_len)
      *msg_len = comm_payload.raw_data_length - sizeof(uint32_t);
    return true;
  }
  return false;
}

bool get_usb_msg_by_cmd_type(En_command_type_t command_type,
                             uint8_t **msg_data,
                             uint16_t *msg_len) {
  if ((msg_len == NULL && msg_data != NULL) ||
      (msg_len != NULL && msg_data == NULL))
    return false;
  if (is_there_any_msg_from_app() &&
      U32_READ_BE_ARRAY(comm_payload.raw_data) == command_type) {
    // TODO: Handle hybrid (raw and protobuf together) messages here
    if (msg_data)
      *msg_data = comm_payload.raw_data + sizeof(uint32_t);
    if (msg_len)
      *msg_len = comm_payload.raw_data_length - sizeof(uint32_t);
    return true;
  }
  return false;
}

void comm_set_payload_struct(uint16_t proto_len, uint16_t raw_len) {
  comm_payload.proto_data_length = proto_len;
  comm_payload.raw_data_length = raw_len;
  comm_payload.proto_data =
      proto_len ? comm_io_buffer + sizeof(uint16_t) * 2 : NULL;
  comm_payload.raw_data = raw_len ? comm_io_buffer + 2 * sizeof(uint16_t) +
                                        comm_payload.proto_data_length
                                  : NULL;
}
