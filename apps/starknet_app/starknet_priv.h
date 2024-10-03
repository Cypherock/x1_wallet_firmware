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

/**
 * @brief Derives a list of public key corresponding to the provided list of
 * derivation paths.
 * @details The function expects the size of list for derivation paths and
 * location for storing derived public keys to be a match with provided count.
 *
 * @param paths Reference to the list of
 * starknet_get_public_keys_derivation_path_t
 * @param count Number of derivation paths in the list and consequently,
 * sufficient space in memory for storing derived public keys.
 * @param seed Reference to a const array containing the seed
 * @param public_keys Reference to the location to store all the public keys to
 * be derived
 *
 * @return bool Indicating if the complete public keys list was derived
 * @retval true If all the requested public keys were derived.
 * @retval false If the public key derivation failed. This could be due to
 * invalid derivation path.
 */
bool fill_starknet_public_keys(
    const starknet_get_public_keys_derivation_path_t *paths,
    const uint8_t *seed,
    uint8_t public_keys[][STARKNET_PUB_KEY_SIZE],
    pb_size_t count);

#endif /* STARKNET_PRIV_H */