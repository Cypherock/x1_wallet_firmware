/**
 * @file    solana_contracts.h
 * @author  Cypherock X1 Team
 * @brief   TRON contract related definitions and types
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef SOLANA_CONTRACTS_H
#define SOLANA_CONTRACTS_H
/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "solana_txn_helpers.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

// Number of entries in whitelisted contracts list
#define SOLANA_WHITELISTED_TOKEN_PROGRAM_COUNT 1

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct solana_token_program {
  /// 32-byte hex coded public address of the program
  const uint8_t address[SOLANA_ACCOUNT_ADDRESS_LENGTH];
  /// Symbol (short alphabetical representation) of the contract token
  const char *symbol;
  /// Decimal value used to display the amount in token transfer in token units
  const uint8_t decimal;
} solana_token_program_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif    // SOLANA_CONTRACTS_H
