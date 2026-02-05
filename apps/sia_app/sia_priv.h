/**
 * @file    sia_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for sia app internal operations
 *          This file is defined to separate Sia's internal use
 * functions, flows, common APIs
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef SIA_PRIV_H
#define SIA_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <sia/core.pb.h>
#include <stdint.h>

#include "sia_context.h"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by sia_handle_initiate_query()
   */
  sia_sign_txn_initiate_request_t init_info;

  // remembers the allocated buffer for holding complete unsigned transaction
  uint8_t *transaction;

  // decoded transaction structures
  sia_transaction_t *txn;

  // holds the length of the unsigned txn used for signing
  uint32_t tagged_txn_len;
} sia_txn_context_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Handler for Sia public key derivation.
 * @details This flow expects SIA_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void sia_get_pub_keys(sia_query_t *query);

/**
 * @brief Handler for signing a transaction on sia.
 * @details The expected request type is SIA_SIGN_TXN_REQUEST_INITIATE_TAG.
 * The function controls the complete data exchange with host, user prompts and
 * confirmations for signing an SIA based transaction.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void sia_sign_transaction(sia_query_t *query);

#endif /* SIA_PRIV_H */