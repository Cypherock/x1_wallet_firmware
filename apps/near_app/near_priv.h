/**
 * @file    near_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for NEAR app internal operations
 *          This file is defined to separate Bitcoin's internal use functions,
 * flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef NEAR_PRIV_H
#define NEAR_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <near/core.pb.h>

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
 * @brief Handler for NEAR public key derivation.
 * @details This flow expects BTC_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG as initial
 * query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void near_get_pub_keys(near_query_t *query);

#endif /* NEAR_PRIV_H */
