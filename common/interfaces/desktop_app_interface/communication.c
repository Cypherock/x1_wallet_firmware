/**
 * @file    communication.c
 * @author  Cypherock X1 Team
 * @brief   USB communication interface.
 *          Handles all USB communication operations for the application from the ISR context.
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
#include "communication.h"
#include "board.h"
#include "sys_state.h"
#include "logger.h"
#include "utils.h"
#if USE_SIMULATOR == 0
#include "controller_main.h"
#include "libusb.h"
#else
#include <unistd.h>
#endif
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define COMM_BUFFER_SIZE        (6 * 1024)
#define COMM_PKT_MAX_LEN        64
#define COMM_MAX_PAYLOAD_SIZE   (COMM_PKT_MAX_LEN - COMM_HEADER_SIZE)

#define COMM_V0_START_OF_HEADER 0xAA
#define COMM_START_OF_HEADER    0x55
#define COMM_HEADER_INDEX       0
#define COMM_CHECKSUM_INDEX     2
#define COMM_CHUNK_NO_INDEX     4
#define COMM_CHUNK_COUNT_INDEX  6
#define COMM_SEQ_NO_INDEX       8
#define COMM_PKT_TYPE_INDEX     10
#define COMM_TIMESTAMP_INDEX    11
#define COMM_PAYLOAD_LEN_INDEX  15
#define COMM_PAYLOAD_INDEX      16

#define comm_get_raw_payload_size(raw_payload) (raw_payload ? (sizeof(uint32_t ) + raw_payload->msg_size) : 0)
#define comm_get_proto_payload_size(proto_payload) (proto_payload ? (proto_payload)->size : 0)

/// @details Macro to get the size of the total payload in Cmd Output response.
#define comm_get_payload_size(payload) (2 * sizeof(uint16_t) + payload.raw_data_length + payload.proto_data_length)

typedef struct comm_sdk_version {
    uint16_t major_version;
    uint16_t minor_version;
    uint16_t patch_level;
} comm_sdk_version;

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

/**
 * @brief Communication Header struct
 * @details
 *
 * @see COMM_HEADER_SIZE
 * @since v1.0.0
 *
 * @note
 */
typedef struct comm_header  {
    uint16_t start_of_header;       ///< 2-byte data representing start of packet
    uint16_t checksum;              ///< Checksum on (chunk_number + total_chunks + sequence_no + comm_packet_type + payload_length + timestamp + payload)
                                    ///< This uses <a href="https://mdfs.net/Info/Comp/Comms/CRC16.htm">CRC-16/XMODEM algorithm</a>
    uint16_t chunk_number;          ///< 2-byte data representing the current chunk number. For the first chunk, this is 1.
    uint16_t total_chunks;          ///< 2-byte data representing the total number of chunks. For single packet data, this is 1.
    uint16_t sequence_no;           ///< Usb host app request sequence number. -1 for packet types PKT_TYPE_STATUS_REQ, PKT_TYPE_STATUS_ACK & PKT_TYPE_ERROR.
    uint8_t  packet_type;           ///< Defines the packet type. @see comm_packet_type
    uint8_t  payload_length;        ///< Length of the payload in the current packet
    uint32_t timestamp;             ///< Packet preparation timestamp of host machine in milliseconds. Each entity (host & peripheral) will reset to 0 across resets.
} comm_header_t;

/**
 * @brief Single package struct
 * @details
 *
 * @see comm_header_t
 * @since v1.0.0
 *
 * @note
 */
typedef struct packet {
    comm_header_t header;
    const uint8_t *payload;
} packet_t;

/**
 * @brief struct for raw serialized data received over usb
 */
typedef struct comm_raw_payload {
    uint32_t msg_type;
    uint8_t *msg_data;
} comm_raw_payload_t;

