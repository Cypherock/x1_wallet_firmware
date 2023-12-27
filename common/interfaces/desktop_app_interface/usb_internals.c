/**
 * @file    usb_internal.c
 * @author  Cypherock X1 Team
 * @brief   Handles internal operations of USB.
 *          These are mostly interrupt related routines.
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
#include <core.pb.h>
#include <math.h>
#include <pb.h>

#include "board.h"
#if USE_SIMULATOR == 0
#include "libusb.h"
#else
#include "sim_usb.h"
#endif
#include "logger.h"
#include "p0_events.h"
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

#define COMM_HEADER_INDEX 0
#define COMM_CHECKSUM_INDEX 2
#define COMM_CHUNK_NO_INDEX 4
#define COMM_CHUNK_COUNT_INDEX 6
#define COMM_SEQ_NO_INDEX 8
#define COMM_PKT_TYPE_INDEX 10
#define COMM_TIMESTAMP_INDEX 11
#define COMM_PAYLOAD_LEN_INDEX 15
#define COMM_PAYLOAD_INDEX 16

#define comm_get_raw_payload_size(raw_payload)                                 \
  (raw_payload ? (sizeof(uint32_t) + raw_payload->msg_size) : 0)
#define comm_get_proto_payload_size(proto_payload)                             \
  (proto_payload ? (proto_payload)->size : 0)

/// @details Macro to get the size of the total payload in Cmd Output response.
#define comm_get_payload_size(payload)                                         \
  (2 * sizeof(uint16_t) + payload->raw_data_length + payload->proto_data_length)

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

typedef enum comm_packet_type {
  PKT_TYPE_STATUS_REQ = 1,
  PKT_TYPE_CMD = 2,
  PKT_TYPE_OUT_REQ = 3,
  PKT_TYPE_STATUS_ACK = 4,
  PKT_TYPE_CMD_ACK = 5,
  PKT_TYPE_OUT_RESP = 6,
  PKT_TYPE_ERROR = 7,
  PKT_TYPE_ABORT = 8,
} comm_packet_type;

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

comm_status_t comm_status;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

static comm_error_code_t comm_process_cmd_packet(const packet_t *rx_packet);
static comm_error_code_t comm_process_status_packet(const packet_t *rx_packet);
static comm_error_code_t comm_process_out_req_packet(const packet_t *rx_packet);
static comm_error_code_t comm_process_abort_packet(const packet_t *rx_packet);

static void send_status_packet(const packet_t *rx_packet);
static void send_cmd_ack_packet(const packet_t *rx_packet);
static void send_cmd_output_packet(const packet_t *rx_packet);

static void comm_write_packet(uint16_t chunk_number,
                              uint16_t total_chunks,
                              uint16_t seq_no,
                              uint8_t packet_type,
                              uint8_t payload_size,
                              const uint8_t *payload,
                              comm_libusb__interface_e interface);

static inline void comm_reset() {
  LOG_SWV("comm_reset\n");
  comm_status.curr_cmd_received_length = 0;
  comm_status.curr_cmd_state = CMD_STATE_NONE;
  comm_status.curr_cmd_chunk_no = 0;
}

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static comm_error_code_t comm_process_status_packet(const packet_t *rx_packet) {
  if (rx_packet->header.chunk_number != 1)
    return INVALID_CHUNK_NO;
  if (rx_packet->header.total_chunks != 1)
    return INVALID_CHUNK_COUNT;
  if (rx_packet->header.payload_length != 0)
    return INVALID_PAYLOAD_LENGTH;
  if (rx_packet->header.sequence_no != 0xFFFF)
    return INVALID_SEQUENCE_NO;

  send_status_packet(rx_packet);
  return NO_ERROR;
}

/**
 * @details Packet type: PKT_TYPE_CMD <br/>
 *
 */
