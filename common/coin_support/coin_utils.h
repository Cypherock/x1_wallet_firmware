/**
 * @file    coin_utils.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
#ifndef COIN_UTILS_HEADER
#define COIN_UTILS_HEADER

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base58.h"
#include "bip32.h"
#include "bip39.h"
#include "curves.h"
#include "ecdsa.h"
#include "ripemd160.h"
#include "secp256k1.h"
#include "sha2.h"
#include "utils.h"
#include "logger.h"
#include "assert_conf.h"

/// Bitcoin coin index
#define BITCOIN 0x80000000

/// BTC TEST coin index
#define BTC_TEST (BITCOIN + 0x01)

/// LITECOIN coin index
#define LITCOIN (BITCOIN + 0x02)

/// DOGE coin index
#define DOGE (BITCOIN + 0x03)

/// DASH coin index
#define DASH (BITCOIN + 0x05)

/// ETHEREUM coin index
#define ETHEREUM (BITCOIN + 0x3c)

/// NATIVE SEGWIT purpose id
#define NATIVE_SEGWIT 0x80000054

/// NON SEGWIT purpose id
#define NON_SEGWIT 0x8000002C

/**
 * @brief Struct to store the type of address.
 * @details Used in the derivation of address.
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct
{
    uint8_t chain_index[4];
    uint8_t address_index[4];
} address_type;
#pragma pack(pop)

/**
 * @brief Struct to store the meta data details of a transaction.
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct
{
    uint8_t wallet_index[1];
    uint8_t purpose_index[4];
    uint8_t coin_index[4];
    uint8_t account_index[4];

    uint8_t input_count[1];
    address_type *input;

    uint8_t output_count[1];
    address_type *output;

    uint8_t change_count[1];
    address_type *change;

    uint8_t transactionFees[4];

    uint8_t decimal[1];

    char token_name[8];

    uint8_t network_chain_id;

} txn_metadata;
#pragma pack(pop)

/**
 * @brief Copies the byte values from source after offset to destination under the given size limit.
 * @details
 *
 * @param [out] dst         Destination address.
 * @param [in] src          Source address.
 * @param [in] size         Size of the source array.
 * @param [in] len          No of bytes to be copied.
 * @param [in, out] offset  Source address offset.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void s_memcpy(uint8_t *dst, const uint8_t *src, uint32_t size, uint64_t len, int64_t *offset);

/**
 * @brief Converts byte array represented transaction metadata to struct txn_metadata.
 * @details
 *
 * @param [in] txn_metadata_byte_array  Transaction metadata byte array to be converted.
 * @param [in] size                     Size of the byte array txn_metadata_byte_array
 * @param [out] txn_metadata_ptr        Pointer to the txn_metadata instance to store the field values.
 *
 * @return Offset used in conversion
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int32_t byte_array_to_txn_metadata(const uint8_t *txn_metadata_byte_array, uint32_t size, txn_metadata *txn_metadata_ptr);

/**
 * @brief Generates xpub for the passed purpose id, coin id and account id.
 * @details
 *
 * @param [in] node         HDNode instance to calculate xpub.
 * @param [in] purpose_id   Purpose Id value.
 * @param [in] coin_id      Coin Id value.
 * @param [in] account_id   Account Id value .
 * @param [out] str         char array of generated xpub
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void generate_xpub(const HDNode *node, uint32_t purpose_id, uint32_t coin_id, uint32_t account_id, char *str);

/**
 * @brief Get the address from HDNode.
 * @details
 *
 * @param [in] txn_metadata_ptr     Pointer to txn_metadata instance.
 * @param [in] index                Input index.
 * @param [in] mnemonic             char array storing mnemonics.
 * @param [in] passphrase           char array storing passphrase.
 * @param [out] hdnode              Pointer to HDNode instance used to derive the address.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void get_address_node(const txn_metadata *txn_metadata_ptr, const int16_t index,
                             const char *mnemonic, const char *passphrase, HDNode *hdnode);
/**
 * @brief Get the coin name for the passed coin index and chain id.
 * @details
 *
 * @param [in] coin_index   Coin index
 * @param [in] chain_id     Chain ID (Passed to distinguish between mainnet and testnet)
 *
 * @return [const] char array of name of the coin.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
const char *get_coin_name(int coin_index, uint8_t chain_id);

/**
 * @brief Get the coin symbol for the passed coin index and chain id
 * @details
 *
 * @param [in] coin_index   Coin index
 * @param [in] chain_id     Chain ID (Passed to distinguish between mainnet and testnet)
 *
 * @return [const] char array of symbol of the coin
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
const char *get_coin_symbol(int coin_index, uint8_t chain_id);

/**
 * @brief Get the version address and public key for segwit and non segwit coins.
 * @details
 *
 * @param [in] purpose_id           Purpose Id of the coin.
 * @param [in] coin_index           Coin index of the coin.
 * @param [out] address_version     Pointer to uint8_t instance to store the addres version.
 * @param [out] pub_version         Pointer to uint32_t to instance store the public key version.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void get_version(uint32_t purpose_id, uint32_t coin_index, uint8_t* address_version, uint32_t* pub_version);

/**
 * @brief
 * @details
 *
 * @param [in] txn_metadata_ptr
 *
 * @return bool
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
bool validate_txn_metadata(const txn_metadata *txn_metadata_ptr);

#endif