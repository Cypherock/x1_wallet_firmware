/**
 * @file    cardano_priv.h
 * @author  Cypherock X1 Team
 * @brief   Headers for cardano top level functions
 * @copyright Copyright (c) 2026 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARDANO_PRIV_H
#define CARDANO_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "cardano/core.pb.h"

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
 * @brief Handler for Cardano public key derivation.
 * @details This flow expects CARDANO_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void cardano_get_pub_keys(cardano_query_t *query);

/**
 * @brief Handler for Cardano transaction signing.
 * @details This flow expects CARDANO_QUERY_SIGN_TXN_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query object for transaction signing query
 */
void cardano_sign_transaction(cardano_query_t *query);

#endif    // CARDANO_PRIV_H
