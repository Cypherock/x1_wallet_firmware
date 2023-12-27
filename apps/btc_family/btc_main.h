/**
 * @file    btc_main.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef BTC_MAIN_H
#define BTC_MAIN_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "btc_context.h"
#include "events.h"

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
 * @brief Entry point for the bitcoin application of the X1 vault. It is invoked
 * by the X1 vault firmware, as soon as there is a USB request raised for the
 * bitcoin app.
 *
 * @param usb_evt The USB event which triggered invocation of the bitcoin app
 * @param app_config An immutable reference to the app configuration info
 */
void btc_main(usb_event_t usb_evt, const void *app_config);

#endif /* BTC_MAIN_H */
