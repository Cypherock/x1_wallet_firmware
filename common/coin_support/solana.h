/**
 * @file    solana.h
 * @author  Cypherock X1 Team
 * @brief   Header for solana coin.
 *          Stores declarations for solana coin functions.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef SOLANA_HEADER
#define SOLANA_HEADER

#include <stdint.h>
#include "coin_utils.h"

#define SOLANA_PURPOSE_INDEX 0x8000002C
#define SOLANA_COIN_INDEX    0x800001F5
#define SOLANA_ACCOUNT_INDEX 0x80000000
#define SOLANA_CHAIN_INDEX   0x80000000
#define SOLANA_ADDRESS_INDEX 0x80000001

#define SOLANA_ACCOUNT_ADDRESS_LENGTH 32
#define SOLANA_BLOCKHASH_LENGTH       32

typedef struct solana_transfer_data {
  uint8_t *funding_account;
  uint8_t *recipient_account;
  uint64_t lamports;
} solana_transfer_data;

typedef struct solana_instruction {
  uint8_t program_id_index;
  uint16_t account_addresses_index_count;
  uint8_t *account_addresses_index;
  uint16_t opaque_data_length;
  uint8_t *opaque_data;
  union {
    solana_transfer_data transfer;
  } program;
} solana_instruction;

typedef struct solana_unsigned_txn {
  uint8_t required_signatures_count;
  uint8_t read_only_accounts_require_signature_count;
  uint8_t read_only_accounts_not_require_signature_count;

  uint16_t account_addresses_count;
  uint8_t *account_addresses;

  uint8_t *blockhash;

  uint16_t instructions_count;  // deserialization only supports single instruction
  solana_instruction instruction;

} solana_unsigned_txn;

/**
 * @brief Get the compact array size and number of bytes used to store the size 
 * 
 * @param data the compact array
 * @param size the size of the compact array 
 * @return uint16_t number of bytes used to store the size
 */
uint16_t get_compact_array_size(const uint8_t *data, uint16_t *size);

/**
 * @brief Convert byte array representation of unsigned transaction to solana_unsigned_txn.
 * @details
 *
 * @param [in] byte_array                   Byte array of unsigned transaction.
 * @param [in] byte_array_size              Size of byte array.
 * @param [out] utxn                        Pointer to the solana_unsigned_txn instance to store the transaction details.
 *
 * @return Status of conversion
 * @retval 0 Success
 * @retval -1 Failure
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int solana_byte_array_to_unsigned_txn(uint8_t *byte_array, uint16_t byte_array_size, solana_unsigned_txn *utxn);

/**
 * @brief Signed unsigned byte array.
 * @details
 *
 * @param [in] unsigned_txn_byte_array      Byte array of unsigned transaction.
 * @param [in] unsigned_txn_len             length of unsigned transaction byte array.
 * @param [in] transaction_metadata         Pointer to txn_metadata instance.
 * @param [in] mnemonics                    char array of mnemonics.
 * @param [in] passphrase                   char array of passphrase.
 * @param [out] sig                         Byte array of signature to store the result of signing unsigned transaction byte array.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void solana_sig_unsigned_byte_array(const uint8_t *unsigned_txn_byte_array,
                                    uint64_t unsigned_txn_len,
                                    const txn_metadata *transaction_metadata,
                                    const char *mnemonics,
                                    const char *passphrase,
                                    uint8_t *sig);
#endif  // SOLANA_HEADER
