/**
 * @file    usb_rx_parser.c
 * @author  Cypherock X1 Team
 * @brief   Handles parsing of received data stream into packets.
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
#include <stdint.h>
#include <string.h>

#if USE_SIMULATOR == 0
#include "libusb.h"
#endif
#include "usb_api_priv.h"
#include "utils.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

#define COMM_V0_START_OF_HEADER 0xAA

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

typedef enum comm_parser_states {
  WAIT4_SOH1,            ///<
  WAIT4_SOH2,            ///<
  WAIT4_CHECKSUM1,       ///< High order byte of CRC-16
  WAIT4_CHECKSUM2,       ///< Low order byte of CRC-16
  WAIT4_CHUNK_NO1,       ///< Highest order byte of current chunk number
  WAIT4_CHUNK_NO2,       ///< Lower order byte of current chunk number
  WAIT4_CHUNK_COUNT1,    ///< High order byte of total chunks
  WAIT4_CHUNK_COUNT2,    ///< Lower order byte of total chunks
  WAIT4_SEQ_NO1,         ///< High order byte of sequence number
  WAIT4_SEQ_NO2,         ///< Lower order byte of sequence number
  WAIT4_PKT_TYPE,        ///< Type of current packet @see comm_packet_type
  WAIT4_TIMESTAMP1,      ///< Highest order byte of timestamp
  WAIT4_TIMESTAMP2,      ///< 3rd highest order byte of timestamp
  WAIT4_TIMESTAMP3,      ///< 2nd highest order byte of timestamp
  WAIT4_TIMESTAMP4,      ///< Lowest order byte of timestamp
  WAIT4_PKT_LEN,         ///< Length of current packet
  WAIT4_PAYLOAD,         ///< Payload of current packet
  WAIT4_PKT_PROCESS,     ///< Wait for application to process the packet
} comm_parser_states;

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/* data length (1 byte), current packet (2 bytes), packet count (2
 * bytes), cmd payload (1 byte), checksum (2 bytes) */
static uint8_t SDK_REQ_PACKET[] = {COMM_V0_START_OF_HEADER,
                                   COMM_SDK_VERSION_REQ,
                                   0x07,
                                   0x00,
                                   0x01,
                                   0x00,
                                   0x01,
                                   0x00,
                                   0x45,
                                   0x85};
static uint8_t SDK_RESP_PACKET[] = {
    COMM_V0_START_OF_HEADER,
    0x01,    // ack packet
    0x0A,
    0x00,
    0x01,
    0x00,
    0x01,    // data length (1 byte), current packet (2 bytes), packet count (2
             // bytes)
    0x00,
    0x00,
    0x00,
    0x00,    // cmd payload (4 byte)
    0x12,
    0x30,    // Checksum
    COMM_V0_START_OF_HEADER,
    COMM_SDK_VERSION_REQ,    // SDK response packet
    0x0C,
    0x00,
    0x01,
    0x00,
    0x01,    // data length (1 byte), current packet (2 bytes), packet count (2
             // bytes)
    0x00,
    0x03,
    0x00,
    0x00,
    0x00,
    0x00,    // SDK version 3.0.0 for current working protocol
    0xBD,
    0x90    // Checksum
};

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

