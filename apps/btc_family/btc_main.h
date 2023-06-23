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

#ifndef BTC_FAM_MAIN_H
#define BTC_FAM_MAIN_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <btc/core.pb.h>
#include <stddef.h>
#include <stdint.h>

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
 * @param app
 */
void btc_main(usb_event_t usb_evt, const btc_config_t *app);

#endif /* BTC_FAM_MAIN_H */
