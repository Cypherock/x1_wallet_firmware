/**
 * @file    usb_api.h
 * @author  Cypherock X1 Team
 * @brief   USB internal purpose APIs.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef USB_API_H
#define USB_API_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "usb_cmd_ids.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define ALLOW_LOG_EXPORT

#define MAXIMUM_DATA_SIZE 40
#define PKT_HEAD_SIZE 4
#define BYTE_STUFFED_DATA_SIZE ((MAXIMUM_DATA_SIZE + PKT_HEAD_SIZE) * 2)
#define COMM_HEADER_SIZE 16
#define DATA_SIZE_INDEX (COMM_HEADER_SIZE - 1)
#define COMM_HEADER_SIZE_V0 (sizeof(comm_header_v0_t))
#define DATA_SIZE_INDEX_V0 (COMM_HEADER_SIZE_V0 - 1)

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef enum cy_app_task {
  CY_UNUSED_TASK = 0x00,
  CY_APP_IDLE_TASK = 0x01,
  CY_APP_USB_TASK = 0x02,
  CY_APP_DEVICE_TASK = 0x03,
} cy_app_task_t;

typedef enum cy_app_status {
  CY_UNUSED_STATE = 0x00,
  CY_APP_IDLE = 0x10,
  CY_APP_WAIT_FOR_CARD = 0x20,
  CY_APP_WAIT_USER_INPUT = 0x30,
  CY_APP_BUSY = 0x40,
} cy_app_status_t;

typedef struct {
  uint8_t flag;
  En_command_type_t cmd_id;
  uint16_t msg_size;
  uint8_t *p_msg;
} usb_event_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

// TODO: Demo class like implementation

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Initialize communication by registering USB receive function in libusb
 *
 * @since v1.0.0
 */
void usb_init();

/**
 * @brief The function handles all the event cleanup operations.
 * @details The function will invalidate the existing usb event. In doing so, it
 * will clear the internal buffer and will also reset its the internal state of
 * USB module. The ideal place for purging is:
 *
 * <ol>
 *   <li> Firmware boot-up </li>
 *   <li> Application entry </li>
 *   <li> Application exit </li>
 *   <li> Events without any response </li>
 * </ol>
 */
void usb_clear_event();

/**
 * @brief Getter for USB event object.
 * @details The function is a simple getter to return a valid USB Event object
 * if and when a USB event is available and ready to dispatch at the USB module.
 * Event receiver is expected to show a necessary action to the usb module.
 * This is majorly due to the shared reference to the static buffer of usb
 * module. It is important to note that when an usb event is available, it is
 * not possible to receive any new events. If the host tries to send new data
 * (except some special data packets), it will receive an error. Hence, it is
 * necessary to clear the usb event after it is consumed. When an application
 * gets an usb event following scenarios might occur:
 *
 * <ol><li>
 * Valid data: <br/> With or without a response. See usb_send_data(),
 * usb_clear_event() respectively.
 * </li><li>
 * Invalid data: <br/> Respond with error code or Send error packet. See
 * usb_send_data(), usb_reject_invalid_request() respectively.
 * </li></ol>
 */
bool usb_get_event(usb_event_t *evt);

/**
 * @brief
 * @details
 *
 * @see
 * @since v1.0.0
 */
void usb_reject_invalid_request();

/**
 * @brief
 * @details
 *
 * @param cmd   .
 * @param data  .
 * @param size  .
 *
 * @see
 * @since v1.0.0
 */
void usb_send_data(uint32_t cmd, const uint8_t *data, uint32_t size);

/**
 * @brief
 * @details
 *
 * @param cmd   .
 * @param data  .
 * @param size  .
 *
 * @see
 * @since v1.0.0
 */
void usb_send_byte(const uint32_t command_type, const uint8_t byte);

#endif