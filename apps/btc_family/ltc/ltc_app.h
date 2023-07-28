/**
 * @file    ltc_app.h
 * @author  Cypherock X1 Team
 * @brief   Litecoin app configuration
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef LTC_APP_H
#define LTC_APP_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "btc_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define COIN_LTC 0x80000002    // 2'

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
 * @brief Returns the config for Litecoin chain app
 *
 * @return A const reference to btc_config_t
 */
const btc_config_t *get_ltc_app();

#endif