/**
 * @brief struct for message received from desktop
 * @details This is the application buffer for the message received from the desktop application. The entire payload is
 * aggregated into this buffer before handing it over to the application. Similarly, the response from the application is
 * stored into this buffer before sending it to the desktop application. The aggregation and fragmentation is handled
 * by the communication layer. For members of the raw serialized message, refer comm_raw_message_t.
 *
 * @see packet_t, comm_raw_payload_t
 * @since v1.0.0
 *
 * @note
 */
typedef struct comm_payload {
    uint16_t proto_data_length;     ///< Length of the protobuf serialized data in the cmd payload
    uint16_t raw_data_length;       ///< Length of the raw serialization data in the cmd payload
    uint8_t *proto_data;            ///< Protobuf serialization data in the cmd payload
    uint8_t *raw_data;              ///< Raw serialization data in the cmd payload
} comm_payload_t;



typedef struct comm_status {
    // Application info
    uint8_t app_busy_status;
    uint8_t abort_disabled;
    uint8_t curr_flow_status;

    // Peripheral info
    uint16_t curr_cmd_seq_no;
    uint8_t curr_cmd_state;

    // Details of the command receive in progress (not to be sent to host)
    uint16_t curr_cmd_chunk_no;
    uint16_t curr_cmd_received_length;

    // Host sync status (not to be sent to host)
    uint32_t host_sync_time;
    uint8_t host_sync_fails;
} comm_status_t;

typedef enum comm_error_code {
    NO_ERROR = 0,
    CHECKSUM_ERROR = 1,
    BUSY_PREVIOUS_CMD = 2,
    OUT_OF_ORDER_CHUNK = 3,
    INVALID_CHUNK_COUNT = 4,
    INVALID_SEQUENCE_NO = 5,
    INVALID_PAYLOAD_LENGTH = 6,
    APP_BUFFER_BLOCKED = 7,
    NO_MORE_CHUNKS = 8,
    INVALID_PACKET_TYPE = 9,
    INVALID_CHUNK_NO = 10,
    INCOMPLETE_PACKET = 11,
} comm_error_code_t;

typedef enum comm_cmd_state {
    CMD_STATE_NONE = 0,
    CMD_STATE_RECEIVING = 1,
    CMD_STATE_RECEIVED = 2,
    CMD_STATE_EXECUTING = 3,
    CMD_STATE_DONE = 4,
    CMD_STATE_FAILED = 5,
    CMD_STATE_INVALID_CMD = 6,
} comm_cmd_state_t;

typedef enum comm_parser_states {
    WAIT4_SOH1,             ///<
    WAIT4_SOH2,             ///<
    WAIT4_CHECKSUM1,        ///< High order byte of CRC-16
    WAIT4_CHECKSUM2,        ///< Low order byte of CRC-16
    WAIT4_CHUNK_NO1,        ///< Highest order byte of current chunk number
    WAIT4_CHUNK_NO2,        ///< Lower order byte of current chunk number
    WAIT4_CHUNK_COUNT1,     ///< High order byte of total chunks
    WAIT4_CHUNK_COUNT2,     ///< Lower order byte of total chunks
    WAIT4_SEQ_NO1,          ///< High order byte of sequence number
    WAIT4_SEQ_NO2,          ///< Lower order byte of sequence number
    WAIT4_PKT_TYPE,         ///< Type of current packet @see comm_packet_type
    WAIT4_TIMESTAMP1,       ///< Highest order byte of timestamp
    WAIT4_TIMESTAMP2,       ///< 3rd highest order byte of timestamp
    WAIT4_TIMESTAMP3,       ///< 2nd highest order byte of timestamp
    WAIT4_TIMESTAMP4,       ///< Lowest order byte of timestamp
    WAIT4_PKT_LEN,          ///< Length of current packet
    WAIT4_PAYLOAD,          ///< Payload of current packet
    WAIT4_PKT_PROCESS,      ///< Wait for application to process the packet
} comm_parser_states;

