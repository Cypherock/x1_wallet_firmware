/**
 * @file    tron_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for tron app internal operations
 *          This file is defined to separate TRON's internal use functions,
 * flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef TRON_PRIV_H
#define TRON_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <tron/core.pb.h>
#include <stdint.h>

#include "tron_context.h"
//#include "solana_txn_helpers.h"
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
void tron_get_pub_keys(tron_query_t *query);

/**
 * @brief Handler for NEAR sign transaction function
 *
 * @param query Buffer containing USB query received from host
 */
void tron_sign_transaction(tron_query_t *query);





#endif /* TRON_PRIV_H */

