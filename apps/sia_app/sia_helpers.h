/**
 * @file    sia_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for Sia chains
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef SIA_HELPERS_H
#define SIA_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "blake2b.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define SIA_IMPLICIT_ACCOUNT_DEPTH 1

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
 * @details Sia uses simple index-based derivation, not BIP44.
 * Just validates that we have a single index parameter.
 *
 * @param[in] path      The derivation path (just an index)
 * @param[in] levels    The number of levels (should be 1 for Sia)
 *
 * @return bool Indicates if the provided derivation path is valid
 * @retval true if the derivation path is valid
 * @retval false otherwise
 */
bool sia_derivation_path_guard(const uint32_t *path, uint8_t levels);

/**
 * @brief Generate full Sia address string from 32-byte address hash
 * @param address_hash 32-byte address hash from transaction output
 * @param full_address Output buffer for 76-character address + null terminator
 * @return bool True if generation successful, false otherwise
 */
bool sia_full_address(const uint8_t *address_hash, char *full_address);

/**
 * @brief Generates a Sia address from a public key
 * @details Follows the Sia address generation algorithm:
 * 1. Creates unlock condition Merkle tree (timelock + pubkey + sigsRequired)
 * 2. Calculates Blake2b hash of the tree
 * 3. Adds 6-byte checksum and converts to hex
 *
 * See https://github.com/SiaFoundation/core/blob/master/types/hash.go#L94
 * @param public_key The 32-byte ED25519 public key
 * @param address Buffer to store the resulting address (must be at least
 * SIA_ADDRESS_SIZE bytes)
 * @return true if the address was generated successfully, false otherwise
 */
bool sia_generate_address(const uint8_t *public_key, char *address);

#endif    // SIA_HELPERS_H