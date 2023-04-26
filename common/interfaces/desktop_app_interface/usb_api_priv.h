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

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Setter for usb_event object
 * @details When the usb data is completely received in the interrupt, the event
 * will be registered with the usb_event module.
 */
void usb_set_event(uint32_t cmd_id, const uint8_t *p_msg, uint16_t msg_size);

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

#endif