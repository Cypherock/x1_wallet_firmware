/**
 * @file    eth.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
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
#include "abi.h"
#include "coin_utils.h"
#include "eip712.pb.h"
#include "evm_txn_helpers.h"

#define ETHEREUM_MAINNET_CHAIN 1

#define ETHEREUM_MAINNET_NAME "Ethereum"
#define ETHEREUM_TOKEN_SYMBOL "ETH"
// \x45(E) is needed otherwise \x19e is considered instead of \x19
#define ETH_PERSONAL_SIGN_IDENTIFIER "\x19\x45thereum Signed Message:\n"
#define ETH_SIGN_TYPED_DATA_IDENTIFIER "\x19\x01"

#define ETH_NONCE_SIZE_BYTES (32U)
#define ETH_GWEI_INDEX (9U)
/// Ref: https://ethereum.org/en/developers/docs/intro-to-ether/#denominations
#define ETH_DECIMAL (18U)

#define ETH_COIN_VERSION 0x00000000

/// Enum used to differentiate between a single val, string of bytes and list of
/// strings during rlp decoding/encoding in raw eth byte array
typedef enum { NONE, STRING, LIST } seq_type;

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
 * @brief Convert byte array representation of message to an object using
 * protobuf.
 * @details
 *
 * @param [in] eth_msg                      Byte array of message.
 * @param [in] byte_array_len               Length of byte array.
 * @param [out] msg_data                    Pointer to the MessageData instance
 * to store the message details.
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
int eth_byte_array_to_msg(const uint8_t *eth_msg,
                          size_t byte_array_len,
                          MessageData *msg_data);

/**
 * @brief Signed unsigned byte array.
 * @details
 *
 * @param [in] eth_unsigned_txn_byte_array  Byte array of unsigned transaction.
 * @param [in] eth_unsigned_txn_len         length of unsigned transaction byte
 * array.
 * @param [in] transaction_metadata         Pointer to txn_metadata instance.
 * @param [in] mnemonics                    char array of mnemonics.
 * @param [in] passphrase                   char array of passphrase.
 * @param [out] sig                         Byte array of signature to store the
 * result of signing unsigned transaction byte array.
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
 * @brief Initialize MessageData structure from protobuf
 *
 * @param msg_data
 */
void eth_init_msg_data(MessageData *msg_data);

void eth_sign_msg_data(const MessageData *msg_data,
                       const txn_metadata *transaction_metadata,
                       const char *mnemonics,
                       const char *passphrase,
                       uint8_t *sig);
#endif