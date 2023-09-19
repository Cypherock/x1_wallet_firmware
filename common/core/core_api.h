/**
 * @file    core_api.h
 * @author  Cypherock X1 Team
 * @brief   Helpers for the application layer
 *
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CORE_API_H
#define CORE_API_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

#include "core.pb.h"

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
 * @brief Helper API for the X1 Vault applications to send messages to the
 * USB host.
 * @details The function adds the core message as a header to the application
 * message which is useful for the host to understand the context of the
 * message.
 *
 * @param msg Refence to memory buffer where application message is located
 * @param size Size of the buffer which needs to be transferred to the host
 */
void send_response_to_host(const uint8_t *msg, const uint32_t size);

/**
 * @brief Helper API for core to send core error messages to the USB host.
 *
 * @param which_error The enum type of the error which needs to be sent
 */
void send_core_error_msg_to_host(uint32_t which_error);

/**
 * @brief Helper API for core to send a response containing a list of
 * application versions to host.
 *
 * @param version_resp A pointer to a structure of type
 * core_app_version_result_response_t.
 */
void send_app_version_list_to_host(
    const core_app_version_result_response_t *version_resp);
#endif /* CORE_API_H */