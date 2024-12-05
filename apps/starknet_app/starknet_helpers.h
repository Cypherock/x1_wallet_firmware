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

#include "bignum.h"
#include "coin_utils.h"
#include "f251.h"
#include "ecdsa.h"
#include "starknet_crypto.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define LOW_PART_BITS 248
#define LOW_PART_BYTES (LOW_PART_BITS / 8)
#define LOW_PART_MASK ((1ULL << LOW_PART_BITS) - 1)
#define STARKNET_BIGNUM_SIZE 32
#define PEDERSEN_HASH_SIZE 32
#define CALL_DATA_PARAMETER_SIZE 3
#define STARKNET_SIZE_PUB_KEY (32)
#define STARKNET_ADDR_SIZE 32
#define STARKNET_ARGENT_CLASS_HASH                                             \
  "036078334509b514626504edc9fb252328d1a240e4e948bef8d0c08dff45927f"
#define STARKNET_DEPLOYER_VALUE 0

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
// bool starknet_derive_bip32_node(const uint8_t *seed, uint8_t *private_key);

/**
 *
 */
bool starknet_derive_key_from_seed(const uint8_t *seed,
                                   const uint32_t *path,
                                   uint32_t path_length,
                                   uint8_t *private_key,
                                   uint8_t *public_key);

bool pederson_hash(uint8_t *x, uint8_t *y, uint8_t len, uint8_t *hash);
bool get_stark_child_node(const uint32_t *path,
                          const size_t path_length,
                          const char *curve,
                          const uint8_t *seed,
                          const uint8_t seed_len,
                          HDNode *hdnode);
void compute_hash_on_elements(uint8_t data[][STARKNET_BIGNUM_SIZE],
                              uint8_t num_elem,
                              uint8_t *hash);
void starknet_uli_to_bn_byte_array(const unsigned long int ui,
                                   uint8_t *bn_array);
void poseidon_hash_many(felt_t state[], uint8_t state_size, felt_t res);


void mpz_to_bn(bignum256 *bn, const mpz_t mpz);

int starknet_sign_digest(const stark_curve *curve,
                         const uint8_t *priv_key,
                         const uint8_t *digest,
                         uint8_t *sig,
                         uint8_t *pby,
                         int (*is_canonical)(uint8_t by, uint8_t sig[64]));
#endif    // STARKNET_HELPERS_H