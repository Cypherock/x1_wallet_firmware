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

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0x3a29dbae */
#define EVM_stake_TAG (0x3a29dbae)
#define EVM_stake_NUM_ARGS 1

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0x76ec871c */
#define EVM_unstake_TAG (0x76ec871c)
#define EVM_unstake_NUM_ARGS 3

/** Pool for stake and unstake:
 * https://etherscan.io/address/0xD523794C879D9eC028960a231F866758e405bE34 */
#define EVM_everstake_POOL_ADDRESS                                             \
  ((const uint8_t[EVM_ADDRESS_LENGTH]){                                        \
      0xd5, 0x23, 0x79, 0x4c, 0x87, 0x9d, 0x9e, 0xc0, 0x28, 0x96,              \
      0x0a, 0x23, 0x1f, 0x86, 0x67, 0x58, 0xe4, 0x05, 0xbe, 0x34})

/**  Refer https://www.4byte.directory/signatures/?bytes4_signature=0x33986ffa
 */
#define EVM_claimWithdrawRequest_TAG (0x33986ffa)
#define EVM_claimWithdrawRequest_NUM_ARGS 0

/** Withdraw treasury:
 * https://etherscan.io/address/0x7a7f0b3c23C23a31cFcb0c44709be70d4D545c6e   */
#define EVM_everstake_TREASURY_ADDRESS                                         \
  ((const uint8_t[EVM_ADDRESS_LENGTH]){                                        \
      0x7a, 0x7f, 0x0b, 0x3c, 0x23, 0xc2, 0x3a, 0x31, 0xcf, 0xcb,              \
      0x0c, 0x44, 0x70, 0x9b, 0xe7, 0x0d, 0x4d, 0x54, 0x5c, 0x6e})

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0x095ea7b3 */
#define EVM_approve_TAG (0x095ea7b3)
#define EVM_approve_NUM_ARGS 2

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0xe4457a8a */
#define EVM_buyVoucherPOL_TAG (0xe4457a8a)
#define EVM_buyVoucherPOL_NUM_ARGS 2

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0xe570b78b */
#define EVM_sellVoucher_newPOL_TAG (0xe570b78b)
#define EVM_sellVoucher_newPOL_NUM_ARGS 2

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0x8759c234 */
#define EVM_unstakeClaimTokens_newPOL_TAG (0x8759c234)
#define EVM_unstakeClaimTokens_newPOL_NUM_ARGS 1

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0xe0db556b */
#define EVM_withdrawRewardsPOL_TAG (0xe0db556b)
#define EVM_withdrawRewardsPOL_NUM_ARGS 0

/** Refer https://www.4byte.directory/signatures/?bytes4_signature=0x9ad5f5a4 */
#define EVM_restakePOL_TAG (0x9ad5f5a4)
#define EVM_restakePOL_NUM_ARGS 0

/** POL token (approve):
 * https://etherscan.io/address/0x455e53CBB86018Ac2B8092FdCd39d8444aFFC3F6 */
#define EVM_pol_TOKEN_ADDRESS                                                  \
  ((const uint8_t[EVM_ADDRESS_LENGTH]){                                        \
      0x45, 0x5e, 0x53, 0xcb, 0xb8, 0x60, 0x18, 0xac, 0x2b, 0x80,              \
      0x92, 0xfd, 0xcd, 0x39, 0xd8, 0x44, 0x4a, 0xff, 0xc3, 0xf6})

/** Polygon StakeManager, the only spender allowed in a POL approve:
 * https://etherscan.io/address/0x5e3Ef299fDDf15eAa0432e6e66473ace8c13D908 */
#define EVM_pol_STAKE_MANAGER_ADDRESS                                          \
  ((const uint8_t[EVM_ADDRESS_LENGTH]){                                        \
      0x5e, 0x3e, 0xf2, 0x99, 0xfd, 0xdf, 0x15, 0xea, 0xa0, 0x43,              \
      0x2e, 0x6e, 0x66, 0x47, 0x3a, 0xce, 0x8c, 0x13, 0xd9, 0x08})

/** Everstake POL validator share (buy, sell and claim):
 * https://etherscan.io/address/0xF30Cf4ed712D3734161fDAab5B1DBb49Fd2D0E5c */
#define EVM_everstake_POL_VALIDATOR_ADDRESS                                    \
  ((const uint8_t[EVM_ADDRESS_LENGTH]){                                        \
      0xf3, 0x0c, 0xf4, 0xed, 0x71, 0x2d, 0x37, 0x34, 0x16, 0x1f,              \
      0xda, 0xab, 0x5b, 0x1d, 0xbb, 0x49, 0xfd, 0x2d, 0x0e, 0x5c})

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
