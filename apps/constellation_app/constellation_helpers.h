/**
 * @file    constellation_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for CONSTELLATION chains
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CONSTELLATION_HELPERS_H
#define CONSTELLATION_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "constellation/sign_txn.pb.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Verifies the derivation path.
 * @details The derivation depth is fixed at level 5. So if the depth level !=
 * 5, then this function return false indicating invalid derivation path. The
 * function supports checking derivation paths for HD wallets Types of
 * derivations: address: m/44'/144'/0'/0/i
 *
 * @param[in] path      The derivation path as an uint32 array
 * @param[in] levels     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided derivation path is valid
 * @retval true if the derivation path is valid
 * @retval false otherwise
 */
bool constellation_derivation_path_guard(const uint32_t *path, uint8_t levels);

/**
 * @brief Generates the DAG address of an uncompressed public key.
 * @details Sends an error to the host if the address can not be encoded.
 *
 * @param[out] address  Buffer of CONSTELLATION_ACCOUNT_ADDRESS_SIZE + 1 bytes
 * @param[in] pubkey    Uncompressed public key (CONSTELLATION_PUB_KEY_SIZE)
 *
 * @return bool true if the address was generated
 */
bool generate_dag_address(char *address, const uint8_t *pubkey);

/**
 * @brief Serializes a constellation_transaction_t.
 * @details Encodes the transaction as per dag4.js library and
 * serializes the encoded txn with kryo_serialize
 *
 * @param[in] txn         The constellation_transaction_t to serialize
 * @param[out] output     The buffer to hold the serialized txn
 * @param[out] output_len The length of the output serialized txn
 *
 * @return None
 */
void serialize_txn(const constellation_transaction_t *txn,
                   uint8_t *output,
                   size_t *output_len);

#endif    // CONSTELLATION_HELPERS_H