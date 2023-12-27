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

#include "coin_utils.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/// EVM function parameter block-size length
#define EVM_FUNC_PARAM_BLOCK_LENGTH 32

/// EVM function parameter block-size length
#define EVM_FUNC_SIGNATURE_LENGTH 4

/// Length of Ethereum public addresses in bytes
#define EVM_ADDRESS_LENGTH 20

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0x7c025200 */
#define EVM_swap_TAG (0x12aa3caf)
#define EVM_swap_NUM_ARGS 10

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0xe449022e */
#define EVM_uniswapV3Swap_TAG (0xe449022e)
#define EVM_uniswapV3Swap_NUM_ARGS 3

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0x42842e0e */
#define EVM_safeTransferFrom_TAG (0x42842e0e)
#define EVM_safeTransferFrom_NUM_ARGS 3

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0xd0e30db0 */
#define EVM_deposit_TAG (0xd0e30db0)
#define EVM_deposit_NUM_ARGS 0

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0xa9059cbb */
#define EVM_transfer_TAG (0xa9059cbb)
#define EVM_transfer_NUM_ARGS 2

/**
 * @brief An expected limit on length of Ethereum based ERC20 token symbols.
 * @details The token symbol is also exchanged in txn_metadata.token_name.
 *
 * @see <a href="https://ethereum.stackexchange.com/a/117645"
 * target="_blank">StackExchange question</a>
 */
#define ETHEREUM_TOKEN_SYMBOL_LENGTH 20

#define ETH_UTXN_ABI_DECODE_OK (0xAA)
#define ETH_UTXN_BAD_PAYLOAD (0x11)
#define ETH_UTXN_FUNCTION_NOT_FOUND (0x11)
#define ETH_BAD_ARGUMENTS (0x22)

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
  const uint8_t address[EVM_ADDRESS_LENGTH];
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

/**
 * @brief This function extracts Abi encoded arguments for EVM functions into UI
 * compatible nodes ui_display_node(s)
 *
 * @param pAbiPayload Pointer to start of payload of the EVM transaction
 * @param sizeOfUTxn Size of payload of the EVM transaction
 * @param displayNode Pointer to storage for ui_display_node
 * @return uint8_t Depicts the status of operation for this function
 * @retval ETH_BAD_ARGUMENTS: If any argument is invalid
 * @retval ETH_UTXN_FUNCTION_NOT_FOUND: If a function NOT supported by X1 wallet
 * is in the EVM tx
 * @retval ETH_UTXN_BAD_PAYLOAD: If a payload contains invalid data
 * @retval ETH_UTXN_ABI_DECODE_OK: If the arguments are extracted successfully
 */
uint8_t ETH_ExtractArguments(const uint8_t *pAbiPayload,
                             const uint64_t sizeOfPayload,
                             ui_display_node **displayNode);

#endif    // EVM_CONTRACTS_H
