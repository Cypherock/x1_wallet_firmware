/**
 * @file    eth.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
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

#include "assert_conf.h"
#include "constant_texts.h"
#include "evm_contracts.h"
#include "int-util.h"
#include "logger.h"
#include "pb_decode.h"
#include "utils.h"

extern const erc20_contracts_t *eth_contracts;

static uint8_t rlp_encode_decimal(uint64_t dec,
                                  uint8_t offset,
                                  uint8_t *metadata);

extern bool evm_is_token_whitelisted;

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
static uint64_t rlp_encode_byte(const uint8_t *byte,
                                const uint64_t len,
                                const seq_type type,
                                uint8_t *encoded_byte) {
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
static uint8_t rlp_encode_decimal(const uint64_t dec,
                                  const uint8_t offset,
                                  uint8_t *metadata) {
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
  uint8_t i = 0;
  while (i < len) {
    int shift = 8 * (len - i - 1);
    uint64_t byte = bytes[i++];
    result = result | (byte << shift);
  }
  return result;
}

uint64_t hex2dec(const char *source) {
  uint64_t sum = 0;
  uint64_t t = 1;
  int len = 0;

  len = strnlen(source, 16);
  for (int i = len - 1; i >= 0; i--) {
    uint64_t j = get_index_of_signs(*(source + i));
    sum += (t * j);
    t *= 16;
  }

  return sum;
}

void eth_sign_msg_data(const MessageData *msg_data,
                       const txn_metadata *transaction_metadata,
                       const char *mnemonics,
                       const char *passphrase,
                       uint8_t *sig) {
}

void eth_init_display_nodes(ui_display_node **node, MessageData *msg_data) {
}

int eth_byte_array_to_msg(const uint8_t *eth_msg,
                          size_t byte_array_len,
                          MessageData *msg_data) {
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
  ecdsa_sign_digest(&secp256k1, hdnode.private_key, digest, sig, &recid, NULL);
  memcpy(sig + 64, &recid, 1);
}
