/**
 * @file    starknet_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for Starknet chains
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef STARKNET_HELPERS_H
#define STARKNET_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "coin_utils.h"
#include "starknet_crypto.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define LOW_PART_BITS 248
#define LOW_PART_BYTES (LOW_PART_BITS / 8)
#define LOW_PART_MASK ((1ULL << LOW_PART_BITS) - 1)

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
 * @details The function supports checking derivation paths for HD wallets
 * Types of derivations:
 * address: m/44'/501'
 * address: m/44'/501'/i'
 * address: m/44'/501'/i'/0'
 *
 * @param[in] path      The derivation path as an uint32 array
 * @param[in] depth     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided derivation path is valid
 * @retval true if the derivation path is valid
 * @retval false otherwise
 */
bool starknet_derivation_path_guard(const uint32_t *path, uint8_t levels);

/**
 *
 */
bool starknet_derive_bip32_node(const uint8_t *seed, uint8_t *private_key);

/**
 *
 */
bool starknet_derive_key_from_seed(const uint8_t *seed,
                                   const uint32_t *path,
                                   uint32_t path_length,
                                   uint8_t *private_key);

bool pederson_hash(uint8_t *x, uint8_t *y, uint8_t len, uint8_t *hash);
bool get_stark_child_node(const uint32_t *path,
                                 const size_t path_length,
                                 const char *curve,
                                 const uint8_t *seed,
                                 const uint8_t seed_len,
                                 HDNode *hdnode);

#endif    // STARKNET_HELPERS_H