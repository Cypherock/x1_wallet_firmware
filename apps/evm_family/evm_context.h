/**
 * @file    evm_context.h
 * @author  Cypherock X1 Team
 * @brief   Ethereum config for various network chains
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef EVM_CONTEXT_H
#define EVM_CONTEXT_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "evm_contracts.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/// this makes length of 7 with a termination NULL byte
#define EVM_SHORT_NAME_MAX_SIZE 8
/// this makes length of 19 with a termination NULL byte
#define EVM_LONG_NAME_MAX_SIZE 20

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef struct {
  /** Largest unit/denomination indicator/symbol. This will be used in UX for
   * displaying fees and amount.
   */
  const char lunit_name[EVM_SHORT_NAME_MAX_SIZE];
  /** Common name of the blockchain known to the users. This will be used in UX
   */
  const char name[EVM_LONG_NAME_MAX_SIZE];

  /** Refer:
   * https://github.com/ethereum/EIPs/blob/830708a049fc982fd595cb0c4dca703aebefd003/EIPS/eip-2294.md
   */
  const uint64_t chain_id;
  bool (*is_token_whitelisted)(const uint8_t *address,
                               const erc20_contracts_t **contract);
} evm_config_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif
