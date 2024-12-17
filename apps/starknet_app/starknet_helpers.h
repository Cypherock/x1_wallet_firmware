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

#include "starknet_pedersen.h"

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
 * @details The function supports checking derivation paths for HD wallets
 * Types of derivation:
 * m/2645'/1195502025'/1148870696'/0'/0'/i
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
 * @brief Derives stark keys (public and/or private) from given seed
 *
 * @param private_key Stores derived stark private key
 * @param public_key Stores derived stark public key
 */
bool starknet_derive_key_from_seed(const uint8_t *seed,
                                   const uint32_t *path,
                                   const uint32_t path_length,
                                   uint8_t *private_key,
                                   uint8_t *public_key);

#endif    // STARKNET_HELPERS_H