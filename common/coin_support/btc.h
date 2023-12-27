/**
 * @file    btc.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
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

#define SATOSHI_PER_BTC 100000000
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
 * @brief Stores the generated signature on transaction data for the send
 * transaction process.
 *
 * @see send_transaction_controller(), send_transaction_task(),
 * desktop_listener_task(), SEND_TXN_START
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Send_Transaction_Cmd {
  uint8_t signed_txn_byte_array[MAX_SCRIPT_SIG_SIZE];
  int signed_txn_length;
} Send_Transaction_Cmd;
#pragma pack(pop)

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

#endif
