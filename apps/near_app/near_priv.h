/**
 * @file    near_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for NEAR app internal operations
 *          This file is defined to separate NEAR's internal use functions,
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
#include <stdint.h>

#include "near_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by handle_initiate_query()
   */
  near_sign_txn_initiate_request_t init_info;

  /**
   * This member holds pointer to the dynamically allocated structure where-in
   * the unsigned transaction is stored
   * @note Populated by fetch_valid_input()
   * TODO: Optimize fetching of unsigned txn as done for other coins and
   * dynamically allocate memory to hold the unsigned txn
   */
  near_sign_txn_unsigned_txn_t unsigned_txn;

  /**
   * This member holds decoded information which is extracted from the unsigned
   * transaction byte array. It is used to display user facing confirmations
   * before signing the transaction.
   * @note Populated by get_user_verification()
   */
  near_unsigned_txn decoded_txn;
} near_txn_context_t;

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

/**
 * @brief Handler for NEAR sign transaction function
 *
 * @param query Buffer containing USB query received from host
 */
void near_sign_transaction(near_query_t *query);

#endif /* NEAR_PRIV_H */
