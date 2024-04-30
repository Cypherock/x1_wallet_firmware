/**
 * @file    tron_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for tron app internal operations
 *          This file is defined to separate tron's internal use functions,
 * flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef TRON_PRIV_H
#define TRON_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <tron/tron.pb.h>

#include "events.h"
#include "tron_api.h"
#include "tron_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by tron_handle_initiate_query()
   */
  tron_sign_txn_initiate_request_t init_info;

  // remembers the allocated buffer for holding complete unsigned transaction
  uint8_t *transaction;

  // decoded raw txn
  tron_transaction_raw_t *raw_txn;

} tron_txn_context_t;
/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Handler for getting public keys for TRON.
 * @details This flow expects TRON_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query Reference to the decoded query struct from the host app
 */
void tron_get_pub_keys(tron_query_t *query);

/**
 * @brief Handler for signing a transaction on tron.
 * @details The expected request type is TRON_SIGN_TXN_REQUEST_INITIATE_TAG. The
 * function controls the complete data exchange with host, user prompts and
 * confirmations for signing an TRON based transaction.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void tron_sign_transaction(tron_query_t *query);

#endif /* TRON_PRIV_H */
