/**
 * @file    xrp_txn_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the XRP app for txn signing flow
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef XRP_TXN_HELPERS_H
#define XRP_TXN_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "xrp_context.h"

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
 * @details Only xrp payment transaction is supported as of now.
 *
 * @param byte_array Constant reference to buffer containing the raw unsigned
 * txn
 * @param byte_array_size Size in bytes of the txn
 * @param utxn Reference to buffer where decoded information will be populated.
 * It can be used at a later stage for user verification.
 * @return true If the parsing was successful
 * @return false If the parsing failed - it could be due to an unsupported
 * transaction or data type or missing information
 */
bool xrp_parse_transaction(const uint8_t *byte_array,
                           uint16_t byte_array_size,
                           xrp_unsigned_txn *utxn);

/**
 * @brief Parse transaction flags into human-readable strings
 * @details Converts raw flag values into descriptive text for user display
 *
 * @param flags Raw flag value from transaction
 * @param txn_type Transaction type (Payment or TrustSet)
 * @param flag_buffer Buffer to store parsed flag strings
 * @param buffer_size Size of the flag_buffer
 */
void parse_transaction_flags(uint32_t flags,
                             uint16_t txn_type,
                             char *flag_buffer,
                             size_t buffer_size);

#endif /* XRP_TXN_HELPERS_H */
