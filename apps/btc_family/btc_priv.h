/**
 * @file    btc_priv.h
 * @author  Cypherock X1 Team
 * @brief   Support for btc app internal operations
 *          This file is defined to separate Bitcoin's internal use functions,
 * flows, common APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BTC_PRIV_H
#define BTC_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <btc/core.pb.h>

#include "btc_context.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  bool filled;
  uint8_t hash_prevouts[32];
  uint8_t hash_sequence[32];
  uint8_t hash_outputs[32];
} btc_segwit_cache_t;

typedef struct {
  pb_byte_t prev_txn_hash[32];
  uint32_t prev_output_index;
  uint64_t value;
  btc_sign_txn_input_script_pub_key_t script_pub_key;
  uint32_t sequence;
  uint32_t change_index;
  uint32_t address_index;
} btc_txn_input_t;

/**
 * @brief Struct to store details of Unsigned Transaction.
 * @details The structure sufficiently describes the transaction to be signed.
 * All the fields are required to be populated. The transaction fields should
 * match the standard specifications (refer:
 * https://en.bitcoin.it/wiki/Protocol_documentation#tx).
 */
typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by handle_initiate_query()
   */
  btc_sign_txn_initiate_request_t init_info;
  /**
   * The structure holds the top-level simple datatypes of the transaction.
   * Refer description of `TxIn` at
   * https://en.bitcoin.it/wiki/Protocol_documentation#tx,
   * https://developer.bitcoin.org/devguide/transactions.html,
   * https://developer.bitcoin.org/devguide/transactions.html#locktime-and-sequence-number,
   * https://developer.bitcoin.org/devguide/transactions.html#signature-hash-types,
   * @note Populated by fetch_transaction_meta()
   */
  btc_sign_txn_metadata_t metadata;
  // Populated for segwit transactions
  btc_segwit_cache_t segwit_cache;

  /**
   * The structure holds the outputs (TxOut) of the transaction. Refer
   * description of `OutPoint` at
   * https://en.bitcoin.it/wiki/Protocol_documentation#tx,
   * https://developer.bitcoin.org/devguide/transactions.html#standard-transactions,
   * https://github.com/bitcoin/bitcoin/blob/bc88f3ab903f8a748a7570a6e17579dc4e9ba791/src/policy/policy.cpp,
   * https://github.com/bitcoin/bitcoin/blob/bc88f3ab903f8a748a7570a6e17579dc4e9ba791/src/script/standard.cpp#L237
   * @note Populated by fetch_valid_output()
   */
  btc_sign_txn_output_t *outputs;
  /**
   * The structure holds the inputs (TxIn) of the transaction. Refer
   * description of `TxIn` at
   * https://en.bitcoin.it/wiki/Protocol_documentation#tx,
   * https://github.com/bitcoin/bitcoin/blob/bc88f3ab903f8a748a7570a6e17579dc4e9ba791/src/policy/policy.cpp
   * @note Populated by fetch_valid_input()
   */
  btc_txn_input_t *inputs;
  // track change output in the list of outputs for quick access
  int change_output_idx;
} btc_txn_context_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

extern const btc_config_t *g_btc_app;

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 *
 * @param query Reference to the decoded query struct from the host app
 */
void btc_get_xpub(btc_query_t *query);

/**
 * @brief Handler for bitcoin public key derivation.
 * @details This flow expects BTC_GET_PUBLIC_KEY_REQUEST_INITIATE_TAG as initial
 * query, otherwise the flow is aborted
 *
 * @param query object for address public key query
 */
void btc_get_pub_key(btc_query_t *query);

/**
 * @brief Handler for bitcoin transaction signing
 * @details This flow expects BTC_SIGN_TXN_REQUEST_INITIATE_TAG as initial
 * query, otherwise the flow is aborted
 *
 * @param query object for sign transaction query
 */
void btc_sign_transaction(btc_query_t *query);

#endif
