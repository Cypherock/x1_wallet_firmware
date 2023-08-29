/**
 * @file    near_txn_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the NEAR app for txn signing flow
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef NEAR_TXN_HELPERS_H
#define NEAR_TXN_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "near_context.h"

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
 * @brief Parse byte array of unsigned txn and store decoded information to be
 * used for user confirmation.
 * @details Only a limited number of action types are supported by this decoder.
 *
 * @param byte_array Constant reference to buffer containing the raw unsigned
 * txn
 * @param byte_array_size Size in bytes of the txn
 * @param utxn Reference to buffer where decoded information will be populated.
 * It can be used at a later stage for user verification.
 * @return true If the parsing was successful
 * @return false If the parsing failed - it could be due to an unsupported
 * action type or missing information
 */
bool near_parse_transaction(const uint8_t *byte_array,
                            uint16_t byte_array_size,
                            near_unsigned_txn *utxn);

#endif /* NEAR_TXN_HELPERS_H */
