/**
 * @file    starknet_crypto.h
 * @author  Cypherock X1 Team
 * @brief   Crypto api definitions for Starknet chain
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef STARKNET_CRYPTO_H
#define STARKNET_CRYPTO_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "mpz_ecdsa.h"
/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/
extern mpz_curve *stark_curve;
extern mpz_pedersen *starknet_pedersen_points;

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Initialize STARKNET context.
 * @details Sets up resources and state for STARKNET operations.
 */
void starknet_init();

/**
 * @brief Clears starknet resources.
 * @details Releases all resources related to starkent hash and curve
 * operations.
 */
void stark_clear();

#endif    // STARKNET_CRYPTO_H