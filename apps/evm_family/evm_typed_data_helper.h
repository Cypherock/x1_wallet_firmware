/**
 * @file    evm_typed_data_helper.h
 * @author  Cypherock X1 Team
 * @brief   Implements the typed data helper functions.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef EVM_TYPED_DATA_HELPER_H
#define EVM_TYPED_DATA_HELPER_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "evm_priv.h"

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
 * @brief The function initializes a display node for a typed data structure and
 * creates display nodes for the domain and message components of the structure.
 *
 * @param node A pointer to a pointer of a `ui_display_node` object. This is
 * used to store the created display nodes.
 * @param typed_data The parameter `typed_data` is a pointer to a structure of
 * type `evm_sign_typed_data_struct_t`.
 *
 * @return The function does not explicitly return a value.
 */
void evm_init_typed_data_display_node(ui_display_node **node,
                                      evm_sign_typed_data_struct_t *typed_data);

/**
 * @brief The function calculates the digest of a typed data structure using the
 * EIP712 hashing algorithm.
 *
 * @param typed_data A pointer to a structure of type
 * `evm_sign_typed_data_struct_t` which contains the typed data to be hashed.
 * @param digest_out A pointer to a buffer where the computed digest will be
 * stored. The buffer should have enough space to store the digest, which is
 * typically 32 bytes.
 *
 * @return a boolean value.
 */
bool evm_get_typed_struct_data_digest(
    const evm_sign_typed_data_struct_t *typed_data,
    uint8_t *digest_out);
#endif
