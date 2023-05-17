/**
 * @file    communication.h
 * @author  Cypherock X1 Team
 * @brief   USB communication interface APIs.
 *          Exposes the APIs for enabling USB communication at application
 * layer.
 * @copyright Copyright (c) 2022-2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef _COMMUNICATION
#define _COMMUNICATION
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "usb_api.h"

/**
 * @brief Set device state.
 * @details
 *
 * @param state
 * @param flow_status
 *
 * @see
 * @since v1.0.0
 */
void mark_device_state(cy_app_status_t state, uint8_t flow_status);

/**
 * @brief Check if device is ready.
 * @details
 *
 * @return device ready state.
 * @return true for device is ready.
 * @return false for device is not ready.
 * @retval
 *
 * @see
 * @since v1.0.0
 */
bool is_device_ready();

/* START TODO: Update after refactor */
/*
 * Replace the existing functions with their new alternatives
 * comm_init                -->  usb_init
 * get_usb_msg_by_cmd_type  -->  usb_get_event
 * get_usb_msg              -->  usb_get_event
 * comm_process_complete    -->  usb_clear_event
 * get_usb_msg              -->  usb_get_event
 * comm_reject_invalid_cmd  -->  usb_reject_invalid_request
 */
// Remove the following definition to enable definitions in
#define OLD_USB_API_H

#define comm_init usb_init
#define transmit_one_byte usb_send_byte
/**
 * @brief This function allows to fetch any available transaction. The caller
 * should provide storage to get the command-type, any accompanying data and
 * length of byte array.
 * @details
 *
 * @param[out] command_type  En_command_type_t to fetch the type of operation
 * @param[out] msg_data      Pointer to store message array location
 * @param[out] msg_len       Length of message contained in the command received
 *
 * @return true, false
 * @retval false             If exactly one out of msg_data and msg_len is NULL
 * @retval true              If the message is available
 * @retval false             If the message in not available
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
bool usb_get_msg(En_command_type_t *command_type,
                 uint8_t **msg_data,
                 uint16_t *msg_len);

#define get_usb_msg usb_get_msg

/**
 * @brief This function allows to fetch a specific command type. The caller
 * should specify the command type, and length of expected data.
 * @details
 *
 * @param command_type  En_command_type_t for which to look for
 * @param msg_data      Pointer to store message array location
 * @param msg_len       Length of message contained in the command received
 *
 * @return true, false
 * @retval false        If exactly one out of msg_data and msg_len is NULL
 * @retval true         If the requested message is available
 * @retval false        If the requested message in not available
 *
 * @see
 * @since v1.0.0
 *
 * @note This should be deprecated
 */
bool get_usb_msg_by_cmd_type(En_command_type_t command_type,
                             uint8_t **msg_data,
                             uint16_t *msg_len);

#define clear_message_received_data()
#define comm_process_complete usb_reset_state

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

/**
 * @brief Reject any ongoing flow.
 * @details This will internally mark the comm_cmd_state to REJECTED. The point
 * of rejection is conveyed to the desktop based on the value of
 * curr_flow_status.
 *
 * @see
 * @since v1.0.0
 */
void comm_reject_request(En_command_type_t command_type, uint8_t byte);

#define transmit_one_byte_confirm(a) usb_send_byte(a, 1)
#define comm_reject_invalid_cmd()

#define transmit_data_to_app usb_send_data
/* END TODO: Update after refactor */

#endif    //_COMMUNICATION
