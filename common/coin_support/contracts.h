#ifndef CY_CONTRACTS_H
#define CY_CONTRACTS_H

#include <stdint.h>

/// EVM function parameter block-size length
#define EVM_FUNC_PARAM_BLOCK_LENGTH     32

/// EVM function parameter block-size length
#define EVM_FUNC_SIGNATURE_LENGTH       4

/// EVM transfer method signature
#define TRANSFER_FUNC_SIGNATURE         0xa9059cbb

/// Length of Ethereum public addresses in bytes
#define ETHEREUM_ADDRESS_LENGTH         20

/**
 * @brief An expected limit on length of Ethereum based ERC20 token symbols.
 * @details The token symbol is also exchanged in txn_metadata.token_name.
 *
 * @see <a href="https://ethereum.stackexchange.com/a/117645" target="_blank">StackExchange question</a>
 */
#define ETHEREUM_TOKEN_SYMBOL_LENGTH    20

/// Number of entries in whitelisted contracts list
#define WHITELISTED_CONTRACTS_COUNT     1778

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
} erc20_contracts_t;

/**
 * @brief Whitelisted contracts with respective token symbol
 * @details A map of Ethereum contract addresses with their token symbols. These will
 * enable the device to verify the ERC20 token transaction in a user-friendly manner.
 *
 * @see erc20_contracts_t
 * @since
 * @deprecated
 *
 * @note
 * @todo
 */
extern const erc20_contracts_t whitelisted_contracts[WHITELISTED_CONTRACTS_COUNT];

#endif //CY_CONTRACTS_H
