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
#include "canton/sign_txn.pb.h"

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
  canton_sign_txn_init_request_t init_info;

  // raw transaction buffer
  uint8_t *transaction;

  // TODO: decoded unsigned txn
  // canton_unsigned_txn *raw_txn;

} canton_txn_context_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

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

#endif
