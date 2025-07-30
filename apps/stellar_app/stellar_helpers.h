/**
 * @file    stellar_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for Stellar chains
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef STELLAR_HELPERS_H
#define STELLAR_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "coin_utils.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define STELLAR_IMPLICIT_ACCOUNT_DEPTH 3

// BIP44 derivation path: m/44'/148'/n'
// See https://github.com/satoshilabs/slips/blob/master/slip-0044.md
#define STELLAR_PURPOSE_INDEX 0x8000002C    // 44'
#define STELLAR_COIN_INDEX 0x80000094       // 148'
#define STELLAR_ACCOUNT_INDEX 0x80000000    // 0'

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
 * @details The derivation depth is fixed at level 3. So if the depth level !=
 * 3, then this function return false indicating invalid derivation path. The
 * function supports checking derivation paths for HD wallets Types of
 * derivations: address: m/44'/148'/0'
 * See https://developers.stellar.org/docs/fundamentals-and-concepts/accounts
 *
 * @param[in] path      The derivation path as an uint32 array
 * @param[in] levels     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided derivation path is valid
 * @retval true if the derivation path is valid
 * @retval false otherwise
 */
bool stellar_derivation_path_guard(const uint32_t *path, uint8_t levels);

/**
 * @brief Generates a Stellar address from a public key
 * @details Follows the Stellar address generation algorithm:
 * 1. Creates a payload with account ID type (0x30) and the public key
 * 2. Calculates CRC16 checksum
 * 3. Encodes the result using base32
 * See
 * https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0023.md
 *
 * @param public_key The 32-byte ED25519 public key
 * @param address Buffer to store the resulting address (must be at least
 * STELLAR_ADDRESS_LENGTH bytes)
 * @return true if the address was generated successfully, false otherwise
 */
bool stellar_generate_address(const uint8_t *public_key, char *address);

#endif    // STELLAR_HELPERS_H