static comm_error_code_t comm_process_cmd_packet(const packet_t *rx_packet) {
  uint8_t *comm_io_buffer = get_io_buffer();
  comm_payload_t *comm_payload = get_comm_payload();
  if (!CY_Usb_Buffer_Free())
    return APP_BUFFER_BLOCKED;
  if (comm_status.curr_cmd_state == CMD_STATE_EXECUTING)
    return BUSY_PREVIOUS_CMD;

  // Set active host interface if not set already
  if (comm_status.active_interface == COMM_LIBUSB__UNDEFINED) {
    comm_status.active_interface = rx_packet->interface;
  } else if (comm_status.active_interface != rx_packet->interface) {
    return APP_BUSY_WITH_OTHER_INTERFACE;
  }

  comm_status.curr_cmd_state = CMD_STATE_RECEIVING;
  if (comm_status.curr_cmd_seq_no != rx_packet->header.sequence_no ||
      rx_packet->header.chunk_number == 1)
    comm_reset();    // Clear current status and start new command

  if (comm_status.curr_cmd_chunk_no + 1 < rx_packet->header.chunk_number)
    return OUT_OF_ORDER_CHUNK;
  if (rx_packet->header.chunk_number > rx_packet->header.total_chunks)
    return INVALID_CHUNK_COUNT;

  comm_status.curr_cmd_seq_no = rx_packet->header.sequence_no;
  if (comm_status.curr_cmd_chunk_no + 1 == rx_packet->header.chunk_number) {
    // Duplicate packets are ignored; Only packets in expected sequence are
    // appended to buffer
    comm_status.curr_cmd_chunk_no = rx_packet->header.chunk_number;
    memcpy(comm_io_buffer + comm_status.curr_cmd_received_length,
           rx_packet->payload,
           rx_packet->header.payload_length);
    comm_status.curr_cmd_received_length += rx_packet->header.payload_length;
    if (rx_packet->header.chunk_number == rx_packet->header.total_chunks) {
      // Last chunk received
      if (comm_status.curr_cmd_received_length !=
          (U16_READ_BE_ARRAY(comm_io_buffer) +
           U16_READ_BE_ARRAY(comm_io_buffer + sizeof(uint16_t)) +
           sizeof(uint16_t) * 2)) {
        LOG_SWV("#RED#Invalid payload length: %d + %d + 4 != %d\n",
                U16_READ_BE_ARRAY(comm_io_buffer),
                U16_READ_BE_ARRAY(comm_io_buffer + sizeof(uint16_t)),
                comm_status.curr_cmd_received_length);
        comm_reset();
        return INVALID_PAYLOAD_LENGTH;
      } else {
        sys_flow_cntrl_u.bits.usb_buffer_free = false;
        comm_set_payload_struct(
            U16_READ_BE_ARRAY(comm_io_buffer),
            U16_READ_BE_ARRAY(comm_io_buffer + sizeof(uint16_t)));
      }
    }
  }
  if (rx_packet->header.chunk_number == rx_packet->header.total_chunks) {
    comm_status.curr_cmd_state = CMD_STATE_RECEIVED;
    usb_set_event(comm_payload->proto_data_length,
                  comm_payload->proto_data,
                  comm_payload->raw_data_length,
                  comm_payload->raw_data);
  }
  send_cmd_ack_packet(rx_packet);
  LOG_SWV("#ORG#bs=%d, cs=%d, seq=%d, ccn=%d, ccc=%d, rl=%d\n",
          CY_Usb_Buffer_Free(),
          comm_status.curr_cmd_state,
          comm_status.curr_cmd_seq_no,
          comm_status.curr_cmd_chunk_no,
          rx_packet->header.total_chunks,
          comm_status.curr_cmd_received_length);
  return NO_ERROR;
}

/**
 * @details Packet type: PKT_TYPE_OUT_REQ <br/>
 * Process the cmd output request based on the state of the application. This
 * command cannot intervene with the ongoing command execution; hence, in such a
 * situation, the command is responded with current status (i.e. status packet).
 * The output is expected to be available only when the execution is completed
 * so it is expected that the curr_cmd_state is set to CMD_STATE_DONE.
 */
static comm_error_code_t comm_process_out_req_packet(
    const packet_t *rx_packet) {
  comm_payload_t *comm_payload = get_comm_payload();
  if (comm_status.curr_cmd_seq_no != rx_packet->header.sequence_no)
    return INVALID_SEQUENCE_NO;
  if (rx_packet->header.chunk_number != 1)
    return INVALID_CHUNK_NO;
  if (rx_packet->header.total_chunks != 1)
    return INVALID_CHUNK_COUNT;
  if (rx_packet->header.payload_length != 6)
    return INVALID_PAYLOAD_LENGTH;
  if (comm_status.curr_cmd_state != CMD_STATE_DONE &&
      comm_status.curr_cmd_state != CMD_STATE_FAILED) {
    send_status_packet(rx_packet);
    return NO_ERROR;
  }
  if ((U16_READ_BE_ARRAY(rx_packet->payload + 4) - 1) * COMM_MAX_PAYLOAD_SIZE >
      comm_get_payload_size(comm_payload))
    return NO_MORE_CHUNKS;    // Invalid output chunk request

  send_cmd_output_packet(rx_packet);
  return NO_ERROR;
}

