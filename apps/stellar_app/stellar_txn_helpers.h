/**
 * @file    stellar_txn_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the Stellar app for txn signing flow
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef STELLAR_TXN_HELPERS_H
#define STELLAR_TXN_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "stellar_context.h"

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
 * @brief Parse XDR byte array of unsigned transaction and store decoded
 * information to be used for user confirmation.
 * @details Supports Stellar payment and create account transactions.
 *
 * @param xdr Constant reference to buffer containing the raw unsigned XDR
 * transaction
 * @param xdr_len Size in bytes of the XDR transaction
 * @param tx Reference to buffer where decoded transaction information will be
 * populated. It can be used at a later stage for user verification.
 * @param tagged_txn_len Pointer to store the length of the tagged txn xdr
 * used for signing
 * @return int 0 if the parsing was successful, negative value if parsing failed
 * @retval 0 If the parsing was successful
 * @retval -1 If the parsing failed - could be due to unsupported transaction,
 * data type, or missing information
 */
int stellar_parse_transaction(const uint8_t *xdr,
                              uint32_t xdr_len,
                              stellar_transaction_t *txn,
                              uint32_t *tagged_txn_len);

#endif /* STELLAR_TXN_HELPERS_H */
