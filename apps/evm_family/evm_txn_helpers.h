/**
 * @file    evm_txn_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Helper apis for interpreting and signing EVM transactions
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef EVM_TXN_HELPERS_H
#define EVM_TXN_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "coin_utils.h"
#include "evm/sign_txn.pb.h"
#include "evm_contracts.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define ETHEREUM_PURPOSE_INDEX 0x8000002C
#define ETHEREUM_COIN_INDEX 0x8000003C

#define ETH_VALUE_SIZE_BYTES (32U)

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

typedef enum {
  /// No data present in the transaction
  EVM_TXN_NO_DATA = 0x0,

  /// Data present in the transaction is invalid
  EVM_TXN_INVALID_DATA,

  /// Function signature in data is not recognized
  EVM_TXN_UNKNOWN_FUNC_SIG,

  /// Data is for token transfer function
  EVM_TXN_TOKEN_TRANSFER_FUNC,

  /// Function signature is known; decode function params to display
  EVM_TXN_KNOWN_FUNC_SIG,
} EVM_TRANSACTION_TYPE;

/**
 * @brief Struct to store Unsigned Ethereum Transaction details.
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t nonce_size[1];
  uint8_t nonce[32];

  uint8_t gas_price_size[1];
  uint8_t gas_price[32];

  uint8_t gas_limit_size[1];
  uint8_t gas_limit[32];

  uint8_t to_address[20];

  uint8_t value_size[1];
  uint8_t value[ETH_VALUE_SIZE_BYTES];

  uint64_t data_size;
  const uint8_t *data;

  uint8_t chain_id_size[1];
  uint8_t chain_id[8];

  uint8_t dummy_r[1];
  uint8_t dummy_s[1];
} evm_unsigned_txn;
#pragma pack(pop)

typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by handle_initiate_query()
   */
  evm_sign_txn_initiate_request_t init_info;

  /// remembers the allocated buffer for holding complete unsigned transaction
  uint8_t *transaction;

  /// store for decoded unsigned transaction info
  evm_unsigned_txn transaction_info;

  /// whitelisted contract in the transaction
  const erc20_contracts_t *contract;

  EVM_TRANSACTION_TYPE txn_type;

  /// pointer to maintain a list of display nodes
  ui_display_node *display_node;
} evm_txn_context_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Convert byte array representation of unsigned transaction to
 * evm_unsigned_txn.
 * @details
 *
 * @param [in] evm_utxn_byte_array  Byte array of unsigned transaction.
 * @param [in] byte_array_len               Length of byte array.
 * @param [out] unsigned_txn_ptr            Pointer to the evm_unsigned_txn
 * instance to store the transaction details.
 *
 * @return Status of conversion
 * @retval 0 Success
 * @retval -1 Failure
 */
int evm_decode_unsigned_txn(const uint8_t *evm_utxn_byte_array,
                            size_t byte_array_len,
                            evm_txn_context_t *txn_context);

/**
 * @brief Verifies the unsigned transaction.
 * @details
 *
 * @param [in] txn_context     Pointer to the evm_unsigned_txn instance.
 *
 * @return true, false
 * @retval true   If all the checks pass for the given instance
 * @retval false  If any of the checks pass for the given instance
 */
bool evm_validate_unsigned_txn(const evm_txn_context_t *txn_context);

/**
 * @brief Get the receivers address from evm_unsigned_txn instance.
 * @details
 *
 * @param [in] utxn_ptr     Pointer to Unsigned transaction
 * instance.
 * @param [in] address                  Byte array of receiver's address.
 * @param [in] metadata_ptr             Pointer to metadata instance
 *
 * @return
 * @retval
 */
void eth_get_to_address(const evm_txn_context_t *txn_context,
                        const uint8_t **address);

/**
 * @brief Get amount to be sent set in the evm_unsigned_txn instance
 * @details
 *
 * @param [in] utxn_ptr     Pointer to Unsigned transaction
 * instance.
 * @param [in] value                    char array to store value.
 *
 * @return
 * @retval
 */
uint32_t eth_get_value(const evm_txn_context_t *txn_context, char *value);

/**
 * @brief Return the string representation of decimal value of transaction fee
 * in ETH.
 *
 * @param utxn_ptr  The unsigned transaction containing gas_limit
 * and gas_price
 * @param fee_decimal_string    Output decimal string of at least 30 character
 * long
 */
void eth_get_fee_string(const evm_unsigned_txn *utxn_ptr,
                        char *fee_decimal_string,
                        uint8_t size,
                        uint8_t decimal);

/**
 * @brief Returns the decimal value of ethereum asset from metadata
 *
 * @param txn_context Pointer to transaction context
 * @return uint8_t Decimal value of current asset
 */
uint8_t evm_get_decimal(const evm_txn_context_t *txn_context);

/**
 * @brief Returns the asset symbol which is currently being used in the flow
 *
 * @param txn_context Pointer to transaction context
 * @return const char*
 */
const char *evm_get_asset_symbol(const evm_txn_context_t *txn_context);

#endif
