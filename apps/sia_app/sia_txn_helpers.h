/**
 * @file    sia_txn_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the Sia app for txn signing flow
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef SIA_TXN_HELPERS_H
#define SIA_TXN_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "sia_context.h"

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
 * @brief Convert lo/hi hastings values to SC for display
 * @param lo Lower 64 bits of hastings amount
 * @param hi Upper 64 bits of hastings amount
 * @return double Amount in SC units
 */
double sia_convert_to_sc(uint64_t lo, uint64_t hi);

/**
 * @brief Parse Sia transaction blob into structured format
 * @param blob Raw transaction data from host
 * @param blob_len Length of transaction data
 * @param txn Output structure for parsed transaction
 * @return bool true on success, false on parsing error
 *
 * See https://github.com/SiaFoundation/core/blob/master/types/encoding.go#L814
 * See
 * https://github.com/Keyur279/Sia_Cli_tool/blob/main/README.md#transaction-blob-format
 */
bool sia_parse_transaction(const uint8_t *blob,
                           size_t blob_len,
                           sia_transaction_t *txn);

#endif /* SIA_TXN_HELPERS_H */
