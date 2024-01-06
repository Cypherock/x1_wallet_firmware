/**
 * @file    usb_api_priv.h
 * @author  Cypherock X1 Team
 * @brief   USB internal purpose APIs.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef USB_API_PRIV_H
#define USB_API_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "communication.h"
#if USE_SIMULATOR == 0
#include "libusb.h"
#else
#include "sim_usb.h"
#endif

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define COMM_START_OF_HEADER 0x55
#define COMM_PKT_MAX_LEN 64
#define COMM_MAX_PAYLOAD_SIZE (COMM_PKT_MAX_LEN - COMM_HEADER_SIZE)

#define COMM_SZ_RESERVED_SPACE 4
#define COMM_BUFFER_SIZE ((size_t)6 * 1024)

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

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
  APP_BUSY_WITH_OTHER_INTERFACE = 12,
} comm_error_code_t;

typedef enum comm_cmd_state {
  CMD_STATE_NONE = 0,
  CMD_STATE_RECEIVING = 1,
  CMD_STATE_RECEIVED = 2,
  CMD_STATE_EXECUTING = 3,
  CMD_STATE_DONE = 4,
  CMD_STATE_FAILED = 5,
  CMD_STATE_INVALID_REQ = 6,
} comm_cmd_state_t;

/**
 * @brief Communication Header struct
 * @details
 *
 * @see COMM_HEADER_SIZE
 * @since v1.0.0
 *
 * @note
 */
typedef struct comm_header {
  uint16_t start_of_header;    ///< 2-byte data representing start of packet
  uint16_t
      checksum; /** Checksum on (chunk_number + total_chunks + sequence_no
                   + This uses <a
                   href="https://mdfs.net/Info/Comp/Comms/CRC16.htm">CRC-16/XMODEM
                   algorithm</a> */
  uint16_t chunk_number;     ///< 2-byte data representing the current chunk
                             ///< number. For the first chunk, this is 1.
  uint16_t total_chunks;     ///< 2-byte data representing the total number of
                             ///< chunks. For single packet data, this is 1.
  uint16_t sequence_no;      ///< Usb host app request sequence number. -1 for
                             ///< packet types PKT_TYPE_STATUS_REQ,
                             ///< PKT_TYPE_STATUS_ACK & PKT_TYPE_ERROR.
  uint8_t packet_type;       ///< Defines the packet type. @see comm_packet_type
  uint8_t payload_length;    ///< Length of the payload in the current packet
  uint32_t timestamp;        /** Packet preparation timestamp of host machine in
                                milliseconds. Each entity (host & peripheral) will
                                reset to 0 across resets. */
} comm_header_t;

/**
 * @brief struct for message received from desktop
 * @details This is the application buffer for the message received from the
 * desktop application. The entire payload is aggregated into this buffer before
 * handing it over to the application. Similarly, the response from the
 * application is stored into this buffer before sending it to the desktop
 * application. The aggregation and fragmentation is handled by the
 * communication layer. For members of the raw serialized message, refer
 * comm_raw_message_t.
 *
 * @see packet_t, comm_raw_payload_t
 * @since v1.0.0
 *
 * @note
 */
typedef struct comm_payload {
  uint16_t proto_data_length;    ///< Length of the protobuf serialized data in
                                 ///< the cmd payload
  uint16_t raw_data_length;    ///< Length of the raw serialization data in the
                               ///< cmd payload
  uint8_t *proto_data;    ///< Protobuf serialization data in the cmd payload
  uint8_t *raw_data;      ///< Raw serialization data in the cmd payload
} comm_payload_t;

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
  comm_libusb__interface_e interface;
} packet_t;

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

  // Host interface while receiving data and while an application is in progress
  comm_libusb__interface_e active_interface;
} comm_status_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Returns the reference to internal instance of io_buffer
 */
uint8_t *get_io_buffer();

/**
 * @brief Returns the reference to internal instance of comm_payload
 */
comm_payload_t *get_comm_payload();

/**
 * @brief Resets the active interface(to COMM_LIBUSB__UNDEFINED) used to
 * determine which interface is allowed to send new commands. The interface must
 * be reset when an application is clsoed to allow any interface to communicate
 * with device.
 */
void comm_reset_interface(void);

/**
 * @brief Returns the reference to internal instance of comm_status
 */
comm_status_t *get_comm_status();

void comm_set_payload_struct(uint16_t proto_len, uint16_t raw_len);

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
 */
uint16_t update_crc16(uint16_t crc_in, uint8_t byte);

/**
 * @brief Send a packet to the host
 * @details This function aggregates a received packet from the host.
 *
 * @param data
 * @param length
 */
void comm_packet_parser(const uint8_t *data,
                        const uint16_t length,
                        comm_libusb__interface_e interface);

/**
 * @brief Setter for usb_event object
 * @details When the usb data is completely received in the interrupt, the event
 * will be registered with the usb_event module.
 */
void usb_set_event(uint16_t core_msg_size,
                   const uint8_t *core_msg_buffer,
                   uint16_t app_msg_size,
                   const uint8_t *app_msg);

#ifndef OLD_USB_API_H    // TODO: Update after refactor; Remove me
/**
 * @brief Clear message from desktop.
 * Sets 0 to variable storing message from desktop.
 * @details
 *
 * @see
 * @since v1.0.0
 */
void usb_free_msg_buffer();

/**
 * @brief Resets the internal command processing state of usb exchange
 *
 */
void usb_reset_state();
#endif

/**
 * @brief Sets the internal state for a command to executing.
 * @details
 */
void usb_set_state_executing();

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
void comm_process_packet(const packet_t *rx_packet);

void send_error_packet(const packet_t *rx_packet, comm_error_code_t error_code);

#endif