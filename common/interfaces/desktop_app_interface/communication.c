/**
 * @file    communication.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
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
/*********************************************
 * 
 *	Author: Atul
 * 
 * *******************************************/

#include "communication.h"
#include "board.h"
#include "sys_state.h"
#include "logger.h"
#include "utils.h"
#if USE_SIMULATOR == 0
#include "controller_main.h"

#else
#include <unistd.h>
#endif
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
//#include "wallet_sign_unsign.h"



#define U16_SWAP_ENDIANNESS(x) ((x) >> 8 | (x) << 8)
#define U32_SWAP_ENDIANNESS(x) ((x) << 24 | ((x) & 0xff00) << 8 | ((x) & 0xff0000) >> 8 | (x) >> 24)

typedef struct comm_sdk_version {
    uint16_t major_version;
    uint16_t minor_version;
    uint16_t patch_level;
} comm_sdk_version;

static const comm_sdk_version USB_COMM_V1 = { .major_version = 0x0001, .minor_version = 0x0000, .patch_level = 0x0000 };

static uint8_t usb_conn_status = 0;
static uint8_t msg_receive_status = 0;
static uint8_t usb_event_type = 0;
static uint16_t rx_packet_number = 0;
static uint16_t tx_packet_number = 0;
static uint16_t ack_received = 0;
static uint8_t data_trans_app = 0;
static uint8_t device_ready_state = 0;
static uint8_t current_packet_num = 0;

static msg_detail_t msg_send;
static msg_detail_t msg_rec;
static bool use_v0_header = false;

/**
 * @brief Sends a response to desktop.
 * @details
 *
 * @param command_type Response Command Type
 * @param packet_no Packet Number.
 * @param total_size Total number of packets.
 * @param data Response Data
 * @param length Length of data.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void protocol_sendResponsePacket(const uint32_t command_type, const uint16_t packet_no, const uint16_t total_pkt, const uint8_t *data, const uint8_t length);

/**
 * @brief calculate checksum of given data stream
 * @details
 *
 * @param p_data pointer to buffer holding data
 * @param size length of data buffer
 *
 * @return calculated check sum to be returned to caller
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint16_t Cal_CRC16(const uint8_t *p_data, uint16_t size);

/**
 * @brief unstuffs the byte 0xA3,0x3A and 0xA3,0x33 with appropriate value 0xAA or 0xA3.
 * @details
 *
 * @param input_data input buffer to be byte unstuffed
 * @param output_data final byte unstuffed data
 * @param size size of input data with byte stuffing
 * @param out_size size of output data after byte unstuffing
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static bool do_byte_unstuffing(const uint8_t *input_data, uint8_t *output_data, int size, int *out_size);

/**
 * @brief stuffs the byte 0xAA(start of frame).Replace each 0xAA with 0xaA3,0x3A and replace each 0xA3 with 0xA3,0x33
 * @details
 *
 * @param input_data - input buffer to be byte stuffed
 * @param output_data - final byte stuffed data
 * @param size -size of input data without byte stuffing
 * @param out_size - size of output data after byte stuffing
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static bool do_byte_stuffing(const uint8_t *input_data, uint8_t *output_data, int size, int *out_size);

/**
 * @brief Processes the received frame. This function decodes the packet data so that further
 * processing/usage is agnostic to endianness of the architecture.
 * @details
 *
 * @param packet pointer to buffer containing frame.
 * @param size
 * @param header
 * @param comm_data
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static command_status_t process_rx_frame(const uint8_t *packet, const uint8_t size, comm_header_t *header, comm_data_t **comm_data);

/**
 * @brief Start the software timer.
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void software_timer_start();

static uint8_t packet_send_status = 1;
static uint8_t send_previous_packet = 0;

/**
 * @brief Timeout handler for the repeated timer.
 * This is called every time timer ends.
 * @details
 * 
 * @param p_context Pointer passed by the callback caller.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void communication_timer_handler(){//(void *p_context) {
  static uint32_t retry_counter = 0;
  if (ack_received == 0) {
    send_previous_packet = 1;
    retry_counter++;
  } else {
    retry_counter = 0;
  }
}


void software_timer_create(void) {
  BSP_App_Timer_Create(BSP_COM_TIMER, communication_timer_handler);
  software_timer_start();
}

void software_timer_start() {
  BSP_App_Timer_Start(BSP_COM_TIMER, 500);
}

uint8_t is_all_packet_send(void) {
  return packet_send_status;
}

void mark_device_state(const uint8_t ready_state) {
  device_ready_state = ready_state;
}

bool is_device_ready() {
  return device_ready_state;
}

void usb_connection_status_change(const uint8_t status) {
  usb_conn_status = status;
}

uint8_t usb_connection_status(void) {
  return usb_conn_status;
}

uint8_t is_there_any_msg_from_app(void) {
  return msg_receive_status;
}

void clear_message_received_data() {
  memset(&msg_rec, 0, sizeof(msg_detail_t));
  msg_receive_status = 0;
  sys_flow_cntrl_u.bits.usb_buffer_free = true;
}

bool get_usb_msg(En_command_type_t *command_type, uint8_t **msg_data, uint16_t *msg_len) {
  if ((msg_len == NULL && msg_data != NULL) || (msg_len != NULL && msg_data == NULL)) return false;
  if (is_there_any_msg_from_app()) {
    if (command_type) *command_type = msg_rec.msg_type;
    if (msg_len) *msg_len = msg_rec.msg_size;
    if (msg_data) *msg_data = msg_rec.data_array;
    return true;
  }
  return false;
}

bool get_usb_msg_by_cmd_type(En_command_type_t command_type, uint8_t **msg_data, uint16_t *msg_len) {
  if ((msg_len == NULL && msg_data != NULL) || (msg_len != NULL && msg_data == NULL)) return false;
  if (is_there_any_msg_from_app() && msg_rec.msg_type == command_type) {
    if (msg_len) *msg_len = msg_rec.msg_size;
    if (msg_data) *msg_data = msg_rec.data_array;
    return true;
  }
  return false;
}

/**
 * @brief Set command from screen.
 * Sets usb_event_type.
 * 
 * @param cmd Command froms screen.
 */
