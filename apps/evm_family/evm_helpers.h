/**
 * @file    evm_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for EVM chains
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef EVM_HELPERS_H
#define EVM_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define EVM_DRV_LEGACY_DEPTH 4
#define EVM_DRV_OTHER_DEPTH 5

#define EVM_PUB_KEY_SIZE 65
#define EVM_SHORT_PUB_KEY_SIZE 33

#define EVM_CHAIN_NAME "ETH"

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
 * @details The function supports checking dephts for derivation paths. The
 * only allowed depth count is either 4 (legacy) for 5 (metamask and ledger
 * live) for public address derivation.
 *
 * @param[in] depth     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided path depth is valid
 * @retval true if the path depth is valid
 * @retval false otherwise
 */
bool evm_derivation_path_guard(uint32_t depth);

#endif /* EVM_HELPERS_H */
