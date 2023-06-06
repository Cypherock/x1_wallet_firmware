/**
 * @file    btc_app.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef BTC_APP_H
#define BTC_APP_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <btc/core.pb.h>
#include <stddef.h>
#include <stdint.h>

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
 */
void btc_app_main(usb_event_t usb_evt);

#endif /* BTC_APP_H */
