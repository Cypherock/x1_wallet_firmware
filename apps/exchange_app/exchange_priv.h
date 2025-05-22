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

/**
 * @brief Initiates the exchange flow by pushing data onto the composable app
 * queue.
 *
 * This function clears the existing queue, then pushes data for the receive
 * flow, fetching the signature, storing the signature, and finally the send
 * flow.
 *
 * @param query Pointer to the exchange query structure.
 */
void exchange_initiate_flow(exchange_query_t *query);

/**
 * @brief Retrieves the signature from the shared context and sends it as a
 * result.
 *
 * This function retrieves the signature stored in the shared context, copies it
 * into the result structure, and sends the result.
 *
 * @param query Pointer to the exchange query structure.
 */
void exchange_get_signature(exchange_query_t *query);

/**
 * @brief Stores the provided signature in the shared context.
 *
 * This function clears the existing shared context and copies the provided
 * signature into it.
 *
 * @param query Pointer to the exchange query structure containing the signature
 * to store.
 */
void exchange_store_signature(exchange_query_t *query);

/**
 * @brief Closes the exchange flow by clearing the composable app
 * queue and shared context.
 *
 * This function clears the existing queue, and the shared context.
 *
 * @param query Pointer to the exchange query structure.
 */
void exchange_close_flow(exchange_query_t *query);

#endif
