/**
 * @file    evm_contracts.h
 * @author  Cypherock X1 Team
 * @brief   EVM contract related definitions and types
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef EVM_CONTRACTS_H
#define EVM_CONTRACTS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/// EVM function parameter block-size length
#define EVM_FUNC_PARAM_BLOCK_LENGTH 32

/// EVM function parameter block-size length
#define EVM_FUNC_SIGNATURE_LENGTH 4

/// EVM transfer method signature
#define TRANSFER_FUNC_SIGNATURE 0xa9059cbb

/// Length of Ethereum public addresses in bytes
#define ETHEREUM_ADDRESS_LENGTH 20

/**
 * @brief An expected limit on length of Ethereum based ERC20 token symbols.
 * @details The token symbol is also exchanged in txn_metadata.token_name.
 *
 * @see <a href="https://ethereum.stackexchange.com/a/117645"
 * target="_blank">StackExchange question</a>
 */
#define ETHEREUM_TOKEN_SYMBOL_LENGTH 20

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/**
 * @brief Class to maintain a mapping of contract address and its token symbol
 * @details
 *
 * @see
 * @since
 * @deprecated
 *
 * @note
 * @todo
 */
typedef struct erc20_contracts {
  /// 20-byte hex coded public address of the contract
  const uint8_t address[ETHEREUM_ADDRESS_LENGTH];
  /// Symbol (short alphabetical representation) of the contract token
  const char *symbol;
  /// Decimal value used to display the amount in token transfer in token units
  const uint8_t decimal;
} erc20_contracts_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

#endif    // EVM_CONTRACTS_H
