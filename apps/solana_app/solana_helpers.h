/**
 * @file    solana_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for Solana chains
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef SOLANA_HELPERS_H
#define SOLANA_HELPERS_H

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

#define SOLANA_PUB_KEY_SIZE 33
#define SOLANA_PURPOSE_INDEX 0x8000002C
#define SOLANA_COIN_INDEX 0x800001F5
#define SOLANA_ACCOUNT_INDEX 0x80000000
#define SOLANA_CHANGE_INDEX 0x80000000

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
 * legacy        : m/44'/60'/0'/x
 * bip44         : m/44'/60'/0'/0/x
 * account model : m/44'/60'/x'/0/0
 *
 * @param[in] path      The derivation path as an uint32 array
 * @param[in] depth     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided derivation path is valid
 * @retval true if the derivation path is valid
 * @retval false otherwise
 */
bool solana_derivation_path_guard(const uint32_t *path, uint8_t levels);

#endif    // SOLANA_HELPERS_H