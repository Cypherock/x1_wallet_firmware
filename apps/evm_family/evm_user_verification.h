/**
 * @file    evm_user_verification.c
 * @author  Cypherock X1 Team
 * @brief   Apis for user verification of all different transaction type info
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef EVM_USER_VERIFICATION_H
#define EVM_USER_VERIFICATION_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "evm_txn_helpers.h"

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
 * @brief User verification for eth and whitelisted token transfer
 *
 * @param txn_context Const reference to an instance of evm_txn_context_t
 * @return bool Indicating if the user confirmed all values
 * @retval true If all the values and prompts were confirmed by the user
 * @retval false If any of the prompt was rejected by the user
 */
bool evm_verify_transfer(const evm_txn_context_t *txn_context);

/**
 * @brief User verification for non-whitelisted token transfer and clear signing
 *
 * @param txn_context Const reference to an instance of evm_txn_context_t
 * @return bool Indicating if the user confirmed all values
 * @retval true If all the values and prompts were confirmed by the user
 * @retval false If any of the prompt was rejected by the user
 */
bool evm_verify_clear_signing(const evm_txn_context_t *txn_context);

/**
 * @brief User verification for blind signing of unknown contract methods
 *
 * @param txn_context Const reference to an instance of evm_txn_context_t
 * @return bool Indicating if the user confirmed all values
 * @retval true If all the values and prompts were confirmed by the user
 * @retval false If any of the prompt was rejected by the user
 */
bool evm_verify_blind_signing(const evm_txn_context_t *txn_context);

#endif
