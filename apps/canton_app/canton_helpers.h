/**
 * @file    canton_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for CANTON chains
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CANTON_HELPERS_H
#define CANTON_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
 * derivations: address: m/44'/6767'/0'/0/i
 *
 * @param[in] path      The derivation path as an uint32 array
 * @param[in] levels     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided derivation path is valid
 * @retval true if the derivation path is valid
 * @retval false otherwise
 */
bool canton_derivation_path_guard(const uint32_t *path, uint8_t levels);

/**
 * @brief Hashes the data using SHA-256 and prepends the canton hash prefix.
 * @details The function will hash the data using SHA-256 and prepend the canton
 * hash prefix.
 *
 * @param[in] data The data to hash
 * @param[in] data_size The size of the data
 * @param[out] hash The hash of the data. Must be at least CANTON_HASH_SIZE
 * bytes.
 */
void sha256_with_prefix(const uint8_t *data, size_t data_size, uint8_t *hash);

/**
 * @brief Generates the party id string from the public key.
 *
 * @param[in] public_key The public key
 * @param[out] party_id_str The party id string
 * @return bool Indicates if the party id string was generated successfully
 * @retval true if the party id string was generated successfully
 * @retval false otherwise
 */
bool get_party_id(const uint8_t *public_key, char *party_id);

/**
 * @brief Verifies the party id derived from the public key with the given party
 * id.
 * @details The function will derive the party id from the public key and
 * compare it with the given party id.
 *
 * @param[in] public_key The public key
 * @param[in] party_id The party id to compare with
 * @return bool Indicates if the party id is valid
 * @retval true if the party id is valid
 * @retval false otherwise
 */
bool verify_party_id(uint8_t *public_key, char *party_id);
#endif    // CANTON_HELPERS_H