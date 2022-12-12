/**
 * @file    eth.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *  
 *  
 * "Commons Clause" License Condition v1.0
 *  
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *  
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *  
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *  
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */
#include "eth.h"
#include "../protocol_buffers/pb_decode.h"
#include "assert_conf.h"
#include "contracts.h"
#include "eth_sign_data/decoder.h"
#include "int-util.h"
#include "logger.h"
#include "utils.h"

static uint8_t rlp_encode_decimal(uint64_t dec, uint8_t offset, uint8_t *metadata);

/**
 * @brief
 * @details
 *
 * @param [in] ch
 *
 * @return return index of hex-chars
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint64_t get_index_of_signs(char ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }
  if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  }
  if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  }
  return -1;
}

/**
 * @brief Get the RLP decoded data, length, type.
 * @details
 *
 * @param [in] seq          Sequence of bytes of RLP encoded data.
 * @param [in] seq_len      Length of RLP decoded data.
 * @param [out] decoded_len  Size of length.
 * @param [out] type         Type of data (LIST or STRING)
 *
 * @return Length of bytes after length.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint64_t get_decode_length(const uint8_t *seq, const uint64_t seq_len, uint64_t *decoded_len, seq_type *type) {
  uint8_t first_byte      = *seq;
  uint64_t item_bytes_len = 0;
  if (first_byte <= 0x7f) {
    item_bytes_len = 1;
    *type          = STRING;
    *decoded_len   = 0;
  } else if (first_byte <= 0xb7 && seq_len > (first_byte - 0x80)) {
    item_bytes_len = first_byte - 0x80;
    *type          = STRING;
    *decoded_len   = 1;
  } else if (first_byte <= 0xbf && seq_len > (first_byte - 0xb7)) {
    uint8_t len = first_byte - 0xb7;
    uint8_t buffer_len[len];
    char hex_len[len * 2 + 1];
    hex_len[len * 2] = '\0';
    *decoded_len     = 1;
    memcpy(buffer_len, seq + *decoded_len, len);
    *decoded_len += len;
    byte_array_to_hex_string(buffer_len, len, hex_len, sizeof(hex_len));
    item_bytes_len = hex2dec(hex_len);
    *type          = STRING;
  } else if (first_byte <= 0xf7 && seq_len > (first_byte - 0xc0)) {
    item_bytes_len = first_byte - 0xc0;
    *type          = LIST;
    *decoded_len   = 1;
  } else if (first_byte <= 0xff && seq_len > (first_byte - 0xf7)) {
    uint8_t len = first_byte - 0xf7;
    uint8_t buffer_len[len];
    char hex_len[len * 2 + 1];
    hex_len[len * 2] = '\0';

    *decoded_len = 1;
    memcpy(buffer_len, seq + *decoded_len, len);
    *decoded_len += len;
    byte_array_to_hex_string(buffer_len, len, hex_len, sizeof(hex_len));
    item_bytes_len = hex2dec(hex_len);
    *type          = LIST;
  } else {
    // Intentionally unimplemented...
  }

  return item_bytes_len;
}

/**
 * @brief Converts byte array to RLP encoded byte array.
 * @details
 *
 * @param [in] byte Byte array to convert.
 * @param [in] len Length of byte array.
 * @param [in] type Type of data (STRING or LIST)
 * @param [out] encoded_byte Converted byte array.
 *
 * @return Size of length.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint64_t rlp_encode_byte(const uint8_t *byte, const uint64_t len, const seq_type type, uint8_t *encoded_byte) {
  uint64_t meta_size = 0;

  if (type == STRING) {
    if (len == 1 && byte[0] < 0x80) {
      if (encoded_byte)
        encoded_byte[0] = byte[0];
    } else if ((len + 0x80) < 0xb8) {
      if (encoded_byte)
        encoded_byte[0] = 0x80 + len;
      meta_size = 1;
    } else {
      meta_size = rlp_encode_decimal(len, 0xb7, encoded_byte);
    }
  } else if (type == LIST) {
    if ((len + 0xc0) < 0xf8) {
      if (encoded_byte)
        encoded_byte[0] = 0xc0 + len;
      meta_size = 1;
    } else {
      meta_size = rlp_encode_decimal(len, 0xf7, encoded_byte);
    }
  }

  if (byte && encoded_byte)
    memcpy(&encoded_byte[meta_size], byte, len);
  return meta_size;
}

/**
 * @brief Converts decimal to RLP encoded byte array.
 * @details
 *
 * @param [] dec        Decimal value.
 * @param [] offset     Offset for the metadata array.
 * @param [] metadata   Byte array to store the converted decimal.
 *
 * @return Size of converted encoded decimal.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint8_t rlp_encode_decimal(const uint64_t dec, const uint8_t offset, uint8_t *metadata) {
  uint32_t meta_size = 0;
  uint8_t len_hex[8];
  uint64_t s = dec_to_hex(dec, len_hex, 8);

  if (offset == 0) {
    meta_size = rlp_encode_byte(len_hex, s, STRING, metadata);
  } else {
    if (metadata) {
      metadata[0] = offset + s;
      memcpy(&metadata[1], len_hex + 8 - s, s);
    }
    meta_size = 1 + s;
  }
  return meta_size;
}

uint64_t bendian_byte_to_dec(const uint8_t *bytes, uint8_t len) {
  uint64_t result = 0;
  uint8_t i       = 0;
  while (i < len) {
    int shift     = 8 * (len - i - 1);
    uint64_t byte = bytes[i++];
    result        = result | (byte << shift);
  }
  return result;
}

uint64_t hex2dec(const char *source) {
  uint64_t sum = 0;
  uint64_t t   = 1;
  int len      = 0;

  len = strnlen(source, 16);
  for (int i = len - 1; i >= 0; i--) {
    uint64_t j = get_index_of_signs(*(source + i));
    sum += (t * j);
    t *= 16;
  }

  return sum;
}

void eth_get_to_address(const eth_unsigned_txn *eth_unsigned_txn_ptr, uint8_t *address) {
  if (eth_unsigned_txn_ptr->payload_size)
    memcpy(address, eth_unsigned_txn_ptr->payload + 16, sizeof(eth_unsigned_txn_ptr->to_address));
  else
    memcpy(address, eth_unsigned_txn_ptr->to_address, sizeof(eth_unsigned_txn_ptr->to_address));
}

uint32_t eth_get_value(const eth_unsigned_txn *eth_unsigned_txn_ptr, char *value) {
  if (eth_unsigned_txn_ptr->payload_size) {
    byte_array_to_hex_string(eth_unsigned_txn_ptr->payload + EVM_FUNC_SIGNATURE_LENGTH + EVM_FUNC_PARAM_BLOCK_LENGTH,
                             EVM_FUNC_PARAM_BLOCK_LENGTH, value, 2 * EVM_FUNC_PARAM_BLOCK_LENGTH + 1);
    return 2 * EVM_FUNC_PARAM_BLOCK_LENGTH;
  } else {
    byte_array_to_hex_string(eth_unsigned_txn_ptr->value, eth_unsigned_txn_ptr->value_size[0], value,
                             2 * eth_unsigned_txn_ptr->value_size[0] + 1);
    return 2 * eth_unsigned_txn_ptr->value_size[0];
  }
}

bool eth_validate_unsigned_txn(eth_unsigned_txn *eth_utxn_ptr, txn_metadata *metadata_ptr) {
  eth_utxn_ptr->contract_verified = 1;
  if (eth_utxn_ptr->chain_id_size[0] == 0 || eth_utxn_ptr->nonce_size[0] == 0)
    return false;
  if (is_zero(eth_utxn_ptr->value, eth_utxn_ptr->value_size[0]) && eth_utxn_ptr->payload_size == 0)
    return false;
  if (is_zero(eth_utxn_ptr->gas_limit, eth_utxn_ptr->gas_limit_size[0]))
    return false;
  if (is_zero(eth_utxn_ptr->gas_price, eth_utxn_ptr->gas_price_size[0]))
    return false;
  if (memcmp(eth_utxn_ptr->chain_id, &metadata_ptr->network_chain_id, eth_utxn_ptr->chain_id_size[0]) != 0)
    return false;

  // Check if payload exists, it's a token transfer operation on EVM
  if (eth_utxn_ptr->payload_size > 0 && U32_READ_BE_ARRAY(eth_utxn_ptr->payload) == TRANSFER_FUNC_SIGNATURE) {
    eth_utxn_ptr->contract_verified = 0;
    for (int16_t i = 0; i < WHITELISTED_CONTRACTS_COUNT; i++) {
      // Verify the contract addresses of whitelisted token symbol
      if (strncmp(metadata_ptr->token_name, whitelisted_contracts[i].symbol, ETHEREUM_TOKEN_SYMBOL_LENGTH) == 0) {
        eth_utxn_ptr->contract_verified = 1;
        return (memcmp(eth_utxn_ptr->to_address, whitelisted_contracts[i].address, ETHEREUM_ADDRESS_LENGTH) == 0);
      }
    }
  }

  return true;
}

bool is_token_whitelisted(txn_metadata *metadata_ptr) {
  if (strnlen(metadata_ptr->token_name, ETHEREUM_TOKEN_SYMBOL_LENGTH) > 0) {
    if (strncmp(metadata_ptr->token_name, ETHEREUM_TOKEN_SYMBOL, ETHEREUM_TOKEN_SYMBOL_LENGTH) == 0) {
      return true;
    }
    for (int16_t i = 0; i < WHITELISTED_CONTRACTS_COUNT; i++) {
      if (strncmp(metadata_ptr->token_name, whitelisted_contracts[i].symbol, ETHEREUM_TOKEN_SYMBOL_LENGTH) == 0) {
        return true;
      }
    }
    return false;
  }
  return true;
}

int eth_byte_array_to_msg(const uint8_t *eth_msg, size_t byte_array_len, MessageData *msg_data) {
  pb_istream_t stream = pb_istream_from_buffer(eth_msg, byte_array_len);

  bool status = pb_decode(&stream, MessageData_fields, msg_data);

  if (!status) {
    return -1;
  }

  return 0;
}

void eth_init_msg_data(MessageData *msg_data) {
  MessageData zero = MessageData_init_zero;
  memcpy(msg_data, &zero, sizeof(zero));

  msg_data->data_bytes.funcs.decode = read_data_bytes;
}

int eth_byte_array_to_unsigned_txn(const uint8_t *eth_unsigned_txn_byte_array,
                                   size_t byte_array_len,
                                   eth_unsigned_txn *unsigned_txn_ptr) {
  seq_type type        = NONE;
  int64_t offset       = 0;
  uint64_t decoded_len = 0;
  uint64_t item_bytes_len;

  item_bytes_len =
      get_decode_length(eth_unsigned_txn_byte_array + offset, byte_array_len - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != LIST)
    return -1;

  // nonce
  item_bytes_len =
      get_decode_length(eth_unsigned_txn_byte_array + offset, byte_array_len - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  unsigned_txn_ptr->nonce_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(unsigned_txn_ptr->nonce, eth_unsigned_txn_byte_array, byte_array_len, item_bytes_len, &offset);

  // gas price
  item_bytes_len =
      get_decode_length(eth_unsigned_txn_byte_array + offset, byte_array_len - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  unsigned_txn_ptr->gas_price_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(unsigned_txn_ptr->gas_price, eth_unsigned_txn_byte_array, byte_array_len, item_bytes_len, &offset);

  // gas limit
  item_bytes_len =
      get_decode_length(eth_unsigned_txn_byte_array + offset, byte_array_len - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  unsigned_txn_ptr->gas_limit_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(unsigned_txn_ptr->gas_limit, eth_unsigned_txn_byte_array, byte_array_len, item_bytes_len, &offset);

  // to address
  item_bytes_len =
      get_decode_length(eth_unsigned_txn_byte_array + offset, byte_array_len - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  s_memcpy(unsigned_txn_ptr->to_address, eth_unsigned_txn_byte_array, byte_array_len, item_bytes_len, &offset);

  // value
  item_bytes_len =
      get_decode_length(eth_unsigned_txn_byte_array + offset, byte_array_len - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  unsigned_txn_ptr->value_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(unsigned_txn_ptr->value, eth_unsigned_txn_byte_array, byte_array_len, item_bytes_len, &offset);

  // payload
  item_bytes_len =
      get_decode_length(eth_unsigned_txn_byte_array + offset, byte_array_len - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  unsigned_txn_ptr->payload_size = item_bytes_len;
  unsigned_txn_ptr->payload      = (uint8_t *)cy_malloc(item_bytes_len * sizeof(uint8_t));
  s_memcpy(unsigned_txn_ptr->payload, eth_unsigned_txn_byte_array, byte_array_len, item_bytes_len, &offset);

  // chain id
  item_bytes_len =
      get_decode_length(eth_unsigned_txn_byte_array + offset, byte_array_len - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  unsigned_txn_ptr->chain_id_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(unsigned_txn_ptr->chain_id, eth_unsigned_txn_byte_array, byte_array_len, item_bytes_len, &offset);
  return (offset > 0 ? 0 : -1);
}

void sig_unsigned_byte_array(const uint8_t *eth_unsigned_txn_byte_array,
                             uint64_t eth_unsigned_txn_len,
                             const txn_metadata *transaction_metadata,
                             const char *mnemonics,
                             const char *passphrase,
                             uint8_t *sig) {
  uint8_t digest[32];
  keccak_256(eth_unsigned_txn_byte_array, eth_unsigned_txn_len, digest);
  uint8_t recid;
  HDNode hdnode;
  get_address_node(transaction_metadata, 0, mnemonics, passphrase, &hdnode);
  ecdsa_sign_digest(&secp256k1, hdnode.private_key, digest, sig, &recid, NULL);
  memcpy(sig + 64, &recid, 1);
}

void eth_get_fee_string(eth_unsigned_txn *eth_unsigned_txn_ptr, char *fee_decimal_string, uint8_t size) {
  uint8_t fee[16] = {0};
  uint64_t txn_fee, carry;
  char fee_hex_string[33] = {'\0'};

  // make sure we do not process over the current capacity (i.e., 8-byte limit for gas limit and price each)
  ASSERT(eth_unsigned_txn_ptr->gas_price_size[0] <= 8 && eth_unsigned_txn_ptr->gas_limit_size[0] <= 8);
  // Capacity to multiply 2 numbers upto 8-byte value and store the result in 2 separate 8-byte variables
  txn_fee =
      mul128(bendian_byte_to_dec(eth_unsigned_txn_ptr->gas_price, eth_unsigned_txn_ptr->gas_price_size[0]),
             bendian_byte_to_dec(eth_unsigned_txn_ptr->gas_limit, eth_unsigned_txn_ptr->gas_limit_size[0]), &carry);
  // prepare the whole 128-bit little-endian representation of fee
  memcpy(fee, &txn_fee, sizeof(txn_fee));
  memcpy(fee + sizeof(txn_fee), &carry, sizeof(carry));
  cy_reverse_byte_array(fee, sizeof(fee));  // outputs 128-bit (16-byte) big-endian representation of fee
  byte_array_to_hex_string(fee, sizeof(fee), fee_hex_string, sizeof(fee_hex_string));
  convert_byte_array_to_decimal_string(sizeof(fee_hex_string) - 1, 18, fee_hex_string, fee_decimal_string, size);
}