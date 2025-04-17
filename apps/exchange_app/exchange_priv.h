/**
 * @file    exchange_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for exchange app internal operations
 *          This file is defined to separate EXCHANGE's internal use
 * functions, flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef EXCHANGE_PRIV_H
#define EXCHANGE_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "exchange/core.pb.h"

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
#pragma pack(push, 1)
#pragma pack(pop)

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

void exchange_initiate_flow(exchange_query_t *query);

void exchange_get_signature(exchange_query_t *query);

void exchange_store_signature(exchange_query_t *query);

#endif
