/**
 * @file    tron_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the TRON app
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef TRON_CONTEXT_H
#define TRON_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <tron/tron.pb.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

// TODO: Populate structure for TRON
typedef struct {
} tron_config_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Function for handling the parameter field of tron txns
 *
 * @param[in] raw_txn: tron raw transaction
 * @return bool True if decoding was successful, else false
 */
bool extract_contract_info(tron_transaction_raw_t *raw_txn,
                           bool use_signature_verification);

/**
 * @brief Checks that the transaction spends from the signing key's own account
 * @details The owner address and the permission id are signed but not shown:
 * a host could otherwise make the device sign for another account that
 * granted this key a permission. The contract owner must be the address of
 * the given key and the permission must be the default (0).
 *
 * @param[in] raw_txn decoded tron raw transaction
 * @param[in] public_key 65-byte uncompressed public key of the signing key
 * @return bool true if the owner is the key's address and permission_id is 0
 */
bool tron_is_own_account_txn(const tron_transaction_raw_t *raw_txn,
                             const uint8_t public_key[65]);

#endif /* TRON_CONTEXT_H */