static void comm_process_packet(const packet_t *rx_packet);
static void comm_packet_parser(const uint8_t *data, uint16_t length);
static uint16_t update_crc16(uint16_t crc_in, uint8_t byte);
static comm_error_code_t comm_process_cmd_packet(const packet_t *rx_packet);
static comm_error_code_t comm_process_status_packet(const packet_t *rx_packet);
static comm_error_code_t comm_process_out_req_packet(const packet_t *rx_packet);
static comm_error_code_t comm_process_abort_packet(const packet_t *rx_packet);
static void send_error_packet(const packet_t *rx_packet, comm_error_code_t error_code);
static void send_status_packet();
static void send_cmd_ack_packet(const packet_t *rx_packet);
static void send_cmd_output_packet(const packet_t *rx_packet);
static void comm_write_packet(uint16_t chunk_number, uint16_t total_chunks, uint16_t seq_no,
                              uint8_t packet_type, uint8_t payload_size, const uint8_t *payload);

static uint8_t comm_io_buffer[COMM_BUFFER_SIZE] = {0};
static comm_payload_t comm_payload;     // received/ready-to-send cmd can be decoded/encoded into this buffer
static comm_status_t comm_status;
static uint8_t SDK_REQ_PACKET[] = {
    COMM_V0_START_OF_HEADER, COMM_SDK_VERSION_REQ,
    0x07, 0x00, 0x01, 0x00, 0x01, 0x00,     // data length (1 byte), current packet (2 bytes), packet count (2 bytes), cmd payload (1 byte)
    0x45, 0x85                              // Checksum
};
static uint8_t SDK_RESP_PACKET[] = {
    COMM_V0_START_OF_HEADER, 0x01,          // ack packet
    0x0A, 0x00, 0x01, 0x00, 0x01,           // data length (1 byte), current packet (2 bytes), packet count (2 bytes)
    0x00, 0x00, 0x00, 0x00,                 // cmd payload (4 byte)
    0x12, 0x30,                             // Checksum
    COMM_V0_START_OF_HEADER, COMM_SDK_VERSION_REQ,       // SDK response packet
    0x0C, 0x00, 0x01, 0x00, 0x01,           // data length (1 byte), current packet (2 bytes), packet count (2 bytes)
    0x00, 0x02, 0x00, 0x00, 0x00, 0x00,     // SDK version 2 for current working protocol
    0x17, 0xC1                              // Checksum
};

static inline void comm_reset() {
    LOG_SWV("comm_reset\n");
    comm_status.curr_cmd_received_length = 0;
    comm_status.curr_cmd_state = CMD_STATE_NONE;
    comm_status.curr_cmd_chunk_no = 0;
}

static inline void comm_set_payload_struct(uint16_t proto_len, uint16_t raw_len) {
    comm_payload.proto_data_length = proto_len;
    comm_payload.raw_data_length = raw_len;
    comm_payload.proto_data = proto_len ? comm_io_buffer + sizeof(uint16_t) * 2 : NULL;
    comm_payload.raw_data = raw_len ? comm_io_buffer + 2 * sizeof(uint16_t) + comm_payload.proto_data_length: NULL;
}

void comm_init() {
    lusb_register_parserFunction(comm_packet_parser);
}

void mark_device_state(cy_app_status_t state, uint8_t flow_status) {
    uint8_t usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
    NVIC_DisableIRQ(OTG_FS_IRQn);
    if (state != CY_UNUSED_STATE) comm_status.app_busy_status = state;
    if (flow_status != 0xFF) comm_status.curr_flow_status = flow_status;
    comm_status.abort_disabled = CY_reset_not_allowed();
    if(usb_irq_enable_on_entry == true)
        NVIC_EnableIRQ(OTG_FS_IRQn);
}

bool is_device_ready() {
    return comm_status.app_busy_status & CY_APP_IDLE;
}

