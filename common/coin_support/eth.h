/**
 * @file    eth.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
#ifndef ETH_HEADER
#define ETH_HEADER

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../crypto/base58.h"
#include "../crypto/bip32.h"
#include "../crypto/bip39.h"
#include "../crypto/curves.h"
#include "../crypto/ecdsa.h"
#include "../crypto/ripemd160.h"
#include "../crypto/secp256k1.h"
#include "../crypto/sha2.h"
#include "../crypto/sha3.h"
#include "coin_utils.h"
#include "eth_sign_data/simple.pb.h"

#define ETHEREUM_PURPOSE_INDEX 0x8000002C
#define ETHEREUM_COIN_INDEX    0x8000003C

#define ETHEREUM_MAINNET_CHAIN 1
#define ETHEREUM_ROPSTEN_CHAIN 3

#define ETHEREUM_MAINNET_NAME "ETH Mainnet"
#define ETHEREUM_ROPSTEN_NAME "ETH Ropsten"
#define ETHEREUM_TOKEN_NAME   "Ether"
#define ETHEREUM_TOKEN_SYMBOL "ETH"

// \x45(E) is needed otherwise \x19e is considered instead of \x19
#define ETH_PERSONAL_SIGN_IDENTIFIER "\x19\x45thereum Signed Message:\n"

/// Convert byte array to unit32_t
#define ETH_VALUE_SIZE_BYTES (32U)
#define ETH_NONCE_SIZE_BYTES (32U)
#define ETH_GWEI_INDEX       (9U)

#define ETH_COIN_VERSION 0x00000000

#define ETH_UTXN_ABI_DECODE_OK      (0xAA)
#define ETH_UTXN_BAD_PAYLOAD        (0x11)
#define ETH_UTXN_FUNCTION_NOT_FOUND (0x11)
#define ETH_BAD_ARGUMENTS           (0x22)


/// Enum used to differentiate between a single val, string of bytes and list of strings during rlp decoding/encoding in raw eth byte array
typedef enum { NONE, STRING, LIST } seq_type;

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

  uint64_t payload_size;
  uint8_t *payload;

  uint8_t chain_id_size[1];
  uint8_t chain_id[4];

  uint8_t dummy_r[1];
  uint8_t dummy_s[1];

  uint8_t contract_verified;
} eth_unsigned_txn;
#pragma pack(pop)

typedef struct ui_display_node {
  char *title;
  char *value;
  struct ui_display_node *next;
} ui_display_node;

/**
 * @brief Converts bendian byte array to decimal uint64_t.
 * @details
 *
 * @param [in] bytes    Bendian byte array to convert.
 * @param [in] len      Length of the byte array.
 *
 * @return Converted byte array to decimal.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint64_t bendian_byte_to_dec(const uint8_t *bytes, uint8_t len);

/**
 * @brief Convert hex char array to decimal.
 * @details
 *
 * @param source    Hex char array.
 *
 * @return Converted decimal uint64_t.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint64_t hex2dec(const char *source);

/**
 * @brief Get the receivers address from eth_unsigned_txn instance.
 * @details
 *
 * @param [in] eth_unsigned_txn_ptr     Pointer to Unsigned transaction instance.
 * @param [in] address                  Byte array of receiver's address.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void eth_get_to_address(const eth_unsigned_txn *eth_unsigned_txn_ptr, uint8_t *address);

/**
 * @brief Get amount to be sent set in the eth_unsigned_txn instance
 * @details
 *
 * @param [in] eth_unsigned_txn_ptr     Pointer to Unsigned transaction instance.
 * @param [in] value                    char array to store value.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint32_t eth_get_value(const eth_unsigned_txn *eth_unsigned_txn_ptr, char *value);

/**
 * @brief Verifies the unsigned transaction.
 * @details
 *
 * @param [in] eth_utxn_ptr     Pointer to the eth_unsigned_txn instance.
 *
 * @return true, false
 * @retval true   If all the checks pass for the given instance
 * @retval false  If any of the checks pass for the given instance
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
bool eth_validate_unsigned_txn(eth_unsigned_txn *eth_utxn_ptr, txn_metadata *metadata_ptr);

/**
 * @brief Convert byte array representation of unsigned transaction to eth_unsigned_txn.
 * @details
 *
 * @param [in] eth_unsigned_txn_byte_array  Byte array of unsigned transaction.
 * @param [in] byte_array_len               Length of byte array.
 * @param [out] unsigned_txn_ptr            Pointer to the eth_unsigned_txn instance to store the transaction details.
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
int eth_byte_array_to_unsigned_txn(const uint8_t *eth_unsigned_txn_byte_array,
                                   size_t byte_array_len,
                                   eth_unsigned_txn *unsigned_txn_ptr);

/**
 * @brief Convert byte array representation of message to an object using protobuf.
 * @details
 *
 * @param [in] eth_msg                      Byte array of message.
 * @param [in] byte_array_len               Length of byte array.
 * @param [out] msg_data                    Pointer to the MessageData instance to store the message details.
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
int eth_byte_array_to_msg(const uint8_t *eth_msg, size_t byte_array_len, MessageData *msg_data);
/**
 * @brief Signed unsigned byte array.
 * @details
 *
 * @param [in] eth_unsigned_txn_byte_array  Byte array of unsigned transaction.
 * @param [in] eth_unsigned_txn_len         length of unsigned transaction byte array.
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
void sig_unsigned_byte_array(const uint8_t *eth_unsigned_txn_byte_array,
                             uint64_t eth_unsigned_txn_len,
                             const txn_metadata *transaction_metadata,
                             const char *mnemonics,
                             const char *passphrase,
                             uint8_t *sig);

/**
 * @brief Return the string representation of decimal value of transaction fee in ETH.
 *
 * @param eth_unsigned_txn_ptr  The unsigned transaction containing gas_limit and gas_price
 * @param fee_decimal_string    Output decimal string of at least 30 character long
 */
