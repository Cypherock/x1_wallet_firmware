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
#define LTC_COIN_VERSION     0x00000000

/// DOGE coin index
#define DOGE (BITCOIN + 0x03)
#define DOGE_COIN_VERSION     0x00000000

/// DASH coin index
#define DASH (BITCOIN + 0x05)
#define DASH_COIN_VERSION     0x00000000

/// ETHEREUM coin index
#define ETHEREUM ETHEREUM_COIN_INDEX

/// NEAR coin index
#define NEAR (BITCOIN + 0x18d)

/// SOLANA coin index
#define SOLANA (BITCOIN + 0x1F5)

/// NATIVE SEGWIT purpose id
#define NATIVE_SEGWIT 0x80000054

/// NON SEGWIT purpose id
#define   NON_SEGWIT 0x8000002C

typedef enum Coin_Type {
    COIN_TYPE_BITCOIN          = 0x01,
    COIN_TYPE_BTC_TEST         = 0x02,
    COIN_TYPE_LITECOIN         = 0x03,
    COIN_TYPE_DOGE             = 0x04,
    COIN_TYPE_DASH             = 0x05,
    COIN_TYPE_ETHEREUM         = 0x06,
    COIN_TYPE_NEAR             = 0x07,
    COIN_TYPE_POLYGON          = 0x08,
    COIN_TYPE_SOLANA           = 0x09,
    COIN_TYPE_BSC              = 0x0A,
    COIN_TYPE_FANTOM           = 0x0B,
    COIN_TYPE_AVALANCHE        = 0x0C,
    COIN_TYPE_OPTIMISM         = 0x0D,
    COIN_TYPE_HARMONY          = 0x0E,
    COIN_TYPE_ETHEREUM_CLASSIC = 0x0f,
    COIN_TYPE_ARBITRUM         = 0x10,
} Coin_Type;

#pragma pack(push, 1)
/**
 * @brief Struct to store the type of address.
 * @details Used in the derivation of address.
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
typedef struct
{
    uint8_t change_index[4];
    uint8_t address_index[4];
} address_type;
#pragma pack(pop)

/**
 * @brief Stores the chosen wallet's public information for the export wallet process.
 * @details The instance of this struct is stored temporarily in the RAM during the add coin process. The coin's
 * information is provided by the desktop app and updated in desktop listener task.
 *
 * @see add_coin_controller(), add_coin_task(), desktop_listener_task(), ADD_COIN_START
 * @since v1.0.0
 */
#pragma pack(push, 1)
typedef struct Add_Coin_Data {
  size_t derivation_depth;
  uint32_t derivation_path[5];
  uint64_t network_chain_id;
} Add_Coin_Data;
#pragma pack(pop)

#pragma pack(push, 1)
/**
 * @brief Struct to store the meta data details of a transaction.
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
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

    uint8_t transaction_fees[8];

    uint8_t eth_val_decimal[1];

    char *token_name;

    uint64_t network_chain_id;

    uint8_t is_harmony_address;
     ///< Used to differentiate between multiple address derivation paths of the same coin
    uint16_t address_tag; 
} txn_metadata;
#pragma pack(pop)

#pragma pack(push, 1)
/**
 * @brief Stores the deserialized information from desktop for the receive transaction process.
 * @details
 *
 * @see receive_transaction_controller(), receive_transaction_controller_eth(), receive_transaction_tasks(),
 * desktop_listener_task(), RECV_TXN_START
 * @since v1.0.0
 */
typedef struct Receive_Transaction_Data {
  uint8_t wallet_id[WALLET_ID_SIZE];
  uint8_t purpose[4];
  uint8_t coin_index[4];
  uint8_t account_index[4];
  uint8_t change_index[4];
  uint8_t address_index[4];
  char *token_name;
  union {
    uint64_t network_chain_id;
    uint64_t near_account_type;
  };
  uint16_t address_tag;
  char near_registered_account[65];
  uint8_t xpub[112];
  char address[43];
  uint8_t eth_pubkeyhash[20];
  uint8_t near_pubkey[32];
  bool near_acc_found;
  size_t near_acc_count;
  uint8_t near_acc_index;
  char solana_address[45];
} Receive_Transaction_Data;
#pragma pack(pop)

typedef struct ui_display_node {
  char *title;
  char *value;
  struct ui_display_node *next;
} ui_display_node;

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
 * @brief Deserialize the request payload to add coin.
 * @details If any of the input references are NULL, this function returns `-1`.
 * The minimum and maximum depth for derivation is 2 and 5 respectively. If the input byte array
 * is shorter than the expected data to be parsed, this function will return -1.
 *
 * @param [out]   Pointer to the add coin data instace
 * @param [in]    Serialized payload to be deserialized
 * @param [in]    Size of the input payload
 *
 * @return Offset used in the conversion
 * @retval -1 if the input does not meet expected format/requirements
 */