void comm_reject_request(En_command_type_t command_type, uint8_t byte) {
    uint8_t arr[1] = {byte}, usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);

    NVIC_DisableIRQ(OTG_FS_IRQn);
    // Make sure to set he curr_cmd_state to CMD_STATE_FAILED
    transmit_data_to_app(command_type, arr, 1);
    comm_status.curr_cmd_state = CMD_STATE_FAILED;               // Imp: Should be updated after writing to buffer
    comm_status.app_busy_status = CY_APP_IDLE | CY_APP_IDLE_TASK;
    if(usb_irq_enable_on_entry == true)
        NVIC_EnableIRQ(OTG_FS_IRQn);
}

void comm_reject_invalid_cmd() {
    uint8_t usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
    NVIC_DisableIRQ(OTG_FS_IRQn);
    comm_status.curr_cmd_state = CMD_STATE_INVALID_CMD;               // Imp: Should be updated after writing to buffer
    comm_status.app_busy_status = CY_APP_IDLE | CY_APP_IDLE_TASK;
    if(usb_irq_enable_on_entry == true)
        NVIC_EnableIRQ(OTG_FS_IRQn);
}

bool is_there_any_msg_from_app() {
    return comm_status.curr_cmd_state == CMD_STATE_RECEIVED;
}

void transmit_one_byte_confirm(const uint32_t command_type) {
    uint8_t arr[1] = {0x01}, usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
    NVIC_DisableIRQ(OTG_FS_IRQn);
    transmit_data_to_app(command_type, arr, 1);
    if(usb_irq_enable_on_entry == true)
        NVIC_EnableIRQ(OTG_FS_IRQn);
}

void transmit_one_byte(const uint32_t command_type, const uint8_t byte) {
    uint8_t arr[1] = {byte}, usb_irq_enable_on_entry = NVIC_GetEnableIRQ(OTG_FS_IRQn);
    NVIC_DisableIRQ(OTG_FS_IRQn);
    transmit_data_to_app(command_type, arr, 1);
    if(usb_irq_enable_on_entry == true)
        NVIC_EnableIRQ(OTG_FS_IRQn);
}

void transmit_data_to_app(const uint32_t command_type, const uint8_t *transmit_data, const uint32_t size) {
    uint16_t proto_len = 0;
    uint8_t offset = 0;
    LOG_SWV("%s: %ld-%ld\n", __func__, command_type, size);

    sys_flow_cntrl_u.bits.usb_buffer_free = true;
    comm_status.curr_cmd_state = CMD_STATE_DONE;
    comm_set_payload_struct(proto_len, size + sizeof(uint32_t));
    comm_io_buffer[offset++] = 0; comm_io_buffer[offset++] = 0;
    comm_io_buffer[offset++] = ((size + sizeof(uint32_t)) >> 8) & 0xFF;
    comm_io_buffer[offset++] = (size + sizeof(uint32_t)) & 0xFF;
    if (comm_payload.proto_data != NULL) {
        // TODO: Handle protobuf encoding here
    }
    offset = 0;
    if (comm_payload.raw_data != NULL) {
        comm_payload.raw_data[offset++] = (command_type >> 24) & 0xFF;
        comm_payload.raw_data[offset++] = (command_type >> 16) & 0xFF;
        comm_payload.raw_data[offset++] = (command_type >> 8) & 0xFF;
        comm_payload.raw_data[offset++] = command_type & 0xFF;
        memcpy(comm_payload.raw_data + comm_payload.proto_data_length + sizeof(uint32_t), transmit_data, size);
    }
}

void clear_message_received_data() {
    sys_flow_cntrl_u.bits.usb_buffer_free = true;

    // If cmd_state has already transitioned, then the cmd is already completed.
    // This will make double calls to clear_message_received_data() safe from protocol.
    if (comm_status.curr_cmd_state != CMD_STATE_RECEIVED) return;
    comm_set_payload_struct(0, 0);
    comm_status.curr_cmd_state = CMD_STATE_EXECUTING;
    LOG_SWV("%s\n", __func__);
}

void comm_process_complete() {
    comm_status.curr_cmd_state = CMD_STATE_NONE;
}

