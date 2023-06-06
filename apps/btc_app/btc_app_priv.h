/**
 * @file    btc_app_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for btc app internal operations
 *          This file is defined to separate Bitcoin's internal use functions,
 * flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BTC_APP_PRIV_H
#define BTC_APP_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "btc_api.h"

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
 *
 * @param query Reference to the decoded query struct from the host app
 */
void btc_get_xpub(btc_query_t *query);

/**
 * @brief Handler for card authentication, this flow expects
 * MANAGER_AUTH_CARD_REQUEST_INITIATE_TAG as initial query, otherwise the flow
 * is aborted
 *
 * @param query object for card auth query
 */
void btc_get_wallet_public_key(btc_query_t *query);

#endif