void usb_cmd_from_screen(const uint8_t cmd) {
  usb_event_type = cmd;
}
#define MAXIMUM_DATA_SIZE_SEND 32
#define MAX_RETRY_NUM 6

static uint8_t send_retry_packet_counter = 0;
static uint8_t send_packet = 0;

bool usb_send_task() {
  static uint32_t current_index = 0;
  static uint8_t trans_len = 0;
  static uint8_t total_pkt = 0;

  if (data_trans_app) {
    if (current_packet_num == 0) {
      total_pkt = (msg_send.msg_size / MAXIMUM_DATA_SIZE_SEND) + 1;

      current_packet_num = 1;
      ack_received = 0;
      send_packet = 1;
      current_index = 0;
    }
    if (ack_received == 1) {
      current_packet_num++;
      current_index = current_index + trans_len;
      send_packet = 1;
      send_retry_packet_counter = 0;
    } else if (send_previous_packet) {
      send_retry_packet_counter++;
      if (send_retry_packet_counter <= MAX_RETRY_NUM) {
        send_packet = 1;
      } else {

        ack_received = 0;
        data_trans_app = 0;
        current_index = 0;
        current_packet_num = 0;
        total_pkt = 0;
        trans_len = 0;
        packet_send_status = 1;
        send_retry_packet_counter = 0;
      }
    }
    if ((current_packet_num <= total_pkt) && send_packet) {
      ack_received = 0;
      send_packet = 0;
      if (total_pkt == current_packet_num)
        trans_len = msg_send.msg_size % MAXIMUM_DATA_SIZE_SEND;
      else
        trans_len = MAXIMUM_DATA_SIZE_SEND;
      protocol_sendResponsePacket(msg_send.msg_type, current_packet_num,
          total_pkt, &msg_send.data_array[current_index], trans_len);

      send_previous_packet = 0;
      ack_received = 0;
    }

    if (current_packet_num == total_pkt + 1) {
      ack_received = 0;
      data_trans_app = 0;
      current_index = 0;
      current_packet_num = 0;
      total_pkt = 0;
      trans_len = 0;
      packet_send_status = 1;
      send_retry_packet_counter = 0;

      return true;
    }
  }

  return false;
}

