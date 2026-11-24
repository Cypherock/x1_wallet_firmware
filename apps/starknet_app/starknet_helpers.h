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
 * @brief Checks that an address is the Argent account of a stark public key
 * @details The deploy-account flow shows the host-provided address; it must be
 * the user's own account (the same address the receive flow shows), or the
 * user would sign the deployment of an account they do not control.
 *
 * @param pub_key Stark public key (x coordinate, 32 bytes)
 * @param address Account address to check (32 bytes, big endian)
 * @return bool true if address is the key's Argent account address
 */
bool starknet_is_own_argent_address(const uint8_t *pub_key,
                                    const uint8_t *address);

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