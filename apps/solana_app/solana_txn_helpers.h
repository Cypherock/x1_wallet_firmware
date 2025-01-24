/**
 * @file    solana_txn_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Helper apis for interpreting and signing Solana transactions
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#ifndef SOLANA_TXN_HELPERS_H
#define SOLANA_TXN_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdint.h>

#include "utils.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/// Ref: https://docs.solana.com/terminology#lamport
#define SOLANA_DECIMAL (9U)

#define SOLANA_ACCOUNT_ADDRESS_LENGTH 32
#define SOLANA_BLOCKHASH_LENGTH 32

#define SOLANA_PROGRAM_ID_COUNT 4    ///< Number of supported program ids
#define SOLANA_SOL_TRANSFER_PROGRAM_ID_INDEX 0
#define SOLANA_TOKEN_PROGRAM_ID_INDEX 1
#define SOLANA_ASSOCIATED_TOKEN_PROGRAM_ID_INDEX 2
#define SOLANA_COMPUTE_BUDGET_PROGRAM_ID_INDEX 3

#define SOLANA_TOKEN_PROGRAM_ADDRESS                                           \
  "06ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a9"    ///< "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA"
#define SOLANA_ASSOCIATED_TOKEN_PROGRAM_ADDRESS                                \
  "8c97258f4e2489f1bb3d1029148e0d830b5a1399daff1084048e7bd8dbe9f859"    ///< "ATokenGPvbdGVxr1b2hvZbsiqW5xWH25efTNsLJA8knL"
#define SOLANA_COMPUTE_BUDGET_PROGRAM_ADDRESS                                  \
  "0306466fe5211732ffecadba72c39be7bc8ce5bbc5f7126b2c439b3a40000000"    ///< "ComputeBudget111111111111111111111111111111"
/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

// Reference:
// https://docs.rs/solana-program/1.14.3/solana_program/system_instruction/enum.SystemInstruction.html
enum SOLANA_SYSTEM_INSTRUCTION {
  SSI_CREATE_ACCOUNT = 0,
  SSI_ASSIGN,
  SSI_TRANSFER,
  SSI_CREATE_ACCOUNT_WITH_SEED,
  SSI_ADVANCE_NONCE_ACCOUNT,
  SSI_WITHDRAW_NONCE_ACCOUNT,
  SSI_INITIALIZE_NONCE_ACCOUNT,
  SSI_AUTHORIZE_NONCE_ACCOUNT,
  SSI_ALLOCATE,
  SSI_ALLOCATE_WITH_SEED,
  SSI_ASSIGN_WITH_SEED,
  SSI_TRANSFER_WITH_SEED,
  SSI_UPGRADE_NONCE_ACCOUNT,
};

/// Ref:
/// https://github.com/solana-labs/solana-program-library/blob/b1c44c171bc95e6ee74af12365cb9cbab68be76c/token/program/src/instruction.rs
enum SOLANA_TOKEN_PROGRAM_INSTRUCTION {
  STPI_INITIALIZE_MINT = 0,
  STPI_INITIALIZE_ACCOUNT,
  STPI_INITIALIZE_MULTISIG,
  STPI_TRANSFER,
  STPI_APPROVE,
  STPI_REVOKE,
  STPI_SET_AUTHORITY,
  STPI_MINT_TO,
  STPI_BURN,
  STPI_CLOSE_ACCOUNT,
  STPI_FREEZE_ACCOUNT,
  STPI_THAW_ACCOUNT,
  STPI_TRANSFER_CHECKED,
  STPI_APPROVE_CHECKED,
  STPI_MINT_TO_CHECKED,
  STPI_BURN_CHECKED,
  STPI_INITIALIZE_ACCOUNT2,
  STPI_SYNC_NATIVE,
  STPI_INITIALIZE_ACCOUNT3,
  STPI_INITIALIZE_MULTISIG2,
  STPI_INITIALIZE_MINT2,
  STPI_GET_ACCOUNT_DATA_SIZE,
  STPI_INITIALIZE_IMMUTABLE_OWNER,
  STPI_AMOUNT_TO_UI_AMOUNT,
  STPI_UI_AMOUNT_TO_AMOUNT
};

/// Ref :
/// https://docs.rs/solana-sdk/latest/solana_sdk/compute_budget/enum.ComputeBudgetInstruction.html
enum SOLANA_COMPUTE_BUDGET_INSTRUCTION {
  SCBI_UNUSED = 0,
  SCBI_REQUEST_HEAP_FRAME,
  SCBI_SET_COMPUTE_UNIT_LIMIT,
  SCBI_SET_COMPUTE_UNIT_PRICE,
  SCBI_SET_LOADED_ACCOUNT_DATA_SIZE_LIMIT
};

enum SOLANA_ERROR_CODES {
  SOL_OK = 0,
  SOL_ERROR,
  SOL_D_MIN_LENGTH,
  SOL_D_COMPACT_U16_OVERFLOW,
  SOL_D_READ_SIZE_MISMATCH,
  SOL_V_UNSUPPORTED_PROGRAM,
  SOL_V_UNSUPPORTED_INSTRUCTION,
  SOL_V_UNSUPPORTED_INSTRUCTION_COUNT,
  SOL_V_INDEX_OUT_OF_RANGE,
  SOL_BU_INVALID_BLOCKHASH,
};

// Reference :
// https://docs.rs/solana-program/1.14.3/solana_program/system_instruction/enum.SystemInstruction.html#variant.Transfer
typedef struct solana_transfer_data {
  uint8_t *funding_account;
  uint8_t *recipient_account;
  uint64_t lamports;
} solana_transfer_data;

// Reference :
// https://docs.rs/spl-token/latest/spl_token/instruction/enum.TokenInstruction.html#variant.TransferChecked
typedef struct solana_token_transfer_checked_data {
  uint8_t *source;
  uint8_t *token_mint;
  uint8_t *destination;
  uint8_t *owner;    // signer/owner of the source account
  uint64_t amount;
  uint8_t decimals;
} solana_token_transfer_checked_data;

// Reference :
// https://docs.rs/solana-sdk/latest/solana_sdk/compute_budget/enum.ComputeBudgetInstruction.html#method.set_compute_unit_limit
typedef struct {
  uint32_t units;
} solana_compute_unit_limit_data;

// Reference :
// https://docs.rs/solana-sdk/latest/solana_sdk/compute_budget/enum.ComputeBudgetInstruction.html#method.set_compute_unit_price
typedef struct {
  uint64_t micro_lamports;
} solana_compute_unit_price_data;

// Reference :
// https://docs.solana.com/developing/programming-model/transactions#instruction-format
typedef struct solana_instruction {
  uint8_t program_id_index;
  uint16_t account_addresses_index_count;
  uint8_t *account_addresses_index;
  uint16_t opaque_data_length;
  uint8_t *opaque_data;
  union {
    solana_transfer_data transfer;
    solana_token_transfer_checked_data transfer_checked;
    solana_compute_unit_limit_data compute_unit_limit_data;
    solana_compute_unit_price_data compute_unit_price_data;
  } program;
} solana_instruction;

// Reference :
// https://docs.solana.com/developing/programming-model/transactions#anatomy-of-a-transaction
typedef struct solana_unsigned_txn {
  uint8_t required_signatures_count;
  uint8_t read_only_accounts_require_signature_count;
  uint8_t read_only_accounts_not_require_signature_count;

  uint16_t account_addresses_count;
  uint8_t *account_addresses;

  uint8_t *blockhash;

  uint16_t instructions_count;    // deserialization only supports max 4
                                  // instructions: compute unit limit, compute unit price, create account and transfer
  solana_instruction instruction[4];     ///< Expects max 4 instructions: TODO: HANDLE ANY NUMBER/TYPE OF INSTRUCTIONS
  uint8_t transfer_instruction_index;    // Expects only 1 transfer instruction
  uint32_t compute_unit_limit;           // To calculate priority fee
  uint64_t compute_unit_price_micro_lamports;

} solana_unsigned_txn;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Get the compact array size and number of bytes used to store the size
 *
 * @param data the compact array
 * @param size the size of the compact array
 * @return uint16_t number of bytes used to store the size
 */