static comm_error_code_t comm_process_abort_packet(const packet_t *rx_packet) {
  if (rx_packet->header.chunk_number != 1)
    return INVALID_CHUNK_NO;
  if (rx_packet->header.total_chunks != 1)
    return INVALID_CHUNK_COUNT;
  if (rx_packet->header.payload_length != 0)
    return INVALID_PAYLOAD_LENGTH;
  if (true == core_status_get_abort_disabled()) {
    comm_reset();
    CY_Reset_Flow();
    p0_set_abort_evt(true);
    comm_status.curr_cmd_seq_no = rx_packet->header.sequence_no;
    sys_flow_cntrl_u.bits.usb_buffer_free = true;
  }

  send_status_packet(rx_packet);
  return NO_ERROR;
}

/**
 * @details Packet type: PKT_TYPE_STATUS_REQ <br/>
 * Respond with the current status of the application. This request will not
 * interrupt the current state of the application. This is a synchronisation
 * mechanism over the USB protocol to help the host identify the precise state
 * of the application so that correct actions can be taken.
 */
static void send_status_packet(const packet_t *rx_packet) {
  uint8_t payload[COMM_MAX_PAYLOAD_SIZE];
  payload[0] = 0x00;
  payload[1] = 0x00;    // proto length
  payload[2] = 0x00;
  payload[3] = 0x00;    // dummy raw length

  // reserve space for length of streams
  core_status_t status = get_core_status();
  pb_ostream_t stream =
      pb_ostream_from_buffer(payload + COMM_SZ_RESERVED_SPACE,
                             sizeof(payload) - COMM_SZ_RESERVED_SPACE);

  // append the info native to comm module; the app-core cannot provide this
  status.current_cmd_seq = comm_status.curr_cmd_seq_no;
  status.cmd_state = comm_status.curr_cmd_state;

  // treat protobuf encoder failure as critical issue
  ASSERT(pb_encode(&stream, &core_status_t_msg, &status));
  payload[0] = (stream.bytes_written >> 8) & 0xFF;
  payload[1] = (stream.bytes_written) & 0xFF;    // proto length
  comm_write_packet(1,
                    1,
                    0xFFFF,
                    PKT_TYPE_STATUS_ACK,
                    stream.bytes_written + COMM_SZ_RESERVED_SPACE,
                    payload,
                    rx_packet->interface);
}

static void send_cmd_ack_packet(const packet_t *rx_packet) {
  uint8_t payload[3 * sizeof(uint16_t)] = {0};
  uint8_t offset = 0;
  payload[offset++] = 0x00;
  payload[offset++] = 0x00;    // proto length
  payload[offset++] = 0x00;
  payload[offset++] = 0x02;    // raw length
  payload[offset++] = (comm_status.curr_cmd_chunk_no >> 8) & 0xFF;
  payload[offset++] = comm_status.curr_cmd_chunk_no & 0xFF;
  comm_write_packet(1,
                    1,
                    rx_packet->header.sequence_no,
                    PKT_TYPE_CMD_ACK,
                    offset,
                    payload,
                    rx_packet->interface);
}

static void send_cmd_output_packet(const packet_t *rx_packet) {
  comm_payload_t *comm_payload = get_comm_payload();
  uint8_t *comm_io_buffer = get_io_buffer();
  ASSERT(comm_payload->raw_data != NULL || comm_payload->proto_data != NULL);
  uint16_t req_chunk_no = U16_READ_BE_ARRAY(
      rx_packet->payload +
      4);    // payload already verified in the caller function
  uint16_t offset = (req_chunk_no - 1) * COMM_MAX_PAYLOAD_SIZE;
  uint16_t remaining_payload_length =
      comm_get_payload_size(comm_payload) - offset;
  uint8_t payload_size =
      CY_MIN(remaining_payload_length, COMM_MAX_PAYLOAD_SIZE);
  comm_write_packet(
      req_chunk_no,
      ceil(comm_get_payload_size(comm_payload) * 1.0 / COMM_MAX_PAYLOAD_SIZE),
      rx_packet->header.sequence_no,
      PKT_TYPE_OUT_RESP,
      payload_size,
      comm_io_buffer + offset,
      rx_packet->interface);
}

