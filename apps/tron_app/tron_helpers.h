/**
 * @file    tron_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for TRON chains
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef TRON_HELPERS_H
#define TRON_HELPERS_H

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

#define TRON_PURPOSE_INDEX 0x8000002C
#define TRON_COIN_INDEX 0x800000C3
#define TRON_ACCOUNT_INDEX 0x80000000
#define TRON_CHANGE_INDEX 0x00000000

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
 * address: m/44'/195'/0'/0/i
 * address: m/44'/195'/0'
 *
 * @param[in] path      The derivation path as an uint32 array
 * @param[in] depth     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided derivation path is valid
 * @retval true if the derivation path is valid
 * @retval false otherwise
 */
bool tron_derivation_path_guard(const uint32_t *path, uint8_t levels);

#endif    // TRON_HELPERS_H