/**
 * @file    xrp_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for xrp app internal operations
 *          This file is defined to separate XRP's internal use
 * functions, flows, common APIs
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef XRP_PRIV_H
#define XRP_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <xrp/core.pb.h>

#include "xrp_context.h"

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
 * @brief Handler for XRP public key derivation.
 * @details This flow expects XRP_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG as initial
 * query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void xrp_get_pub_keys(xrp_query_t *query);

#endif /* XRP_PRIV_H */