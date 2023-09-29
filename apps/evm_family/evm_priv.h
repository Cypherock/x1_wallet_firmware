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

/**
 * TODO: update the size of msg data same as EVM_TRANSACTION_SIZE_CAP.
 * Constraints : The LVGL buffer cannot handle more than 3Kb data size which
 * puts a limit on how much data can be displayed on the device. Possible fix is
 * to show the long messages in chunks in line with max LVGL buffer size.
 */
#define MAX_MSG_DATA_SIZE 3072

/**
 * TODO: Update the size of typed data msg same as EVM_TRANSACTION_SIZE_CAP.
 * Constraints : For typed data implementation we use dynamic allocation for raw
 * data and then decode using nanopb, which again dynamically allocates data
 * leading to double allocation. Also multiple strings are allocated space for
 * display. To safely perform sign operation, need to analyze the size
 * requirement and find alternatives to the double allocation issue.
 * One possible solution for double allocation is to use field callbacks when
 * decoding typed struct data.
 */
#define MAX_MSG_DATA_TYPED_DATA_SIZE 5120
/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct evm_sign_msg_context {
  /// @brief  Contains initialization data for evm sign msg received from host
  evm_sign_msg_initiate_request_t init;

  /// @brief  Pointer to msg data in raw format, size from init member is
  /// allocated dynamically with a max size cap of @ref MAX_MSG_DATA_SIZE
  uint8_t *msg_data;

  evm_sign_typed_data_struct_t typed_data;
} evm_sign_msg_context_t;

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

/**
 * @brief This function signs a message of type ETH Sign/Personal Sign/Typed
 * data(TODO) and sends the signature as a response.
 *
 * @param query Reference to the decoded query struct from the host app
 */
void evm_sign_msg(evm_query_t *query);

#endif /* EVM_PRIV_H */
