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
#ifndef OLD_USB_API_H    // TODO: Update after refactor; Remove me
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

#endif
#endif