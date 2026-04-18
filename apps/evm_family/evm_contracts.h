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

/** ERC-20 approve(address spender, uint256 amount)
 *  Refer https://www.4byte.directory/signatures/?bytes4_signature=0x095ea7b3 */
#define EVM_approve_TAG (0x095ea7b3)
#define EVM_approve_NUM_ARGS 2

/** HYSP depositInstant(address tokenIn, uint256 amount, uint256
 * minReceiveAmount, bytes32 referrerId) Refer
 * https://www.4byte.directory/signatures/?bytes4_signature=0xc02dd27a */
#define EVM_hysp_depositInstant_TAG (0xc02dd27a)
#define EVM_hysp_depositInstant_NUM_ARGS 4

/** HYSP redeemInstant(address tokenOut, uint256 amount, uint256
 * minReceiveAmount) Refer
 * https://www.4byte.directory/signatures/?bytes4_signature=0x8b53f75e */
#define EVM_hysp_redeemInstant_TAG (0x8b53f75e)
#define EVM_hysp_redeemInstant_NUM_ARGS 3

/** HYSP redeemRequest(address tokenOut, uint256 amount)
 *  Refer https://www.4byte.directory/signatures/?bytes4_signature=0xbfc2d46a */
#define EVM_hysp_redeemRequest_TAG (0xbfc2d46a)
#define EVM_hysp_redeemRequest_NUM_ARGS 2

/**
 * @brief An expected limit on length of Ethereum based ERC20 token symbols.
 * @details The token symbol is also exchanged in txn_metadata.token_name.
 *
 * @see <a href="https://ethereum.stackexchange.com/a/117645"
 * target="_blank">StackExchange question</a>
 */
#define ETHEREUM_TOKEN_SYMBOL_LENGTH 20
#define HYSP_MEVUSD_DECIMALS (18)

#define ETH_UTXN_ABI_DECODE_OK (0xAA)
#define ETH_UTXN_BAD_PAYLOAD (0x11)
/* Changed ETH_UTXN_FUNCTION_NOT_FOUND from 0x11 to 0x12: With semantic parsers,
 * routing must distinguish "unknown function" (allow blind signing) from
 * "malformed known function" (reject). Both being 0x11 caused broken HYSP txns
 * to be blind-signable. */
#define ETH_UTXN_FUNCTION_NOT_FOUND (0x12)
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
 * @brief Extracts ABI-encoded arguments for EVM functions into UI display
 * nodes.
 * @details Checks the semantic parser registry first (for HYSP and future
 *          protocols). If a semantic parser is registered for the selector,
 *          it is called to produce token-aware display (symbols, decimals).
 *          Otherwise falls back to the generic ABI parser.
 *
 * @param pAbiPayload   Pointer to start of payload (includes 4-byte selector).
 * @param sizeOfPayload Size of the payload in bytes.
 * @param toAddress     Transaction to_address (20 bytes). Required for
 * approve() which is called on the token contract.
 * @param displayNode   Output: pointer to storage for ui_display_node list.
 * @return uint8_t Depicts the status of operation for this function
 * @retval ETH_BAD_ARGUMENTS: If any argument is invalid
 * @retval ETH_UTXN_FUNCTION_NOT_FOUND: Selector not found in any parser
 * @retval ETH_UTXN_BAD_PAYLOAD: Payload contains invalid data
 * @retval ETH_UTXN_ABI_DECODE_OK: Arguments extracted successfully
 */
uint8_t ETH_ExtractArguments(const uint8_t *pAbiPayload,
                             const uint64_t sizeOfPayload,
                             const uint8_t *toAddress,
                             ui_display_node **displayNode);

/**
 * @brief Return a human-readable name for a known HYSP contract address.
 * @details Returns the token symbol for token contracts (USDC, USDT, mevUSD)
 *          and a descriptive name for vault contracts (Midas Issuance Vault,
 *          Midas Redemption Vault). Returns NULL for any unknown address.
 *          Used by evm_verify_clear_signing to replace the generic
 *          "Unverified contract" text when the to_address is a known HYSP
 *          contract — without suppressing the address display itself.
 *
 * @param addr20  20-byte contract address (raw bytes, not a 32-byte ABI slot).
 * @return Pointer to a static string, or NULL if address is not HYSP-known.
 */
const char *HYSP_FindContractName(const uint8_t *addr20);

#endif    // EVM_CONTRACTS_H
