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

#endif /* TRON_CONTEXT_H */