bool get_usb_msg(En_command_type_t *command_type, uint8_t **msg_data, uint16_t *msg_len) {
    if ((msg_len == NULL && msg_data != NULL) || (msg_len != NULL && msg_data == NULL)) return false;
    if (is_there_any_msg_from_app()) {
        // TODO: Handle hybrid (raw and protobuf together) messages here
        if (command_type) *command_type = U32_READ_BE_ARRAY(comm_payload.raw_data);
        if (msg_data) *msg_data = comm_payload.raw_data + sizeof(uint32_t);
        if (msg_len) *msg_len = comm_payload.raw_data_length - sizeof(uint32_t);
        return true;
    }
    return false;
}

bool get_usb_msg_by_cmd_type(En_command_type_t command_type, uint8_t **msg_data, uint16_t *msg_len) {
    if ((msg_len == NULL && msg_data != NULL) || (msg_len != NULL && msg_data == NULL)) return false;
    if (is_there_any_msg_from_app() && U32_READ_BE_ARRAY(comm_payload.raw_data) == command_type) {
        // TODO: Handle hybrid (raw and protobuf together) messages here
        if (msg_data) *msg_data = comm_payload.raw_data + sizeof(uint32_t);
        if (msg_len) *msg_len = comm_payload.raw_data_length - sizeof(uint32_t);
        return true;
    }
    return false;
}

/**
 * @brief Send a packet to the host
 * @details This function aggregates a received packet from the host.
 *
 * @param data
 * @param length
 */
