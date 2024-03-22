/**
 * @file    bittensor_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for bittensor app internal operations
 *          This file is defined to separate BITTENSOR's internal use functions,
 * flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BITTENSOR_PRIV_H
#define BITTENSOR_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <bittensor/core.pb.h>
#include <stdint.h>

#include "bittensor_context.h"
#include "bittensor_txn_helpers.h"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by bittensor_handle_initiate_query()
   */
  bittensor_sign_txn_initiate_request_t init_info;

  /// remembers the allocated buffer for holding complete unsigned transaction
  uint8_t *transaction;
  /// store for decoded unsigned transaction info
  bittensor_unsigned_txn transaction_info;
} bittensor_txn_context_t;

/**
 * @brief Handler for BITTENSOR public key derivation.
 * @details This flow expects BITTENSOR_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void bittensor_get_pub_keys(bittensor_query_t *query);

/**
 * @brief Handler for signing a transaction on Bittensor.
 * @details The expected request type is
 * BITTENSOR_SIGN_TXN_REQUEST_INITIATE_TAG. The function controls the complete
 * data exchange with host, user prompts and confirmations for signing an
 * Bittensor based transaction.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void bittensor_sign_transaction(bittensor_query_t *query);

#endif /* BITTENSOR_PRIV_H */