static void comm_write_packet(const uint16_t chunk_number,
                              const uint16_t total_chunks,
                              const uint16_t seq_no,
                              const uint8_t packet_type,
                              const uint8_t payload_size,
                              const uint8_t *payload,
                              comm_libusb__interface_e interface) {
  uint8_t buffer[COMM_PKT_MAX_LEN] = {0}, index = 4;
  uint32_t crc = 0;

  buffer[COMM_HEADER_INDEX] = COMM_START_OF_HEADER;
  buffer[COMM_HEADER_INDEX + 1] = COMM_START_OF_HEADER;
  buffer[COMM_CHUNK_NO_INDEX] = (chunk_number >> 8) & 0xFF;
  buffer[COMM_CHUNK_NO_INDEX + 1] = chunk_number & 0xFF;
  buffer[COMM_CHUNK_COUNT_INDEX] = (total_chunks >> 8) & 0xFF;
  buffer[COMM_CHUNK_COUNT_INDEX + 1] = total_chunks & 0xFF;
  buffer[COMM_SEQ_NO_INDEX] = (seq_no >> 8) & 0xFF;
  buffer[COMM_SEQ_NO_INDEX + 1] = seq_no & 0xFF;
  buffer[COMM_PKT_TYPE_INDEX] = packet_type;
  buffer[COMM_TIMESTAMP_INDEX] = (uwTick >> 24) & 0xFF;
  buffer[COMM_TIMESTAMP_INDEX + 1] = (uwTick >> 16) & 0xFF;
  buffer[COMM_TIMESTAMP_INDEX + 2] = (uwTick >> 8) & 0xFF;
  buffer[COMM_TIMESTAMP_INDEX + 3] = uwTick & 0xFF;
  buffer[COMM_PAYLOAD_LEN_INDEX] = payload_size;

  memcpy(buffer + COMM_PAYLOAD_INDEX, payload, payload_size);
  while (index < payload_size + COMM_HEADER_SIZE) {
    crc = update_crc16(crc, buffer[index++]);
  }
  crc = update_crc16(crc, 0);
  crc = update_crc16(crc, 0);
  buffer[COMM_CHECKSUM_INDEX] = (crc >> 8) & 0xFF;
  buffer[COMM_CHECKSUM_INDEX + 1] = crc & 0xFF;
#if USE_SIMULATOR == 1
  SIM_Transmit_FS(buffer, payload_size + COMM_HEADER_SIZE);
#else
  lusb_write(buffer, payload_size + COMM_HEADER_SIZE, interface);
#endif
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void comm_reset_interface(void) {
  comm_status.active_interface = COMM_LIBUSB__UNDEFINED;
  return;
}

comm_status_t *get_comm_status() {
  return &comm_status;
}

void comm_process_packet(const packet_t *rx_packet) {
  static uint8_t temp_type = 0;
  if (temp_type != rx_packet->header.packet_type) {
    temp_type = rx_packet->header.packet_type;
    LOG_SWV("#GRN#Received packet: %d\n", rx_packet->header.packet_type);
  }
  comm_error_code_t proc_error = NO_ERROR;
#if 0
    // TODO: Define meaning/use-case for timestamp on device's end
    if (comm_status.host_sync_time > rx_packet->header.timestamp) {
        comm_status.host_sync_fails++;
        if (comm_status.host_sync_fails > 10) {
            LOG_CRITICAL("cysync restart: %d", comm_status.host_sync_fails);
            comm_status.host_sync_fails = 0;
        }
    } else {
        comm_status.host_sync_time = rx_packet->header.timestamp;
    }
#endif

  switch (rx_packet->header.packet_type) {
    case PKT_TYPE_CMD:
      proc_error = comm_process_cmd_packet(rx_packet);
      break;

    case PKT_TYPE_STATUS_REQ:
      proc_error = comm_process_status_packet(rx_packet);
      break;

    case PKT_TYPE_OUT_REQ:
      proc_error = comm_process_out_req_packet(rx_packet);
      break;

    case PKT_TYPE_ABORT:
      proc_error = comm_process_abort_packet(rx_packet);
      break;

    default:
      proc_error = INVALID_PACKET_TYPE;
      break;
  }
  if (proc_error != NO_ERROR)
    send_error_packet(rx_packet, proc_error);
}

void send_error_packet(const packet_t *rx_packet,
                       const comm_error_code_t error_code) {
  LOG_SWV("#RED#Error: %d\r\n", error_code);
  uint8_t payload[3 * sizeof(uint16_t)] = {0}, offset = 0;
  payload[offset++] = 0x00;
  payload[offset++] = 0x00;    // proto length
  payload[offset++] = 0x00;
  payload[offset++] = 0x02;    // raw length
  payload[offset++] = (error_code >> 8) & 0xFF;
  payload[offset++] = error_code & 0xFF;
  comm_write_packet(
      1, 1, 0xFFFF, PKT_TYPE_ERROR, offset, payload, rx_packet->interface);
}

uint16_t update_crc16(const uint16_t crc_in, const uint8_t byte) {
  uint32_t crc = crc_in;
  uint32_t in = byte | 0x100;

  do {
    crc <<= 1;
    in <<= 1;
    if (in & 0x100)
      ++crc;
    if (crc & 0x10000)
      crc ^= 0x1021;
  }

  while (!(in & 0x10000));

  return crc & 0xffff;
}