int64_t byte_array_to_add_coin_data(Add_Coin_Data *data_ptr, const uint8_t *byte_array, size_t size);

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
int64_t byte_array_to_txn_metadata(const uint8_t *txn_metadata_byte_array, uint32_t size, txn_metadata *txn_metadata_ptr);

/**
 * @brief Deserialize byte array to receive transaction data
 * 
 * @param [out] txn_data_ptr            Pointer to the receive transaction data instance 
 * @param [in] data_byte_array          Byte array to be deserialized    
 * @param [in] size                     Size of the byte array data_byte_array
 * @return int32_t Offset used in conversion
 */
int64_t byte_array_to_recv_txn_data(Receive_Transaction_Data *txn_data_ptr,const uint8_t *data_byte_array, const uint32_t size);

/**
 * @brief Generates xpub for the passed purpose id, coin id and account id.
 * 
 * @param [in] path             Path of the node to derive xpub uses fingerprint of second last node. and assumes first two nodes are purpose and coin index.
 * @param [in] path_length      Length of the given path.
 * @param [in] curve            Curve to be used.
 * @param [in] seed             Seed to generate the master node.
 * @param [out] str              String to store the xpub.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void generate_xpub(const uint32_t *path,const size_t path_length, const char *curve,const uint8_t *seed, char *str);

/**
 * @brief Get the hdnode at given path from seed.
 * @details
 *
 * @param [in] path                 Path to derive the hdnode.
 * @param [in] path_length          Length of the path.
 * @param [in] curve                Curve name.
 * @param [in] seed                 Seed to derive the hdnode.
 * @param [out] hdnode              Pointer to the HDNode instance used to store the derived hdnode.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void derive_hdnode_from_path(const uint32_t *path, const size_t path_length, const char *curve, const uint8_t *seed, HDNode *hdnode);

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
 * @param [in] chain_id     Chain ID (Passed to distinguish between different EVM chains)
 *
 * @return [const] char array of name of the coin.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
const char *get_coin_name(uint32_t coin_index, uint64_t chain_id);

/**
 * @brief Get the coin symbol for the passed coin index and chain id
 * @details
 *
 * @param [in] coin_index   Coin index
 * @param [in] chain_id     Chain ID (Passed to distinguish between different EVM chains)
 *
 * @return [const] char array of symbol of the coin
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
const char *get_coin_symbol(uint32_t coin_index, uint64_t chain_id);

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

/**
 * @brief Validates transaction metadata for near coin.
 * @details
 *
 * @param [in] metadata_ptr
 *
 * @return bool
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
bool validate_txn_metadata_near(const txn_metadata *mdata_ptr);

void bech32_addr_encode(char *output, char *hrp, uint8_t *address_bytes, uint8_t byte_len);

/**
 * @brief Verifies the derivation path for xpub during coin export step
 *
 * @param[in] path          The address derivation path to be checked
 * @param[in] depth         The number of levels in the derivation path
 *
 * @return bool     true if the path values are valid. False otherwise.
 *
 * @since v1.0.0
 */
bool verify_xpub_derivation_path(const uint32_t *path, uint8_t depth);

/**
 * @brief Verifies if the specified derivation path is valid based on checks
 * on intermediate values.
 *
 * @param[in] path          The address derivation path to be checked
 * @param[in] depth         The number of levels in the derivation path
 *
 * @return bool     Returns true if the path values are valid. False otherwise.
 *
 * @since v1.0.0
 */
bool verify_receive_derivation_path(const uint32_t *path, uint8_t depth);

/**
 * @brief Generates an user readable derivation path from a uint32_t path array.
 *        will only write the output until the out_len is reached, i.e. an error will be returned
 *        if the output exceeds the out_len. Will also return an error if path or output is NULL
 *        or out_len is 0.
 * 
 * @param path              The derivation path array to be used
 * @param path_length       Number of elements (depth) of the path array
 * @param harden_all        Treat all elements as hardened i.e. insert ' after every element
 * @param output            Pointer to the character array to be used for output
 * @param out_len           Maximum length of the output character array
 */
FUNC_RETURN_CODES derivation_path_array_to_string(const uint32_t *path,
                                     const size_t path_length,
                                     const bool harden_all,
                                     char *output,
                                     const size_t out_len);

/**
 * @brief Create a new display node and return its pointer
 * 
 * @param title 
 * @param title_size 
 * @param value 
 * @param value_size 
 * @return ui_display_node* 
 */
ui_display_node *ui_create_display_node(const char *title,
                                        const size_t title_size,
                                        const char *value,
                                        const size_t value_size);

#endif