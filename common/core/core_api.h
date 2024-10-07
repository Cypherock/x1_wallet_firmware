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
void send_core_error_msg_to_host(uint32_t core_error_type);

/**
 * @brief Helper API for core to send a response containing a list of
 * application versions to host.
 *
 * @param version_resp A pointer to a structure of type
 * core_app_version_result_response_t.
 */
void send_app_version_list_to_host(
    const core_app_version_result_response_t *version_resp);

/**
 * @brief Sends a response to the host after receiving a session start payload.
 *
 * This function constructs a core message containing the confirmation
 * initiation response for a session start. It extracts relevant information
 * from the payload, such as device random public key, device ID, signature, and
 * postfix values.
 *
 * @param payload Pointer to the received payload data.
 */
void send_core_session_start_response_to_host(const uint8_t *payload);

/**
 * @brief Sends an acknowledgment to the host after successfully starting a
 * session.
 *
 * This function constructs a core message with the confirmation start response
 * for a session start. It indicates that the session has been initiated
 * successfully.
 */
void send_core_session_start_ack_to_host();

/**
 * @brief Sends a response to the host when closing a session.
 *
 * This function constructs a core message to acknowledge the session close
 * request. It specifies that the session should be cleared.
 */
void send_core_session_close_response_to_host();

#endif /* CORE_API_H */
