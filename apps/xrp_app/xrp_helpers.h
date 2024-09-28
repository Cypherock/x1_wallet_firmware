/**
 * @file    xrp_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for XRP chains
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef XRP_HELPERS_H
#define XRP_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "coin_utils.h"
#include "hasher.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define XRP_IMPLICIT_ACCOUNT_DEPTH 5

#define XRP_PURPOSE_INDEX 0x8000002C    // 44'
#define XRP_COIN_INDEX 0x80000090       // 144'
#define XRP_ACCOUNT_INDEX 0x80000000    // 0'
#define XRP_CHANGE_INDEX 0x00000000     // 0

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
 * @details The derivation depth is fixed at level 5. So if the depth level != 5, then
 * this function return false indicating invalid derivation path. 
 * The function supports checking derivation paths for HD wallets
 * Types of derivations:
 * address: m/44'/144'/0'/0/i
 *
 * @param[in] path      The derivation path as an uint32 array
 * @param[in] levels     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided derivation path is valid
 * @retval true if the derivation path is valid
 * @retval false otherwise
 */
bool xrp_derivation_path_guard(const uint32_t *path, uint8_t levels);

/**
 * @brief Generates the base58 encoded string from the data.
 * @details The given data is hashed as per the given hasher_type.
 * Then the checksum (first 4 bytes of hash digest) is taken and concatenated
 * with the data. The concatenated data is then encoded using base58
 * with the dictionary rpshnaf39wBUDNEGHJKLM4PQRST7VWXYZ2bcdeCg65jkm8oFqi1tuvAxyz
 *
 * @param[in] data        The data to encode
 * @param[in] data_len    The length of the given data
 * @param[in] hasher_type The type of hash to generate checksum
 * @param[out] str        The string to store the encoded data in
 * @param[in] strsize     The size of str to generate
 * @return int            The size of str generated
 * @retval                Same as provided if successful
 * @retval                0 otherwise
 */
int xrp_base58_encode_check(const uint8_t *data, int datalen,
                        HasherType hasher_type, char *str, int strsize);

#endif    // XRP_HELPERS_H