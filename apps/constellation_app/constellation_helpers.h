/**
 * @file    constellation_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for CONSTELLATION chains
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CONSTELLATION_HELPERS_H
#define CONSTELLATION_HELPERS_H

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
 * derivations: address: m/44'/144'/0'/0/i
 *
 * @param[in] path      The derivation path as an uint32 array
 * @param[in] levels     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided derivation path is valid
 * @retval true if the derivation path is valid
 * @retval false otherwise
 */
bool constellation_derivation_path_guard(const uint32_t *path, uint8_t levels);

#endif    // CONSTELLATION_HELPERS_H