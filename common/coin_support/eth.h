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


#include "../crypto/bip39.h"
#include "../crypto/bip32.h"
#include "../crypto/curves.h"
#include "../crypto/secp256k1.h"
#include "../crypto/sha2.h"
#include "../crypto/ecdsa.h"
#include "../crypto/ripemd160.h"
#include "../crypto/base58.h"
#include "../crypto/sha3.h"
#include "coin_utils.h"
#include "abi.h"
#include "eip712.pb.h"

#define ETHEREUM_PURPOSE_INDEX    0x8000002C
#define ETHEREUM_COIN_INDEX       0x8000003C

#define ETHEREUM_MAINNET_CHAIN    1

#define ETHEREUM_MAINNET_NAME   "Ethereum"
#define ETHEREUM_TOKEN_SYMBOL   "ETH"
// \x45(E) is needed otherwise \x19e is considered instead of \x19
#define ETH_PERSONAL_SIGN_IDENTIFIER   "\x19\x45thereum Signed Message:\n"
#define ETH_SIGN_TYPED_DATA_IDENTIFIER "\x19\x01"

/// Convert byte array to unit32_t
#define ETH_VALUE_SIZE_BYTES (32U)
#define ETH_NONCE_SIZE_BYTES (32U)
#define ETH_GWEI_INDEX       (9U)
#define ETH_DECIMAL          (18U)

#define ETH_COIN_VERSION     0x00000000

#define ETH_UTXN_ABI_DECODE_OK      (0xAA)
#define ETH_UTXN_BAD_PAYLOAD        (0x11)
#define ETH_UTXN_FUNCTION_NOT_FOUND (0x11)
#define ETH_BAD_ARGUMENTS           (0x22)

/// Enum used to differentiate between a single val, string of bytes and list of strings during rlp decoding/encoding in raw eth byte array
typedef enum { NONE, STRING, LIST } seq_type;

/**
 * @brief Enum used to represent the status of payload field in a transaction.
 * 
 */
typedef enum {
  PAYLOAD_ABSENT = 0x0,               // No payload present in the transaction
  PAYLOAD_SIGNATURE_NOT_WHITELISTED,  // Payload function signature is not recognized [Blind Signing]
  PAYLOAD_CONTRACT_NOT_WHITELISTED,   // Payload function signature is whitelisted but contract is not (for Transfer function) [Unverified Contract]
  PAYLOAD_CONTRACT_INVALID,           // Payload function signature and contract both are whitelisted but doesn't match [Invalid Transaction]
  PAYLOAD_WHITELISTED,                // Payload is completely recognized [Clear Signing]
} PAYLOAD_STATUS;

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
typedef struct
{
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
  uint8_t chain_id[8];

  uint8_t dummy_r[1];
  uint8_t dummy_s[1];

  PAYLOAD_STATUS payload_status;
} eth_unsigned_txn;
#pragma pack(pop)

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
 * @param [in] metadata_ptr             Pointer to metadata instance
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void eth_get_to_address(const eth_unsigned_txn *eth_unsigned_txn_ptr,
                        uint8_t *address,
                        const txn_metadata *metadata_ptr);

/**
 * @brief Get amount to be sent set in the eth_unsigned_txn instance
 * @details
 *
 * @param [in] eth_unsigned_txn_ptr     Pointer to Unsigned transaction instance.
 * @param [in] value                    char array to store value.
 * @param [in] metadata_ptr             Pointer to metadata instance
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint32_t eth_get_value(const eth_unsigned_txn *eth_unsigned_txn_ptr, char *value, const txn_metadata *metadata_ptr);

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
bool eth_validate_unsigned_txn(const eth_unsigned_txn *eth_utxn_ptr, txn_metadata *metadata_ptr);

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
                                   eth_unsigned_txn *unsigned_txn_ptr,
                                   const txn_metadata *metadata_ptr);

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
void sig_unsigned_byte_array(const uint8_t *eth_unsigned_txn_byte_array, uint64_t eth_unsigned_txn_len,
                             const txn_metadata *transaction_metadata, const char *mnemonics,
                             const char *passphrase, uint8_t *sig);

/**
 * @brief Return the string representation of decimal value of transaction fee in ETH.
 *
 * @param eth_unsigned_txn_ptr  The unsigned transaction containing gas_limit and gas_price
 * @param fee_decimal_string    Output decimal string of at least 30 character long
 */
void eth_get_fee_string(eth_unsigned_txn *eth_unsigned_txn_ptr, char *fee_decimal_string, uint8_t size, uint8_t decimal);

/**
 * @brief Initialize MessageData structure from protobuf
 * 
 * @param msg_data 
 */
void eth_init_msg_data(MessageData *msg_data);

/**
 * @brief Initialize Display Nodes from message data
 * 
 * @param node 
 * @param msg_data 
 */
void eth_init_display_nodes(ui_display_node **node, MessageData *msg_data);

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

void eth_sign_msg_data(const MessageData *msg_data,
                       const txn_metadata *transaction_metadata,
                       const char *mnemonics,
                       const char *passphrase,
                       uint8_t *sig);
/**
 * @brief Return the string representation of the derivation path received in transaction metadata for ethereum transaction.
 * 
 * @param txn_metadata_ptr      Pointer to transaction metadata
 * @param output                Pointer to the output string
 * @param out_len               Maximum length of output string
 */
void eth_derivation_path_to_string(const txn_metadata *txn_metadata_ptr, char *output, const size_t out_len);

/**
 * @brief Returns the decimal value of ethereum asset from metadata
 * 
 * @param txn_metadata_ptr Pointer to transaction metadata
 * @return uint8_t Decimal value of current asset
 */
uint8_t eth_get_decimal(const txn_metadata *txn_metadata_ptr);
#endif