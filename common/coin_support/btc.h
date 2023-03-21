/**
 * @file    btc.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
#ifndef BTC_HEADER
#define BTC_HEADER

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base58.h"
#include "bip32.h"
#include "bip39.h"
#include "coin_utils.h"
#include "curves.h"
#include "ecdsa.h"
#include "ripemd160.h"
#include "secp256k1.h"
#include "sha2.h"

#define BTC_COIN_VERSION 0x00000000

#define OP_RETURN 0x6A

#define SATOSHI_PER_BTC     100000000
#define MAX_SCRIPT_SIG_SIZE 128

/**
 * @brief Struct to store details of Unsigned Transaction Input.
 * @details
 *
 * @see signed_txn_input
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t segwit[1];
  uint8_t previous_txn_hash[32];
  uint8_t previous_output_index[4];
  uint8_t value[8];
  uint8_t script_length[1];
  uint8_t script_public_key[25];
  uint8_t sequence[4];
} unsigned_txn_input;
#pragma pack(pop)

/**
 * @brief Struct to store details of Signed Transaction Input.
 * @details
 *
 * @see unsigned_txn_input
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t previous_txn_hash[32];
  uint8_t previous_output_index[4];
  uint8_t script_length[1];
  uint8_t *script_sig;
  uint8_t sequence[4];
} signed_txn_input;
#pragma pack(pop)

/**
 * @brief Struct to store details of Transaction Output.
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t value[8];
  uint8_t script_length[1];
  uint8_t script_public_key[35];
} txn_output;
#pragma pack(pop)

/**
 * @brief Struct to store the details of segwit witnesses.
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t item_count[1];

  uint8_t witness_length[1];
  uint8_t *witness;
} txn_witness;
#pragma pack(pop)

/**
 * @brief Struct to store the details of an Unsigned Transaction.
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t network_version[4];
  uint8_t input_count[1];
  unsigned_txn_input *input;
  uint8_t output_count[1];
  txn_output *output;
  uint8_t locktime[4];
  uint8_t sighash[4];

} unsigned_txn;
#pragma pack(pop)

/**
 * @brief Struct to store details of the preimage of a Transaction.
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t network_version[4];
  uint8_t hash_prevouts[32];
  uint8_t hash_sequence[32];
  uint8_t previous_txn_hash[32];
  uint8_t previous_output_index[4];
  uint8_t script_length[1];
  uint8_t script_code[25];
  uint8_t value[8];
  uint8_t sequence[4];
  uint8_t hash_outputs[32];
  uint8_t locktime[4];
  uint8_t sighash[4];
  uint8_t filled;
} txn_preimage;
#pragma pack(pop)

/**
 * @brief Struct to store the details of a Signed Transaction.
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t network_version[4];
  uint8_t marker[1];
  uint8_t flag[1];
  uint8_t input_count[1];
  signed_txn_input *input;
  uint8_t output_count[1];
  txn_output *output;
  txn_witness *witness;
  uint8_t locktime[4];
} signed_txn;
#pragma pack(pop)

/**
 * @brief Stores the generated signature on transaction data for the send transaction process.
 *
 * @see send_transaction_controller(), send_transaction_task(), desktop_listener_task(), SEND_TXN_START
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Send_Transaction_Cmd {
  uint8_t signed_txn_byte_array[MAX_SCRIPT_SIG_SIZE];
  int signed_txn_length;
} Send_Transaction_Cmd;
#pragma pack(pop)

/**
 * @brief Converts byte array represented unsigned transaction to struct unsigned_txn.
 * @details
 *
 * @param [in] utxn_byte_array  Unsigned transaction byte array to be converted.
 * @param [in] size             Size of the byte array utxn_byte_array
 * @param [out] utxn_ptr        Pointer to the unsigned_txn instance to store the field values.
 *
 * @return Offset used in conversion
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int32_t byte_array_to_unsigned_txn(const uint8_t *utxn_byte_array,
                                   uint32_t size,
                                   unsigned_txn *utxn_ptr);

/**
 * @brief Signs the provided unsigned transaction
 * @details
 *
 * @param [in] utxn_ptr          Pointer to the unsigned_txn instance.
 * @param [in] txn_metadata_ptr  Pointer to the txn_metadata instance.
 * @param [in] index             Input index that will be included
 * @param [in] mnemonic          char array of mnemonic
 * @param [in] passphrase        char array of passphrase
 * @param [in] preimage          Pointer to txn_preimage instance.
 * @param [out] script_sig       Byte array which will store the signature.
 *
 * @return Length of the signature byte array.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int sig_from_unsigned_txn(const unsigned_txn *utxn_ptr,
                          txn_metadata *txn_metadata_ptr,
                          uint32_t index,
                          const char *mnemonic,
                          const char *passphrase,
                          txn_preimage *preimage,
                          uint8_t *script_sig);

/**
 * @brief Verifies the UTXOs.
 * @details
 *
 * @param [in] raw_txn  Byte array of a transaction.
 * @param [in] size     Size of the raw_txn.
 * @param [in] input    Pointer to the unsigned_txn_input.
 *
 * @return Result of verification, true if verified and false if unverified.
 * @retval true UTXO verified.
 * @retval false UTXO unverified.
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
bool btc_verify_utxo(const uint8_t *raw_txn,
                     uint32_t size,
                     const unsigned_txn_input *input);

/**
 * @brief Get the segwit address string.
 * @details
 *
 * @param [in] public_key   Byte array representation of public key.
 * @param [in] key_len      Length of public key byte array.
 * @param [in] coin_index   Byte value of coin index.
 * @param [out] address     char array to store segwit address.
 *
 * @return 1 if successful and 0 if failure.
 * @retval 1 Success
 * @retval 0 Failure
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int get_segwit_address(const uint8_t *public_key,
                       uint8_t key_len,
                       uint32_t coin_index,
                       char *address);

/**
 * @brief Get the address from the passed public key.
 * @details
 *
 * @param [in] hrp              hrp value for segwit addresses
 * @param [in] script_pub_key   Byte array of public key.
 * @param [out] address_output  char array of address
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int get_address(const char *hrp,
                const uint8_t *script_pub_key,
                uint8_t addr_version,
                char *address_output);

/**
 * @brief Get the transaction fee of an unsigned transaction.
 * @details
 *
 * @param [in] utxn_ptr     Pointer to unsigned_txn instance.
 *
 * @return Transaction fee.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint64_t btc_get_txn_fee(const unsigned_txn *utxn_ptr);

/**
 * @brief
 * @details
 *
 * @param [in] unsigned_txn_ptr Instance of unsigned_txn
 * @param [in] coin_index       Coin index in HD-Key derivation path
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint64_t get_transaction_fee_threshold(const unsigned_txn *unsigned_txn_ptr,
                                       uint32_t coin_index);

/**
 * @brief Validates the structure of a Bitcoin (and its forks) transaction. It checks for existance of at
 * least one input and one output transaction. Supported UTXOs length validation and supported sighash value.
 * @details
 *
 * @param [in] unsigned_txn_ptr
 *
 * @return true, false
 * @retval true     If all the checks are success.
 * @retval false,   If any of the checks fail.
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
bool btc_validate_unsigned_txn(const unsigned_txn *unsigned_txn_ptr);

/**
 * @brief Validates the change address for a Bitcoin (and its forks) transaction.
 * @details It checks for existence of the change address and if it is a valid address. The change address is extracted
 * from the script (only P2PKH & P2WPKH supported) of the last output transaction.
 *
 * @param [in] utxn_ptr                 - Pointer to unsigned_txn instance.
 * @param [in, out] txn_metadata_ptr    - Pointer to txn_metadata instance.
 * @param [in] mnemonic                 - char array of mnemonic
 * @param [in] passphrase               - char array of passphrase
 *
 * @return bool - Indicates whether the change address is valid or not.
 * @retval true     If the change address is valid.
 * @retval false    If the change address is invalid or script type is unsupported.
 *
 * @see btc_sign_unsigned_txn(), BITCOIN, unsigned_txn, txn_metadata, HDNode, btc_validate_unsigned_txn()
 * @since v1.0.0
 *
 * @note The BTC change address is always a segwit address. So the metadata is updated to segwit's
 * purpose index ignoring what is passed in the metadata.
 */
bool validate_change_address(const unsigned_txn *utxn_ptr,
                             const txn_metadata *txn_metadata_ptr,
                             const char *mnemonic,
                             const char *passphrase);

#endif
