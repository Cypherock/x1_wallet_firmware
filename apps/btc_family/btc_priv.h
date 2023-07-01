/**
 * @file    btc_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for btc app internal operations
 *          This file is defined to separate Bitcoin's internal use functions,
 * flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BTC_PRIV_H
#define BTC_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <btc/core.pb.h>

#include "btc_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

extern const btc_config_t *g_app;

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 *
 * @param query Reference to the decoded query struct from the host app
 */
void btc_xpub(btc_query_t *query);

/**
 * @brief Handler for bitcoin public key derivation.
 * @details This flow expects BTC_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG as initial
 * query, otherwise the flow is aborted
 *
 * @param query object for card auth query
 */
void btc_pub_key(btc_query_t *query);

#endif
