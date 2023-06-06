/**
 * @file    host_interface.h
 * @author  Cypherock X1 Team
 * @brief   Header file for the host interface for the main-menu
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef HOST_INTERFACE_H
#define HOST_INTERFACE_H

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
 * @brief Host interface for USB events at main menu
 * @details
 *
 * @param ctx
 * @param usb_evt
 * @param data
 */
void main_menu_host_interface(engine_ctx_t *ctx,
                              usb_event_t usb_evt,
                              const void *data);

#endif