static void comm_packet_parser(const uint8_t *data, const uint16_t length) {
    static comm_parser_states state = WAIT4_SOH1;
    static packet_t rx_packet = {0};
    static uint8_t payload_size = 0;
    static uint32_t packet_crc = 0;

    if (memcmp(data, SDK_REQ_PACKET, CY_MIN(sizeof(SDK_REQ_PACKET), length)) == 0)
        return lusb_write(SDK_RESP_PACKET, sizeof(SDK_RESP_PACKET));

    for (int i = 0; i < length; i++) {
        uint8_t byte = data[i];
        switch (state) {
            case WAIT4_SOH1:
                payload_size = 0;
                rx_packet.header.start_of_header = byte;
                if (byte == COMM_START_OF_HEADER) state = WAIT4_SOH2;
                break;
            case WAIT4_SOH2:
                rx_packet.header.start_of_header = (rx_packet.header.start_of_header << 8) | byte;
                if (byte != COMM_START_OF_HEADER) state = WAIT4_SOH1;
                else state = WAIT4_CHECKSUM1;
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
                rx_packet.header.chunk_number = (rx_packet.header.chunk_number << 8) | byte;
                state = WAIT4_CHUNK_COUNT1;
                break;
            case WAIT4_CHUNK_COUNT1:
                rx_packet.header.total_chunks = byte;
                state = WAIT4_CHUNK_COUNT2;
                break;
            case WAIT4_CHUNK_COUNT2:
                rx_packet.header.total_chunks = (rx_packet.header.total_chunks << 8) | byte;
                state = WAIT4_SEQ_NO1;
                break;
            case WAIT4_SEQ_NO1:
                rx_packet.header.sequence_no = byte;
                state = WAIT4_SEQ_NO2;
                break;
            case WAIT4_SEQ_NO2:
                rx_packet.header.sequence_no = (rx_packet.header.sequence_no << 8) | byte;
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
                } break;
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
            } else {
                send_error_packet(&rx_packet, CHECKSUM_ERROR);
            }
            state = WAIT4_SOH1;
        }
    }
    if (state != WAIT4_SOH1) {
        state = WAIT4_SOH1;
        send_error_packet(&rx_packet, INCOMPLETE_PACKET);
    }
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
static uint16_t update_crc16(const uint16_t crc_in, const uint8_t byte) {
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

/**
 * @brief Processes the received frame.
 * @details
 *
 * @param recData pointer to buffer containing frame
 * @param size total length
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 */
static void comm_process_packet(const packet_t *rx_packet) {
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
    if (proc_error != NO_ERROR) send_error_packet(rx_packet, proc_error);
}

static comm_error_code_t comm_process_status_packet(const packet_t *rx_packet) {
    if (rx_packet->header.chunk_number != 1) return INVALID_CHUNK_NO;
    if (rx_packet->header.total_chunks != 1) return INVALID_CHUNK_COUNT;
    if (rx_packet->header.payload_length != 0) return INVALID_PAYLOAD_LENGTH;
    if (rx_packet->header.sequence_no != 0xFFFF) return INVALID_SEQUENCE_NO;

    send_status_packet();
    return NO_ERROR;
}

/**
 * @details Packet type: PKT_TYPE_CMD <br/>
 *
 */
static comm_error_code_t comm_process_cmd_packet(const packet_t *rx_packet) {
    if (!CY_Usb_Buffer_Free()) return APP_BUFFER_BLOCKED;
    if (comm_status.curr_cmd_state == CMD_STATE_EXECUTING) return BUSY_PREVIOUS_CMD;

    comm_status.curr_cmd_state = CMD_STATE_RECEIVING;
    if (comm_status.curr_cmd_seq_no != rx_packet->header.sequence_no)
        comm_reset();       // Clear current status and start new command

    if (comm_status.curr_cmd_chunk_no + 1 < rx_packet->header.chunk_number) return OUT_OF_ORDER_CHUNK;
    if (rx_packet->header.chunk_number > rx_packet->header.total_chunks) return INVALID_CHUNK_COUNT;

    comm_status.curr_cmd_seq_no = rx_packet->header.sequence_no;
    if (comm_status.curr_cmd_chunk_no + 1 == rx_packet->header.chunk_number) {
        // Duplicate packets are ignored; Only packets in expected sequence are appended to buffer
        comm_status.curr_cmd_chunk_no = rx_packet->header.chunk_number;
        memcpy(comm_io_buffer + comm_status.curr_cmd_received_length, rx_packet->payload,
               rx_packet->header.payload_length);
        comm_status.curr_cmd_received_length += rx_packet->header.payload_length;
        if (rx_packet->header.chunk_number == rx_packet->header.total_chunks) {
            // Last chunk received
            if (comm_status.curr_cmd_received_length != (U16_READ_BE_ARRAY(comm_io_buffer) +
                                                        U16_READ_BE_ARRAY(comm_io_buffer + sizeof(uint16_t)) +
                                                        sizeof(uint16_t) * 2)) {
                LOG_SWV("#RED#Invalid payload length: %d + %d + 4 != %d\n", U16_READ_BE_ARRAY(comm_io_buffer),
                        U16_READ_BE_ARRAY(comm_io_buffer + sizeof(uint16_t)), comm_status.curr_cmd_received_length);
                comm_reset();
                return INVALID_PAYLOAD_LENGTH;
            } else {
                sys_flow_cntrl_u.bits.usb_buffer_free = false;
                comm_set_payload_struct(U16_READ_BE_ARRAY(comm_io_buffer), U16_READ_BE_ARRAY(comm_io_buffer + sizeof(uint16_t)));
            }
        }
    }
    if (rx_packet->header.chunk_number == rx_packet->header.total_chunks)
        comm_status.curr_cmd_state = CMD_STATE_RECEIVED;
    send_cmd_ack_packet(rx_packet);
    LOG_SWV("#ORG#bs=%d, cs=%d, seq=%d, ccn=%d, ccc=%d, rl=%d\n", CY_Usb_Buffer_Free(), comm_status.curr_cmd_state,
           comm_status.curr_cmd_seq_no, comm_status.curr_cmd_chunk_no, rx_packet->header.total_chunks,
           comm_status.curr_cmd_received_length);
    return NO_ERROR;
}

/**
 * @details Packet type: PKT_TYPE_OUT_REQ <br/>
 * Process the cmd output request based on the state of the application. This command cannot intervene
 * with the ongoing command execution; hence, in such a situation, the command is responded with current
 * status (i.e. status packet). The output is expected to be available only when the execution
 * is completed so it is expected that the curr_cmd_state is set to CMD_STATE_DONE.
 */
static comm_error_code_t comm_process_out_req_packet(const packet_t *rx_packet) {
    if (comm_status.curr_cmd_seq_no != rx_packet->header.sequence_no) return INVALID_SEQUENCE_NO;
    if (rx_packet->header.chunk_number != 1) return INVALID_CHUNK_NO;
    if (rx_packet->header.total_chunks != 1) return INVALID_CHUNK_COUNT;
    if (rx_packet->header.payload_length != 6) return INVALID_PAYLOAD_LENGTH;
    if (comm_status.curr_cmd_state != CMD_STATE_DONE && comm_status.curr_cmd_state != CMD_STATE_FAILED) {
        send_status_packet();
        return NO_ERROR;
    }
    if ((U16_READ_BE_ARRAY(rx_packet->payload + 4) - 1) * COMM_MAX_PAYLOAD_SIZE > comm_get_payload_size(comm_payload))
        return NO_MORE_CHUNKS;           // Invalid output chunk request

    send_cmd_output_packet(rx_packet);
    return NO_ERROR;
}

static comm_error_code_t comm_process_abort_packet(const packet_t *rx_packet) {
    if (rx_packet->header.chunk_number != 1) return INVALID_CHUNK_NO;
    if (rx_packet->header.total_chunks != 1) return INVALID_CHUNK_COUNT;
    if (rx_packet->header.payload_length != 0) return INVALID_PAYLOAD_LENGTH;
    if (!comm_status.abort_disabled) {
        comm_reset();
        CY_Reset_Flow();
        comm_status.curr_cmd_seq_no = rx_packet->header.sequence_no;
        sys_flow_cntrl_u.bits.usb_buffer_free = true;
    }

    send_status_packet();
    return NO_ERROR;
}

/**
 * @details Packet type: PKT_TYPE_STATUS_REQ <br/>
 * Respond with the current status of the application. This request will not interrupt the current
 * state of the application. This is a synchronisation mechanism over the USB protocol to help the host
 * identify the precise state of the application so that correct actions can be taken.
 */
static void send_status_packet() {
    uint8_t payload[COMM_MAX_PAYLOAD_SIZE], offset = 0;
    payload[offset++] = 0x00; payload[offset++] = 0x00;       // proto length
    payload[offset++] = 0x00; payload[offset++] = 0x00;       // dummy raw length
    payload[offset++] = comm_status.app_busy_status;
    payload[offset++] = comm_status.abort_disabled;
    payload[offset++] = (comm_status.curr_cmd_seq_no >> 8) & 0xFF;
    payload[offset++] = comm_status.curr_cmd_seq_no & 0xFF;
    payload[offset++] = comm_status.curr_cmd_state;
    payload[offset++] = (comm_status.curr_flow_status >> 8) & 0xFF;
    payload[offset++] = comm_status.curr_flow_status & 0xFF;
    memset(payload + offset, 0x00, 11); offset += 11;
    payload[2] = ((offset - 4) >> 8) & 0xFF; payload[3] = (offset - 4) & 0xFF;      // raw length
    ASSERT(offset <= COMM_MAX_PAYLOAD_SIZE);
    comm_write_packet(1, 1, 0xFFFF, PKT_TYPE_STATUS_ACK, offset, payload);
}

static void send_error_packet(const packet_t *rx_packet, const comm_error_code_t error_code) {
    LOG_SWV("#RED#Error: %d\r\n", error_code);
    uint8_t payload[3 * sizeof(uint16_t)] = {0}, offset = 0;
    payload[offset++] = 0x00; payload[offset++] = 0x00;       // proto length
    payload[offset++] = 0x00; payload[offset++] = 0x02;       // raw length
    payload[offset++] = (error_code >> 8) & 0xFF; payload[offset++] = error_code & 0xFF;
    comm_write_packet(1, 1, 0xFFFF, PKT_TYPE_ERROR, offset, payload);
}

static void send_cmd_ack_packet(const packet_t *rx_packet) {
    uint8_t payload[3 * sizeof(uint16_t)] = {0}, offset = 0;
    payload[offset++] = 0x00; payload[offset++] = 0x00;       // proto length
    payload[offset++] = 0x00; payload[offset++] = 0x02;       // raw length
    payload[offset++] = (comm_status.curr_cmd_chunk_no >> 8) & 0xFF; payload[offset++] = comm_status.curr_cmd_chunk_no & 0xFF;
    comm_write_packet(1, 1, rx_packet->header.sequence_no, PKT_TYPE_CMD_ACK, offset, payload);
}

static void send_cmd_output_packet(const packet_t *rx_packet) {
    ASSERT(comm_payload.raw_data != NULL || comm_payload.proto_data != NULL);
    uint16_t req_chunk_no = U16_READ_BE_ARRAY(rx_packet->payload + 4);      // payload already verified in the caller function
    uint16_t offset = (req_chunk_no - 1) * COMM_MAX_PAYLOAD_SIZE;
    uint16_t remaining_payload_length = comm_get_payload_size(comm_payload) - offset;
    uint8_t payload_size = CY_MIN(remaining_payload_length, COMM_MAX_PAYLOAD_SIZE);
    comm_write_packet(req_chunk_no, ceil(comm_get_payload_size(comm_payload) * 1.0 / COMM_MAX_PAYLOAD_SIZE),
                      rx_packet->header.sequence_no, PKT_TYPE_OUT_RESP, payload_size, comm_io_buffer + offset);
}

static void comm_write_packet(const uint16_t chunk_number, const uint16_t total_chunks, const uint16_t seq_no,
                              const uint8_t packet_type, const uint8_t payload_size, const uint8_t *payload) {
    uint8_t buffer[COMM_PKT_MAX_LEN] = {0}, index = 4;
    uint32_t crc = 0;

    buffer[COMM_HEADER_INDEX] = COMM_START_OF_HEADER; buffer[COMM_HEADER_INDEX + 1] = COMM_START_OF_HEADER;
    buffer[COMM_CHUNK_NO_INDEX] = (chunk_number >> 8) & 0xFF; buffer[COMM_CHUNK_NO_INDEX + 1] = chunk_number & 0xFF;
    buffer[COMM_CHUNK_COUNT_INDEX] = (total_chunks >> 8) & 0xFF; buffer[COMM_CHUNK_COUNT_INDEX + 1] = total_chunks & 0xFF;
    buffer[COMM_SEQ_NO_INDEX] = (seq_no >> 8) & 0xFF; buffer[COMM_SEQ_NO_INDEX + 1] = seq_no & 0xFF;
    buffer[COMM_PKT_TYPE_INDEX] = packet_type;
    buffer[COMM_TIMESTAMP_INDEX] = (uwTick >> 24) & 0xFF; buffer[COMM_TIMESTAMP_INDEX + 1] = (uwTick >> 16) & 0xFF;
    buffer[COMM_TIMESTAMP_INDEX + 2] = (uwTick >> 8) & 0xFF; buffer[COMM_TIMESTAMP_INDEX + 3] = uwTick & 0xFF;
    buffer[COMM_PAYLOAD_LEN_INDEX] = payload_size;

    memcpy(buffer + COMM_PAYLOAD_INDEX, payload, payload_size);
    while (index < payload_size + COMM_HEADER_SIZE) {
        crc = update_crc16(crc, buffer[index++]);
    }
    crc = update_crc16(crc, 0); crc = update_crc16(crc, 0);
    buffer[COMM_CHECKSUM_INDEX] = (crc >> 8) & 0xFF; buffer[COMM_CHECKSUM_INDEX + 1] = crc & 0xFF;
    lusb_write(buffer, payload_size + COMM_HEADER_SIZE);
}