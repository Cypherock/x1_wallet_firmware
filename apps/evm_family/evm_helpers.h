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

#include "coin_utils.h"
#include "evm_priv.h"
/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define EVM_DRV_LEGACY_DEPTH 4
#define EVM_DRV_BIP44_DEPTH 5
#define EVM_DRV_ACCOUNT_DEPTH 5

#define EVM_PUB_KEY_SIZE 65
#define EVM_SHORT_PUB_KEY_SIZE 33

#define EVM_DRV_ACCOUNT 0x80000000

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
bool evm_derivation_path_guard(const uint32_t *path, uint32_t depth);

/**
 * @brief This function calculates the hash of the message data based on the
 * message type.
 *
 * @param ctx A pointer to a structure of type @ref evm_sign_msg_context_t,
 * which contains information about the signing context.
 * @param digest The `digest` parameter is a pointer to a `uint8_t` array where
 * the computed message data hash will be stored.
 *
 * @return a boolean value indicating if digest of the msg data was computed
 * correctly or not
 */
bool evm_get_msg_data_digest(const evm_sign_msg_context_t *ctx,
                             uint8_t *digest);
#endif /* EVM_HELPERS_H */
