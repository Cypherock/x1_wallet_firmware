/**
 * @file    dash_app.h
 * @author  Cypherock X1 Team
 * @brief   Dash app configuration
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef DASH_APP_H
#define DASH_APP_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "btc_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define COIN_DASH 0x80000005    // 5'

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
 * @brief Returns the config for Dash chain app
 *
 * @return A const reference to btc_config_t
 */
const btc_config_t *get_dash_app();

#endif