void comm_packet_parser(const uint8_t *data,
                        const uint16_t length,
                        comm_libusb__interface_e interface) {
  static comm_parser_states state = WAIT4_SOH1;
  static packet_t rx_packet = {0};
  static uint8_t payload_size = 0;
  static uint32_t packet_crc = 0;

  if (memcmp(data, SDK_REQ_PACKET, CY_MIN(sizeof(SDK_REQ_PACKET), length)) == 0)
#if USE_SIMULATOR == 1
    return SIM_Transmit_FS(SDK_RESP_PACKET, sizeof(SDK_RESP_PACKET));
#else
    return lusb_write(SDK_RESP_PACKET, sizeof(SDK_RESP_PACKET), interface);
#endif

  rx_packet.interface = interface;

  for (int i = 0; i < length; i++) {
    uint8_t byte = data[i];
    switch (state) {
      case WAIT4_SOH1:
        payload_size = 0;
        rx_packet.header.start_of_header = byte;
        if (byte == COMM_START_OF_HEADER)
          state = WAIT4_SOH2;
        break;
      case WAIT4_SOH2:
        rx_packet.header.start_of_header =
            (rx_packet.header.start_of_header << 8) | byte;
        if (byte != COMM_START_OF_HEADER)
          state = WAIT4_SOH1;
        else
          state = WAIT4_CHECKSUM1;
        break;
      case WAIT4_CHECKSUM1:
        rx_packet.header.checksum = byte;
        state = WAIT4_CHECKSUM2;
        break;
      case WAIT4_CHECKSUM2:
        rx_packet.header.checksum = (rx_packet.header.checksum << 8) | byte;
        state = WAIT4_CHUNK_NO1;
        break;
      case WAIT4_CHUNK_NO1:
        packet_crc = 0;
        rx_packet.header.chunk_number = byte;
        state = WAIT4_CHUNK_NO2;
        break;
      case WAIT4_CHUNK_NO2:
        rx_packet.header.chunk_number =
            (rx_packet.header.chunk_number << 8) | byte;
        state = WAIT4_CHUNK_COUNT1;
        break;
      case WAIT4_CHUNK_COUNT1:
        rx_packet.header.total_chunks = byte;
        state = WAIT4_CHUNK_COUNT2;
        break;
      case WAIT4_CHUNK_COUNT2:
        rx_packet.header.total_chunks =
            (rx_packet.header.total_chunks << 8) | byte;
        state = WAIT4_SEQ_NO1;
        break;
      case WAIT4_SEQ_NO1:
        rx_packet.header.sequence_no = byte;
        state = WAIT4_SEQ_NO2;
        break;
      case WAIT4_SEQ_NO2:
        rx_packet.header.sequence_no =
            (rx_packet.header.sequence_no << 8) | byte;
        state = WAIT4_PKT_TYPE;
        break;
      case WAIT4_PKT_TYPE:
        rx_packet.header.packet_type = byte;
        state = WAIT4_TIMESTAMP1;
        break;
      case WAIT4_TIMESTAMP1:
        rx_packet.header.timestamp = byte;
        state = WAIT4_TIMESTAMP2;
        break;
      case WAIT4_TIMESTAMP2:
        rx_packet.header.timestamp = (rx_packet.header.timestamp << 8) | byte;
        state = WAIT4_TIMESTAMP3;
        break;
      case WAIT4_TIMESTAMP3:
        rx_packet.header.timestamp = (rx_packet.header.timestamp << 8) | byte;
        state = WAIT4_TIMESTAMP4;
        break;
      case WAIT4_TIMESTAMP4:
        rx_packet.header.timestamp = (rx_packet.header.timestamp << 8) | byte;
        state = WAIT4_PKT_LEN;
        break;
      case WAIT4_PKT_LEN:
        payload_size = byte;
        rx_packet.header.payload_length = byte;
        rx_packet.payload = payload_size ? &data[i + 1] : NULL;
        state = payload_size ? WAIT4_PAYLOAD : WAIT4_PKT_PROCESS;
        if (byte > COMM_MAX_PAYLOAD_SIZE) {
          state = WAIT4_SOH1;
        }
        break;
      case WAIT4_PAYLOAD:
        if (--payload_size == 0)
          state = WAIT4_PKT_PROCESS;
        break;
      default:
        state = WAIT4_SOH1;
        break;
    }
    packet_crc = update_crc16(packet_crc, byte);
    if (state == WAIT4_PKT_PROCESS) {
      packet_crc = update_crc16(packet_crc, 0);
      packet_crc = update_crc16(packet_crc, 0);
      packet_crc &= 0xFFFF;
      if (packet_crc == rx_packet.header.checksum) {
        comm_process_packet(&rx_packet);
        memzero(&rx_packet, sizeof(rx_packet));
      } else {
        send_error_packet(&rx_packet, CHECKSUM_ERROR);
        memzero(&rx_packet, sizeof(rx_packet));
      }
      state = WAIT4_SOH1;
    }
  }
  if (state != WAIT4_SOH1) {
    state = WAIT4_SOH1;
    send_error_packet(&rx_packet, INCOMPLETE_PACKET);
    memzero(&rx_packet, sizeof(rx_packet));
  }
}
