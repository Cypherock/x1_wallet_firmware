/**
 * @file    tron_txn_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Helper apis for interpreting and signing tron transactions
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef TRON_TXN_HELPERS_H
#define TRON_TXN_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include <tron/tron.pb.h>

#include "tron_priv.h"
#include "tron_txn_helpers.h"
#include "utils.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define TRON_NAME "Tron"
#define TRON_LUNIT "TRX"
#define TRON_PUB_KEY_SIZE 65
#define TRON_INITIAL_ADDRESS_LENGTH 21
#define TRON_ACCOUNT_ADDRESS_LENGTH 34

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
 * @brief Convert byte array representation of unsigned transaction to
 * tron_unsigned_txn.
 * @details
 *
 * @param [in] byte_array                   Byte array of unsigned transaction.
 * @param [in] byte_array_size              Size of byte array.
 * @param [out] utxn                        Pointer to the tron_unsigned_txn
 * instance to store the transaction details.
 *
 * @return Status of conversion
 * @retval 0 if successful
 * @retval -1 if unsuccessful
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int tron_byte_array_to_raw_txn(uint8_t *byte_array,
                               size_t size,
                               tron_transaction_raw_t *raw_txn);

/**
 * @brief Validate the deserialized unsigned transaction
 *
 * @param utxn Pointer to the tron_unsigned_txn instance to validate the
 * transaction
 * @return 0 if validation succeeded
 * @return -1 if validation failed
 */
int tron_validate_unsigned_txn(const tron_transaction_raw_t *raw_txn);

#endif