void eth_get_fee_string(eth_unsigned_txn *eth_unsigned_txn_ptr,
                        char *fee_decimal_string,
                        uint8_t size,
                        uint8_t decimal);

/**
 * @brief checks whether given token in metadata is whitelisted
 * 
 * @param metadata_ptr 
 * @return true 
 * @return false 
 */
bool is_token_whitelisted(txn_metadata *metadata_ptr);

/**
 * @brief Initialize MessageData structure from protobuf
 * 
 * @param msg_data 
 */
void eth_init_msg_data(MessageData *msg_data);

/**
 * @brief Get the data as string object
 * 
 * @param msg_data 
 * @param output 
 * @param out_length 
 * @return true 
 * @return false 
 */
bool get_data_as_string(const MessageData *msg_data, char *output, size_t out_length);

/**
 * @brief Initialize Display Nodes from message data
 * 
 * @param node 
 * @param msg_data 
 */
void eth_init_display_nodes(ui_display_node **node, MessageData *msg_data);

/**
 * @brief Create a new display node and return its pointer
 * 
 * @param title 
 * @param title_size 
 * @param value 
 * @param value_size 
 * @return ui_display_node* 
 */
ui_display_node *eth_create_display_node(const char *title,
                                         const size_t title_size,
                                         const char *value,
                                         const size_t value_size);

/**
 * @brief This function extracts Abi encoded arguments for EVM functions into UI 
 * compatible nodes ui_display_node(s)
 * 
 * @param pAbiPayload Pointer to start of payload of the EVM transaction
 * @param sizeOfUTxn Size of payload of the EVM transaction
 * @return uint8_t Depicts the status of operation for this function
 * ETH_BAD_ARGUMENTS: If any argument is invalid
 * ETH_UTXN_FUNCTION_NOT_FOUND: If a function NOT supported by X1 wallet is in the EVM tx
 * ETH_UTXN_BAD_PAYLOAD: If a payload contains invalid data
 * ETH_UTXN_ABI_DECODE_OK: If the arguments are extracted successfully
 */
uint8_t ETH_ExtractArguments(const uint8_t *pAbiPayload, const uint64_t sizeOfPayload);

uint16_t get_unsigned_data_array_from_msg(const MessageData *msg_data, uint8_t **out);

void eth_sign_msg_data(const MessageData *msg_data,
                       const txn_metadata *transaction_metadata,
                       const char *mnemonics,
                       const char *passphrase,
                       uint8_t *sig);
#endif