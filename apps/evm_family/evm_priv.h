/**
 * @file    evm_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for evm app internal operations
 *          This file is defined to separate EVM's internal use functions,
 * flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef EVM_PRIV_H
#define EVM_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "events.h"
#include "evm_api.h"
#include "evm_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define EVM_TRANSACTION_SIZE_CAP 20480

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by handle_initiate_query()
   */
  evm_sign_txn_initiate_request_t init_info;

  /// remembers the allocated buffer for holding complete unsigned transaction
  uint8_t *transaction;
  /// store for decoded unsigned transaction info
  evm_unsigned_txn transaction_info;
} evm_txn_context_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

extern const evm_config_t *g_evm_app;

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Handler for getting public keys for EVM.
 * @details This flow expects EVM_GET_PUBLIC_KEYS_REQUEST_INITIATE_TAG as
 * initial query, otherwise the flow is aborted
 *
 * @param query Reference to the decoded query struct from the host app
 */
void evm_get_pub_keys(evm_query_t *query);

/**
 * @brief Handler for signing a transaction on EVM.
 * @details The expected request type is EVM_SIGN_TXN_REQUEST_INITIATE_TAG. The
 * function controls the complete data exchange with host, user prompts and
 * confirmations for signing an EVM based transaction.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void evm_sign_transaction(evm_query_t *query);

#endif /* EVM_PRIV_H */