void transmit_one_byte_reject(const uint32_t command_type) {
  uint8_t arr[1] = {0x0};
  transmit_data_to_app(command_type, arr, 1);
}

void transmit_one_byte_confirm(const uint32_t command_type) {
  uint8_t arr[1] = {0x01};
  transmit_data_to_app(command_type, arr, 1);
}

void transmit_one_byte(const uint32_t command_type, const uint8_t byte) {
  uint8_t arr[1] = {byte};
  transmit_data_to_app(command_type, arr, 1);
}

void transmit_data_to_app(const uint32_t command_type, const uint8_t *transmit_data, const uint32_t size) {
  msg_send.msg_type = command_type;
  msg_send.msg_size = size;
  packet_send_status = 0;
  memcpy(msg_send.data_array, transmit_data, size);
  data_trans_app = 1;
  current_packet_num = 0;
}

uint8_t is_data_sent() {
  return data_trans_app == 0;
}

/// Acknowledgement packet
packet_t sendAck;

/**
 * @brief Serialises payload of a packet (Current packet number || Total packet number
 * || Payload chunk). Performs a CRC16 on the serialised payload and appends the
 * calculated CRC to the already serialised data.
 * @details
 * 
 * @param [out] payload           Output storage for the serialised payload with CRC16
 * @param [in] current_packet_no  Takes current packet number
 * @param [in] total_packet_no    Takes total packet count
 * @param [in] data               Actual data chunk to include in this packet
 * @param [in] data_size          Size of input data chunk
 *
 * @return uint8_t                Size of serialised byte-array
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint8_t serialisePayload(uint8_t *payload, const uint32_t current_packet_no,
                                const uint32_t total_packet_no, const uint8_t *data,
                                const uint8_t data_size) {
  uint8_t offset = 0;
  uint16_t crc;
  payload[offset++] = (current_packet_no >> 8) & 0xFF;
  payload[offset++] = current_packet_no & 0xFF;
  payload[offset++] = (total_packet_no >> 8) & 0xFF;
  payload[offset++] = total_packet_no & 0xFF;
  memcpy(payload + offset, data, data_size);

  offset += data_size;
  crc = Cal_CRC16(payload, offset);
  payload[offset++] = (crc >> 8) & 0xFF; payload[offset++] = crc & 0xFF;
  return offset;
}

/**
 * @brief Serialise the USB packet from provided data (use network byte order
 * for multi-byte data). After serialising, the data is sent to HAL for transfer.
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void USB_writePacket(const uint32_t command_type, const uint8_t *payload,
                               const uint8_t payload_size) {
  if (payload_size > BYTE_STUFFED_DATA_SIZE) {
    LOG_ERROR("xxx22");
    return;
  }
  uint8_t packet[BYTE_STUFFED_DATA_SIZE + COMM_HEADER_SIZE];
  uint8_t offset = 0;
  if (use_v0_header) {
    packet[offset++] = START_OF_FRAME;
    packet[offset++] = command_type;
    packet[offset++] = payload_size;
  } else {
    packet[offset++] = (START_OF_FRAME_U16 >> 8) & 0xFF;
    packet[offset++] = START_OF_FRAME_U16 & 0xFF;
    packet[offset++] = (command_type >> 24) & 0xFF;
    packet[offset++] = (command_type >> 16) & 0xFF;
    packet[offset++] = (command_type >> 8) & 0xFF;
    packet[offset++] = command_type & 0xFF;
    packet[offset++] = payload_size;
  }

  memcpy(packet + offset, payload, payload_size);
  BSP_DebugPort_Write(packet, offset + payload_size);
}

/**
 * @brief Used to send acknowledgement for received data packets
 * @details
 *
 * @param [in] packet_no packet number for which acknowledgement is being sent
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void protocol_sendAckPacket(const uint16_t packet_no) {
  uint8_t payload[BYTE_STUFFED_DATA_SIZE] = {0}, offset = 0;

  memset(sendAck.com_data.data, 0, 4);
  offset = serialisePayload(payload, packet_no, packet_no, sendAck.com_data.data, 4);
  USB_writePacket(ACK_PACKET, payload, offset);
}

/**
 * @brief Send error packet to desktop.
 * @details
 * 
 * @param packet_no Packet number.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void protocol_sendErrorPacket(const uint16_t packet_no) {
  uint8_t payload[BYTE_STUFFED_DATA_SIZE] = {0}, offset = 0;
  
  memset(sendAck.com_data.data, 0, 4);
  offset = serialisePayload(payload, packet_no, packet_no, sendAck.com_data.data, 4);
  USB_writePacket(ERROR_PACKET, payload, offset);
}

static void protocol_sendResponsePacket(const uint32_t command_type, const uint16_t packet_no,
                                        const uint16_t total_pkt, const uint8_t *data, const uint8_t length) {
  uint8_t byte_stuffed_data[BYTE_STUFFED_DATA_SIZE] = {0};
  uint8_t payload[BYTE_STUFFED_DATA_SIZE] = {0}, offset = 0;
  int out_size = 0;

  tx_packet_number = packet_no;
  offset = serialisePayload(payload, packet_no, total_pkt, data, length);
  if (false == do_byte_stuffing(payload, byte_stuffed_data, offset, &out_size)) {
    sendAck.com_header.data_size = 0;
    return;
  }
  USB_writePacket(command_type, byte_stuffed_data, out_size);
  software_timer_start();
}

void receive_packet_parser(const uint8_t *recData, const uint8_t size) {
  if (!CY_Usb_Buffer_Free()) {
      //TODO:Send BUSY packet instead of ERROR, so desktop will resend packet after a specific delay
      protocol_sendErrorPacket(rx_packet_number);
      return;
  }
  comm_header_t header;
  command_status_t status = CMD_RECEIVED;
  comm_data_t *comm_data;
  status = process_rx_frame(recData, size, &header, &comm_data);

  if (status == CMD_RECEIVED && header.command_type != ACK_PACKET) {
    protocol_sendAckPacket(rx_packet_number);
  } else if (status != CMD_RECEIVED) {
    protocol_sendErrorPacket(rx_packet_number);
    return;
  } else if (header.command_type == ERROR_PACKET) {
      return;
  }
#if USE_SIMULATOR == 1
  usleep(200 * 1000);
#endif
  switch (header.command_type) {

  case ACK_PACKET:
    if (rx_packet_number == tx_packet_number) {
      ack_received = 1;
    }
    break;

  case USB_CONNECTION_STATE_PACKET:
    usb_conn_status = 1;
    break;

  case READY_STATE_PACKET: {
    uint8_t arr[4] = {0};
    if (device_ready_state) {
      arr[0] = STATUS_CMD_READY;
      transmit_data_to_app(STATUS_PACKET, arr, sizeof(arr));
    } else {
      arr[0] = STATUS_CMD_NOT_READY;
      transmit_data_to_app(STATUS_PACKET, arr, sizeof(arr));
    }
  } break;

  case DEVICE_FLOW_RESET_REQ:
    CY_Reset_Flow();
    transmit_one_byte_confirm(1);
    break;
  case COMM_SDK_VERSION_REQ: {
    uint8_t version[6];
    version[0] = USB_COMM_V1.major_version >> 8; version[1] = USB_COMM_V1.major_version & 0xFF;
    version[2] = USB_COMM_V1.minor_version >> 8; version[3] = USB_COMM_V1.minor_version & 0xFF;
    version[4] = USB_COMM_V1.patch_level >> 8; version[5] = USB_COMM_V1.patch_level & 0xFF;
    transmit_data_to_app(COMM_SDK_VERSION_REQ, version, sizeof(version));
  } break;
  case DEVICE_INFO:
  case START_DEVICE_AUTHENTICATION:
  case ADD_COIN_START:
  case START_EXPORT_WALLET:
  case SEND_TXN_START:
  case RECV_TXN_START:
  case SEND_TXN_REQ_UNSIGNED_TXN:
  case SEND_TXN_UNSIGNED_TXN:
  case SEND_WALLET_TO_DESKTOP:
  case RECEIVE_TRANSACTION_PACKET:
  case START_AUTH_PROCESS:
  case APP_SEND_RAND_NUM:
  case SEND_SIGNATURE_TO_APP:
  case SIGNED_CHALLENGE:
#ifdef ALLOW_LOG_EXPORT
  case APP_LOG_DATA_REQUEST:
  case APP_LOG_DATA_SEND:
#endif
  case STATUS_PACKET:
  case START_CARD_AUTH:
#ifdef DEBUG_BUILD
  case START_CARD_UPGRADE:
  case APDU_PACKET:
  case EXPORT_ALL:
  case STOP_CARD_UPGRADE:
#endif
  case START_FIRMWARE_UPGRADE:
#if X1WALLET_INITIAL
  case START_DEVICE_PROVISION:
#endif
  case TRANSACTION_PACKET: {
    static uint16_t total_received_packet = 0;
    static uint16_t current_index = 0;
    if ((comm_data->current_packet_no < comm_data->total_Packet) && total_received_packet + 1 == comm_data->current_packet_no) {
      total_received_packet++;
      memcpy(&msg_rec.data_array[current_index], comm_data->data , header.data_size - (PKT_HEAD_SIZE + CRC16_SIZE));
      current_index = current_index + (header.data_size - (PKT_HEAD_SIZE + CRC16_SIZE));
    } else if (comm_data->current_packet_no == comm_data->total_Packet) {
      total_received_packet++;
      memcpy(&msg_rec.data_array[current_index], comm_data->data, header.data_size - (PKT_HEAD_SIZE + CRC16_SIZE));
      msg_rec.msg_size = current_index + header.data_size - (PKT_HEAD_SIZE + CRC16_SIZE);
      msg_rec.msg_type = header.command_type;

      if(CY_reset_not_allowed() && msg_rec.msg_type == STATUS_PACKET && msg_rec.data_array[0] == STATUS_CMD_ABORT) {
        clear_message_received_data();
      } else {
        msg_receive_status = 1;
        sys_flow_cntrl_u.bits.usb_buffer_free = false;
      }

      current_index = 0;
      total_received_packet = 0;
    }
  } break;
  default:
    protocol_sendErrorPacket(rx_packet_number);
    break;
  }
}

static command_status_t process_rx_frame(const uint8_t *packet, const uint8_t size, comm_header_t *header, comm_data_t **comm_data) {
  uint16_t crc = 0;
  uint16_t received_crc = 0;
  uint8_t unstuffed_data[MAXIMUM_DATA_SIZE] = {0};
  uint8_t *payload;
  int out_size = 0;

  if (packet[0] == START_OF_FRAME) {
    use_v0_header = true;
    header->start_frame = START_OF_FRAME;
    header->command_type = packet[1];
    header->data_size = packet[2];
    payload = (uint8_t *) (packet + COMM_HEADER_SIZE_V0);
  } else if (U16_READ_BE_ARRAY(packet) == START_OF_FRAME_U16) {
    use_v0_header = false;
    header->start_frame = START_OF_FRAME_U16;
    header->command_type = U32_READ_BE_ARRAY(packet + 2);
    header->data_size = packet[6];
    payload = (uint8_t *) (packet + COMM_HEADER_SIZE);
  } else {
    LOG_ERROR("xxx20");
    return CMD_SIZE_ERROR;
  }

  if (header->data_size > 0) {
    if (false == do_byte_unstuffing(payload, unstuffed_data, header->data_size, &out_size)) {
      return CMD_SIZE_ERROR;
    }
  } else {
    return CMD_SIZE_ERROR;
  }

  if (out_size <= 2 || out_size > MAXIMUM_DATA_SIZE) {
      return CRC_ERROR;
  }

  rx_packet_number = (unstuffed_data[1]) | (unstuffed_data[0] << 8);
  received_crc = (unstuffed_data[out_size - 1]) | (unstuffed_data[out_size - 2] << 8);
  crc = Cal_CRC16(unstuffed_data, out_size - 2);

  if (crc != received_crc) {
    return CRC_ERROR;
  }
  header->data_size = out_size;
  memcpy(payload, unstuffed_data, out_size);      // Replace the original payload with unstuffed payload
  *comm_data = (comm_data_t *) payload;           // Point to unstuffed payload (pointer saves a little memory), later decode multi-byte values
  (*comm_data)->current_packet_no = U16_READ_BE_ARRAY(payload);
  (*comm_data)->total_Packet = U16_READ_BE_ARRAY(payload + 2);

  return CMD_RECEIVED;
}

/**
  * @brief  Update CRC16 for input byte
  * @details
  *
  * @param  crc_in input value
  * @param  input byte
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint16_t UpdateCRC16(const uint16_t crc_in, const uint8_t byte) {
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

static uint16_t Cal_CRC16(const uint8_t *p_data, const uint16_t size) {
  uint32_t crc = 0;
  const uint8_t *dataEnd = p_data + size;

  while (p_data < dataEnd)
    crc = UpdateCRC16(crc, *p_data++);

  crc = UpdateCRC16(crc, 0);
  crc = UpdateCRC16(crc, 0);
  return crc & 0xffff;
}

static bool do_byte_stuffing(const uint8_t *input_data, uint8_t *output_data, const int size, int *out_size) {
  int in_index = 0;
  int out_index = 0;
  uint8_t sof_byte = use_v0_header ? START_OF_FRAME : (0xFF & START_OF_FRAME_U16);

  if (input_data == NULL || output_data == NULL || out_size == NULL || size > MAXIMUM_DATA_SIZE) {
    return false;
  }

  for (in_index = 0; in_index < size; in_index++, out_index++) {
    if (out_index > BYTE_STUFFED_DATA_SIZE) {
      return false;
    }
    if (input_data[in_index] == sof_byte) {
      output_data[out_index++] = 0xA3;
      output_data[out_index] = 0x3A;
    } else if (input_data[in_index] == 0xA3) {
      output_data[out_index++] = 0xA3;
      output_data[out_index] = 0x33;
    } else {
      output_data[out_index] = input_data[in_index];
    }
  }
  *out_size = size + (out_index - in_index);
  return true;
}

static bool do_byte_unstuffing(const uint8_t *input_data, uint8_t *output_data, const int size, int *out_size) {
  int out_index = 0;

  if (input_data == NULL || output_data == NULL || out_size == NULL || size > BYTE_STUFFED_DATA_SIZE) {
    return false;
  }

  for (int in_index = 0; in_index < size; in_index++, out_index++) {
    if (input_data[in_index] == 0xA3 && in_index < size - 1) {
      if (input_data[in_index + 1] == 0x3A) {
        output_data[out_index] = (use_v0_header ? START_OF_FRAME : 0xFF & START_OF_FRAME_U16);
        in_index++;
      } else if (input_data[in_index + 1] == 0x33) {
        output_data[out_index] = 0xA3;
        in_index++;
      } else {
        output_data[out_index] = input_data[in_index];
      }
    } else {
      output_data[out_index] = input_data[in_index];
    }
  }
  *out_size = out_index;
  return true;
}
