/**
 * @file    starknet_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for starknet app internal operations
 *          This file is defined to separate Starknet's internal use functions,
 * flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef STARKNET_PRIV_H
#define STARKNET_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <starknet/core.pb.h>
#include <stdint.h>

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by starknet_handle_initiate_query()
   */
  starknet_sign_txn_initiate_request_t init_info;

  /// remembers the allocated buffer for holding complete unsigned transaction
  // TODO: malloc this for clear signing
  uint8_t transaction[32];
} starknet_txn_context_t;

/**
 * @brief Handler for STARKNET public key derivation.
 * @details This flow expects STARKNET_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void starknet_get_pub_keys(starknet_query_t *query);

/**
 * @brief Handler for signing a transaction on Starknet.
 * @details The expected request type is STARKNET_SIGN_TXN_REQUEST_INITIATE_TAG.
 * The function controls the complete data exchange with host, user prompts and
 * confirmations for signing an Starknet based transaction.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void starknet_sign_transaction(starknet_query_t *query);

#endif /* STARKNET_PRIV_H */