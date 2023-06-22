/**
 * @file    btc_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for Bitcoin chain
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BTC_HELPERS_H
#define BTC_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define PURPOSE_LEGACY 0x8000002C     // 44'
#define PURPOSE_SEGWIT 0x80000031     // 49'
#define PURPOSE_NSEGWIT 0x80000054    // 84'
#define PURPOSE_TAPROOT 0x80000056    // 86'

#define COIN_BTC 0x80000000            // 0'
#define COIN_BTC_TESTNET 0x80000001    // 1'

#define BTC_ACC_XPUB_DEPTH 3
#define BTC_ACC_ADDR_DEPTH 5

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
 * @brief Verifies the derivation path for any inconsistent/unsupported values.
 * @details The function supports checking for multiple derivation paths. Also,
 * based on the provided depth value, the function can act as xpub or address
 * derivation path.
 * The only allowed depth count is either 3 (account node level) for xpub or 5
 * (address node level) for public address derivation.
 * Currently, following purpose indices are allowed: 0x8000002C, 0x80000031,
 * 0x80000054, 0x80000056.
 * The only supported coin indices are: 0x80000000 & 0x80000001. The function
 * accepts 0 & 1 for change indices. For the rest of the levels, only correct
 * hardness is checked.
 *
 * @param[in] path      The derivation path to be checked
 * @param[in] depth     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided path indices are valid
 * @retval true if the path values are valid
 * @retval false otherwise
 */
bool btc_derivation_path_guard(const uint32_t *path, uint32_t depth);

#endif
