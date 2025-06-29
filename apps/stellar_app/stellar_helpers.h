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
 *
 * @param[in] path      The derivation path as an uint32 array
 * @param[in] levels     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided derivation path is valid
 * @retval true if the derivation path is valid
 * @retval false otherwise
 */
bool stellar_derivation_path_guard(const uint32_t *path, uint8_t levels);

#endif    // STELLAR_HELPERS_H