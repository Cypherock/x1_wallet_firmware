/**
 * @file    solana_main.h
 * @author  Cypherock X1 Team
 * @brief   Header file for a common entry point to various Solana coin actions
 supported.
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef SOLANA_MAIN_H
#define SOLANA_MAIN_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "events.h"
#include "solana_context.h"

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
 * @brief Entry point for the SOLANA application of the X1 vault. It is invoked
 * by the X1 vault firmware, as soon as there is a USB request raised for the
 * Solana app.
 *
 * @param usb_evt The USB event which triggered invocation of the bitcoin app
 */
void solana_main(usb_event_t usb_evt);

#endif /* SOLANA_MAIN_H */
