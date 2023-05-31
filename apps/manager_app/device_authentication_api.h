/**
 * @file    device_authentication_api.h
 * @author  Cypherock X1 Team
 * @brief   Header file for APIs to support X1 vault device authentication
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef DEVICE_AUTHENTICATION_API_H
#define DEVICE_AUTHENTICATION_API_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <manager/core.pb.h>

#include "stddef.h"
#include "stdint.h"

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
 * @brief This function requests ATECC SE chip to sign the serial number using
 * the device root key
 *
 * @return manager_auth_device_response_t Postfix1, postfix2, device serial and
 * ECDSA signature of the device serial of the particular device
 */
manager_auth_device_response_t sign_serial_number(void);

/**
 * @brief This function requests ATECC SE chip to sign a random number XOR'ed
 * with the hash of the installed firmware on the device serial number using the
 * device root key
 *
 * @param challenge The random challenge of 32 bytes sent by the host
 * @return manager_auth_device_response_t Postfix1, postfix2, device serial and
 * ECDSA signature of the device serial of the particular device
 */
manager_auth_device_response_t sign_random_challenge(uint8_t *challenge);

#endif /* DEVICE_AUTHENTICATION_API_H */
