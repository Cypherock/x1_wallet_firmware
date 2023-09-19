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
#define COMM_HEADER_SIZE 16
#define DATA_SIZE_INDEX (COMM_HEADER_SIZE - 1)

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
  uint16_t msg_size;
  const uint8_t *p_msg;
} usb_event_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

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
 * In case the event received was a core request, a response is returned to the
 * host and false is returned to the caller.
 *
 * NOTE:
 * Event receiver is expected to show a necessary action to the usb module.
 * This is majorly due to the shared reference to the static buffer of usb
 * module. It is important to note that when an usb event is available, it is
 * not possible to receive any new events. If the host tries to send new data
 * (except some special data packets), it will receive an error. Hence, it is
 * necessary to clear the usb event after it is consumed. When an application
 * gets an usb event following scenarios might occur:
 *
 * <ol><li>
 * Valid data: <br/> With or without a response. See usb_send_msg(),
 * usb_clear_event() respectively.
 * </li><li>
 * Invalid data: <br/> Respond with error code or Send error packet. See
 * usb_send_msg()
 * </li></ol>
 *
 * @return true if USB event occured with CORE_MSG_CMD_TAG tag, false if no
 * event or core request was processed.
 */
bool usb_get_event(usb_event_t *evt);

/**
 * @brief Sends data stream to the host application over usb.
 * @details Allows applications to send data to the host. The functions
 * internally clears any existing usb event. This is due to the common buffer
 * used by the receive and transmit actions over usb. Hence, the applications
 * should make sure that any existing usb events are consumed before sending any
 * data.
 *
 * @param core_msg Reference to the buffer of encoded data to be sent to the
 * host application at the core msg layer.
 * @param core_msg_size Size of the message length to be sent to the host
 * application at the core msg layer.
 * @param app_msg Reference to the buffer of encoded data to be sent to the
 * host application at the app msg layer.
 * @param app_msg_size Size of the message length to be sent to the host
 * application at the core msg layer.
 */
void usb_send_msg(const uint8_t *core_msg,
                  uint32_t core_msg_size,
                  const uint8_t *app_msg,
                  uint32_t app_msg_size);

// TODO: Update after refactor; remove the following
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
// TODO: Update after refactor; remove the following
void usb_send_byte(uint32_t command_type, uint8_t byte);

#endif