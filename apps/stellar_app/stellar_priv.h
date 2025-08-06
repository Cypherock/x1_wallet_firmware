/**
 * @file    stellar_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for stellar app internal operations
 *          This file is defined to separate Stellar's internal use
 * functions, flows, common APIs
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef STELLAR_PRIV_H
#define STELLAR_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <stellar/core.pb.h>

#include "stellar_context.h"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by stellar_handle_initiate_query()
   */
  stellar_sign_txn_initiate_request_t init_info;

  // remembers the allocated buffer for holding complete unsigned transaction
  uint8_t *transaction;

  // decoded transaction structures
  stellar_transaction_t *txn;

  // holds the length of the xdr txn used for signing
  uint32_t tagged_txn_len;
} stellar_txn_context_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Handler for Stellar public key derivation.
 * @details This flow expects STELLAR_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void stellar_get_pub_keys(stellar_query_t *query);

/**
 * @brief Handler for signing a transaction on stellar.
 * @details The expected request type is STELLAR_SIGN_TXN_REQUEST_INITIATE_TAG.
 * The function controls the complete data exchange with host, user prompts and
 * confirmations for signing an STELLAR based transaction.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void stellar_sign_transaction(stellar_query_t *query);

#endif /* STELLAR_PRIV_H */