uint16_t get_compact_array_size(const uint8_t *data,
                                uint16_t *size,
                                int *error);

/**
 * @brief Convert byte array representation of unsigned transaction to
 * solana_unsigned_txn.
 * @details
 *
 * @param [in] byte_array                   Byte array of unsigned transaction.
 * @param [in] byte_array_size              Size of byte array.
 * @param [out] utxn                        Pointer to the solana_unsigned_txn
 * instance to store the transaction details.
 *
 * @return Status of conversion
 * @retval 0 if successful
 * @retval -1 if unsuccessful
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int solana_byte_array_to_unsigned_txn(uint8_t *byte_array,
                                      uint16_t byte_array_size,
                                      solana_unsigned_txn *utxn);

/**
 * @brief Validate the deserialized unsigned transaction
 *
 * @param utxn Pointer to the solana_unsigned_txn instance to validate the
 * transaction
 * @return 0 if validation succeeded
 * @return -1 if validation failed
 */
int solana_validate_unsigned_txn(const solana_unsigned_txn *utxn);

/**
 * @brief Update given blockhash in serialized array
 *
 * @param byte_array    pointer to serialized array of unsigned transaction to
 * be modified
 * @param blockhash     pointer to array of latest blockhash
 * @return int
 */
int solana_update_blockhash_in_byte_array(uint8_t *byte_array,
                                          const uint8_t *blockhash);

/**
 * @brief Returns the decimal value of solana asset
 *
 * @return uint8_t decimal value
 */
uint8_t solana_get_decimal();

#endif
