/**
 * @file    near_txn_user_verification.h
 * @author  Cypherock X1 Team
 * @brief   Header file exporting function for user verification before signing
 *          txns.
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef NEAR_TXN_USER_VERIFICATION_H
#define NEAR_TXN_USER_VERIFICATION_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "near_priv.h"

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
 * @brief Performs user verification flow for the NEAR transfer action
 * @note If the user rejects at any step, a rejection message is sent to the
 * USB host
 *
 * @param decoded_utxn
 * @return true If the user accepted the transaction and is safe to proceed with
 * signing
 * @return false If the user rejected any step or a P0 event occurred
 */
bool user_verification_transfer(const near_unsigned_txn *decoded_utxn);

/**
 * @brief Performs user verification flow for the NEAR function action
 * @note If the user rejects at any step, a rejection message is sent to the
 * USB host
 *
 * @param decoded_utxn
 * @return true If the user accepted the transaction and is safe to proceed with
 * signing
 * @return false If the user rejected any step or a P0 event occurred
 */
bool user_verification_function(const near_unsigned_txn *decoded_utxn);

#endif /* NEAR_TXN_USER_VERIFICATION_H */
