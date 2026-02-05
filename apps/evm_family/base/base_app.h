/**
 * @file    base_app.h
 * @author  Cypherock X1 Team
 * @brief   Base chain application configuration
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BASE_APP_H
#define BASE_APP_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "app_registry.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/* Number of entries in whitelisted contracts list */
#define BASE_WHITELISTED_CONTRACTS_COUNT 0

#define BASE_NETWORK_NAME "Base"

#define BASE_CURRENCY_SYMBOL "ETH"

/*
 * Chain ids for BASE
 * @ref https://docs.base.org/base-chain/quickstart/connecting-to-base
 */
#define BASE_CHAIN_ID_MAINNET 8453
#define BASE_CHAIN_ID_TESTNET 84532

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
 * @brief Returns the config for Base Chain app descriptor
 *
 * @return A const reference to cy_app_desc_t
 */
const cy_app_desc_t *get_base_app_desc();

#endif    // BASE_APP_H
