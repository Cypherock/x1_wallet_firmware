/**
 * @file    doge_app.h
 * @author  Cypherock X1 Team
 * @brief   Dogecoin app configuration
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef DOGE_APP_H
#define DOGE_APP_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "btc_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define COIN_DOGE 0x80000003    // 3'

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
 * @brief Returns the config for Dogecoin chain app
 *
 * @return A const reference to btc_config_t
 */
const btc_config_t *get_doge_app();

#endif
