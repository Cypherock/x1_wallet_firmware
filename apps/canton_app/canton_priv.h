/**
 * @file    canton_priv.h
 * @author  Cypherock X1 Team
 * @brief   Canton private api headers
 *
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CANTON_PRIV_H
#define CANTON_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdint.h>

#include "canton/core.pb.h"
#include "canton_context.h"
#include "canton_txn_encoding.h"
#include "sha2.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * This is populated by `handle_initiate_query` present in canton_txn.c
   */
  canton_sign_txn_initiate_request_t init_info;

  canton_unsigned_txn unsigned_txn;

  uint8_t encoded_txn[ENCODED_TXN_LENGTH];
} canton_txn_context_t;

typedef struct {
  canton_sign_topology_txn_initiate_request_t init_info;

  canton_unsigned_topology_txn unsigned_topology_txn;

} canton_topology_txn_context_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/
extern canton_txn_context_t *canton_txn_context;

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Handler for Canton public key derivation.
 * @details This flow expects CANTON_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void canton_get_pub_keys(canton_query_t *query);

/**
 * @brief Entry point for sign_transaction type queries
 * @details Function handles complete flow required to sign and required
 * transaction, this includes: recieving further data, parsing and validating
 * data, user confirmation, returning data to the host. And incase of any error,
 * returns error to the host.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void canton_sign_transaction(canton_query_t *query);

/**
 * @brief Entry point for sign_topology_transaction type queries
 * @details Function handles complete flow required to sign and required
 * transaction, this includes: recieving further data, parsing and validating
 * data, user confirmation, returning data to the host. And incase of any error,
 * returns error to the host.
 */
void canton_sign_topology_transaction(canton_query_t *query);

#endif
