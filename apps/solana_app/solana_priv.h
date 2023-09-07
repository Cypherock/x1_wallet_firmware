/**
 * @file    evm_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for evm app internal operations
 *          This file is defined to separate EVM's internal use functions,
 * flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef SOLANA_PRIV_H
#define SOLANA_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <solana/core.pb.h>
#include <stdint.h>

#include "solana_context.h"

/**
 * @brief Handler for SOLANA public key derivation.
 * @details This flow expects BTC_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG as initial
 * query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void solana_get_pub_keys(solana_query_t *query);


#endif /* NEAR_PRIV_H */