/**
 * @file    solana_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for solana app internal operations
 *          This file is defined to separate SOLANA's internal use functions,
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
#include "solana_txn_helpers.h"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by solana_handle_initiate_query()
   */
  solana_sign_txn_initiate_request_t init_info;

  /// remembers the allocated buffer for holding complete unsigned transaction
  uint8_t *transaction;
  /// store for decoded unsigned transaction info
  solana_unsigned_txn transaction_info;
} solana_txn_context_t;

/**
 * @brief Handler for SOLANA public key derivation.
 * @details This flow expects SOLANA_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void solana_get_pub_keys(solana_query_t *query);

/**
 * @brief Handler for signing a transaction on Solana.
 * @details The expected request type is SOLANA_SIGN_TXN_REQUEST_INITIATE_TAG.
 * The function controls the complete data exchange with host, user prompts and
 * confirmations for signing an Solana based transaction.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void solana_sign_transaction(solana_query_t *query);

#endif /* SOLANA_PRIV_H */