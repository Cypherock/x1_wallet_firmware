/**
 * @file    constellation_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for constellation app internal operations
 *          This file is defined to separate CONSTELLATION's internal use
 * functions, flows, common APIs
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CONSTELLATION_PRIV_H
#define CONSTELLATION_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <constellation/core.pb.h>
#include <stdint.h>

#include "constellation/sign_msg.pb.h"
#include "constellation_context.h"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by constellation_handle_initiate_query()
   */
  constellation_sign_txn_initiate_request_t init_info;

  const constellation_transaction_t *txn;

} constellation_txn_context_t;

typedef struct {
  /// @brief  Contains initialization data for constellation sign msg received
  /// from host
  constellation_sign_msg_initiate_request_t init;

  /// @brief  Pointer to msg data in raw format, size from init member is
  /// allocated dynamically with a max size cap of @ref MAX_MSG_DATA_SIZE
  uint8_t *msg_data;

} constellation_sign_msg_context_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Handler for CONSTELLATION public key derivation.
 * @details This flow expects CONSTELLATION_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG
 * as initial query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void constellation_get_pub_keys(constellation_query_t *query);

/**
 * @brief Handler for signing a transaction on constellation.
 * @details The expected request type is
 * CONSTELLATION_SIGN_TXN_REQUEST_INITIATE_TAG. The function controls the
 * complete data exchange with host, user prompts and confirmations for signing
 * an CONSTELLATION based transaction.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void constellation_sign_transaction(constellation_query_t *query);

/**
 * @brief This function signs a message with or without(blind sign)
 * user verification and sends the signature as a response.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void constellation_sign_msg(constellation_query_t *query);

#endif /* CONSTELLATION_PRIV_H */