/**
 * @file    restricted_host_interface.h
 * @author  Cypherock X1 Team
 * @brief   Header file for the host interface for the restricted flow
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef RESTRICTED_HOST_INTERFACE_H
#define RESTRICTED_HOST_INTERFACE_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "flow_engine.h"

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
 * @brief This is the USB callback called when the device core detects a valid
 * USB event when no application is booted. This function passes control to the
 * application, which can further communicate with the host. Control is passed
 * back to the callback once the flow is complete in the application.
 *
 * @param ctx The engine context* from which the flow is invoked
 * @param usb_evt The USB event object which triggered the callback
 * @param data_ptr Currently unused pointer set by the engine
 */
void restricted_host_interface(engine_ctx_t *ctx,
                               usb_event_t usb_evt,
                               const void *data);

#endif /* RESTRICTED_HOST_INTERFACE_H */
