/**
 * @file    btc_app.h
 * @author  Cypherock X1 Team
 * @brief   Bitcoin app configuration
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BTC_APP_H
#define BTC_APP_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "btc_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define COIN_BTC 0x80000000    // 0'

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
 * @brief Returns the config for Bitcoin chain app
 *
 * @return A const reference to btc_config_t
 */
const btc_config_t *get_btc_app();

#endif
