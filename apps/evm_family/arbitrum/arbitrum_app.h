/**
 * @file    arbitrum_app.h
 * @author  Cypherock X1 Team
 * @brief   Arbitrum application configuration
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef ARBITRUM_APP_H
#define ARBITRUM_APP_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "evm_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/// Number of entries in whitelisted contracts list
#define ARBITRUM_WHITELISTED_CONTRACTS_COUNT 0

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
 * @brief Returns the config for Arbitrum chain app
 *
 * @return A const reference to evm_config_t
 */
const evm_config_t *get_arbitrum_app();

#endif    // ARBITRUM_APP_H
