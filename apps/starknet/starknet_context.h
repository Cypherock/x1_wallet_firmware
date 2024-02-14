/**
 * @file    starknet_context.h
 * @author  Cypherock X1 Team
 * @brief   Header file defining typedefs and MACROS for the STARKNET app
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef STARKNET_CONTEXT_H
#define STARKNET_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
// STARKNET implicit account derivation path: m/44'/397'/0'/0'/i'

// Rationale to keep it m/44'/397'/0'/0'/i':
// STARKNET implicit accounts can be any public/private keypair
// https://docs.starknet.org/integrator/implicit-accounts

// STARKNET official CLI uses m/44'/397'/0'/0'/1' as the first account
// https://docs.starknet.org/tools/starknet-cli

// Ledger uses m/44'/397'/0'/0'/0'
// https://github.com/starknet/starknet-wallet/issues/1537

// There are some wallets which even use m/44'/9000'/0'/0/0
// https://support.atomicwallet.io/article/146-list-of-derivation-paths

#define STARKNET_IMPLICIT_ACCOUNT_DEPTH 5

#define STARKNET_PURPOSE_INDEX (0x8000002C)    // 44'
#define STARKNET_COIN_INDEX (0x8000232C)       // 9004'
#define STARKNET_ACCOUNT_INDEX 0x80000000      // 0'
#define STARKNET_CHANGE_INDEX 0x0000000        // 0

/// this makes length of 5 with a termination NULL byte
#define STARKNET_SHORT_NAME_MAX_SIZE 6
/// this makes length of 5 with a termination NULL byte
#define STARKNET_LONG_NAME_MAX_SIZE 9

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  /** Largest unit/denomination indicator/symbol. This will be used in UX for
   * displaying fees and amount.
   */
  const char lunit_name[STARKNET_SHORT_NAME_MAX_SIZE];
  /** Common name of the blockchain known to the users. This will be used in UX
   */
  const char name[STARKNET_LONG_NAME_MAX_SIZE];
} starknet_config_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/
extern const starknet_config_t starknet_app;

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif /* STARKNET_CONTEXT_H */
