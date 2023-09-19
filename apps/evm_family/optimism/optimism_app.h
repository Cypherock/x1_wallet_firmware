/**
 * @file    optimism_app.h
 * @author  Cypherock X1 Team
 * @brief   Optimism application configuration
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef OPTIMISM_APP_H
#define OPTIMISM_APP_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "app_registry.h"
#include "evm_context.h"
/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/// Number of entries in whitelisted contracts list
#define OPTIMISM_WHITELISTED_CONTRACTS_COUNT 0

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
 * @brief Returns the config for Optimism chin app descriptor
 *
 * @return A const reference to cy_app_desc_t
 */
const cy_app_desc_t *get_optimism_app_desc();

#endif    // OPTIMISM_APP_H
