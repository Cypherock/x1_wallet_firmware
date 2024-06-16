/**
 * @file    tron_contracts.h
 * @author  Cypherock X1 Team
 * @brief   TRON contract related definitions and types
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef TRON_CONTRACTS_H
#define TRON_CONTRACTS_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <tron_txn_helpers.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

// Number of entries in whitelisted contracts list
#define TRC20_WHITELISTED_CONTRACTS_COUNT 9
#define TRC20_FUNCTION_SELECTOR_COUNT 1

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct trc20_contracts {
  /// 21-byte hex coded public address of the contract
  const uint8_t address[TRON_INITIAL_ADDRESS_LENGTH];
  /// Symbol (short alphabetical representation) of the contract token
  const char *symbol;
  /// Decimal value used to display the amount in token transfer in token units
  const uint8_t decimal;
} trc20_contracts_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif    // TRON_CONTRACTS_H
