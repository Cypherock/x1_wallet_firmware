/**
 * @file    evm_main.h
 * @author  Cypherock X1 Team
 * @brief
 * @details
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef EVM_MAIN_H
#define EVM_MAIN_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <evm/core.pb.h>
#include <stddef.h>
#include <stdint.h>

#include "events.h"
#include "evm_context.h"

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
 * @brief Entry point for the EVM application of the X1 vault. It is invoked
 * by the X1 vault firmware, as soon as there is a USB request raised for the
 * EVM app.
 *
 * @param usb_evt The USB event which triggered invocation of the EVM app
 * @param app_config Const reference to the preferred evm app configuration to
 * govern appropriate runtime execution
 */
void evm_main(usb_event_t usb_evt, const void *app_config);

#endif /* EVM_MAIN_H */
