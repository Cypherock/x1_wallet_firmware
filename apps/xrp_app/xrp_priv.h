/**
 * @file    xrp_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for xrp app internal operations
 *          This file is defined to separate XRP's internal use
 * functions, flows, common APIs
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef XRP_PRIV_H
#define XRP_PRIV_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <xrp/core.pb.h>

#include "xrp_context.h"

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by xrp_handle_initiate_query()
   */
  xrp_sign_txn_initiate_request_t init_info;

  // remembers the allocated buffer for holding complete unsigned transaction
  uint8_t *transaction;

  // decoded raw txn
  xrp_unsigned_txn *raw_txn;

} xrp_txn_context_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Handler for XRP public key derivation.
 * @details This flow expects XRP_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG as initial
 * query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void xrp_get_pub_keys(xrp_query_t *query);

/**
 * @brief Handler for signing a transaction on xrp.
 * @details The expected request type is XRP_SIGN_TXN_REQUEST_INITIATE_TAG. The
 * function controls the complete data exchange with host, user prompts and
 * confirmations for signing an XRP based transaction.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void xrp_sign_transaction(xrp_query_t *query);

#endif /* XRP_PRIV_H */