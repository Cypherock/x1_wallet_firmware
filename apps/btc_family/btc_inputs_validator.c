/**
 * @author  Cypherock X1 Team
 * @brief   Bitcoin inputs validator, uses a stream to read data
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2024 by HODL TECH PTE LTD
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

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "btc_inputs_validator.h"

#include "byte_stream.h"
#include "sha2.h"
#include "utils.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/**
 * @brief Decodes a variable-length integer (VarInt) from the byte stream.
 *
 * This function reads the first byte from the stream to determine how many more
 * bytes are needed to decode the full integer. The number of bytes varies based
 * on the value of the first byte (0xfd, 0xfe, or 0xff for extended length
 * encoding).
 *
 * @param stream Pointer to the byte stream from which the VarInt is decoded.
 * @param hash_ctx Pointer to an SHA256 context for updating the hash
 * (optional).
 *
 * ref:
 * https://en.bitcoin.it/wiki/Protocol_documentation#Variable_length_integer
 *
 * @return The decoded 64-bit integer value.
 * @note If the stream is invalid, or if there's an error reading from the
 * stream, 0 is returned.
 */
static uint64_t decode_varint(byte_stream_t *stream, SHA256_CTX *hash_ctx) {
  uint8_t buffer[MAX_VARINT_SIZE] = {0};
  if (stream == NULL) {
    return 0;
  }
  uint8_t first_byte = 0;

  {
    byte_stream_status_e status = read_byte_stream(stream, buffer, 1);
    if (status != BYTE_STREAM_SUCCESS) {
      return 0;
    }
    first_byte = buffer[0];
  }

  if (first_byte < 0xfd) {
    if (hash_ctx != NULL) {
      sha256_Update(hash_ctx, buffer, 1);
    }

    return first_byte;
  }

  if (first_byte == 0xfd) {
    byte_stream_status_e status =
        read_byte_stream(stream, buffer + TX_IN_INDEX_OFFSET, 2);
    if (status != BYTE_STREAM_SUCCESS) {
      return 0;
    }

    if (hash_ctx != NULL) {
      sha256_Update(hash_ctx, buffer, 3);
    }
    return U32_READ_LE_ARRAY(&buffer[TX_IN_INDEX_OFFSET]);
  }

  if (first_byte == 0xfe) {
    byte_stream_status_e status =
        read_byte_stream(stream, buffer + TX_IN_INDEX_OFFSET, 4);
    if (status != BYTE_STREAM_SUCCESS) {
      return 0;
    }

    if (hash_ctx != NULL) {
      sha256_Update(hash_ctx, buffer, 5);
    }
    return U32_READ_LE_ARRAY(&buffer[TX_IN_INDEX_OFFSET]);
  }

  if (first_byte == 0xff) {
    byte_stream_status_e status =
        read_byte_stream(stream, buffer + TX_IN_INDEX_OFFSET, 8);
    if (status != BYTE_STREAM_SUCCESS) {
      return 0;
    }

    if (hash_ctx != NULL) {
      sha256_Update(hash_ctx, buffer, 9);
    }
    return U32_READ_LE_ARRAY(&buffer[TX_IN_INDEX_OFFSET]);
  }
  return 0;
}

btc_validation_error_e btc_validate_inputs(byte_stream_t *stream,
                                           const btc_sign_txn_input_t *input) {
  SHA256_CTX hash_ctx = {0};
  sha256_Init(&(hash_ctx));
  bool is_segwit = false;
  uint64_t found_output_value_for_input_index = 0;
  if (stream == NULL || input == NULL) {
    return BTC_VALIDATE_ERR_INVALID_PARAMS;
  }
  byte_stream_status_e status = {0};

  uint8_t version_no[VERSION_NO_SIZE] = {0};
  status = read_byte_stream(stream, version_no, sizeof(version_no));
  if (status != BYTE_STREAM_SUCCESS) {
    return BTC_VALIDATE_ERR_READ_STREAM;
  }
  sha256_Update(&hash_ctx, version_no, sizeof(version_no));

  // optional flag
  if (stream->stream_pointer[stream->offset] == 0) {
    status = skip_byte_stream(stream, 2);
    if (status != BYTE_STREAM_SUCCESS) {
      return BTC_VALIDATE_ERR_READ_STREAM;
    }
    is_segwit = true;
  }

  uint64_t in_counter = decode_varint(stream, &hash_ctx);
  while (in_counter--) {
    uint8_t prev_transaction_hash[SHA256_DIGEST_LENGTH] = {0};
    status = read_byte_stream(
        stream, prev_transaction_hash, sizeof(prev_transaction_hash));
    if (status != BYTE_STREAM_SUCCESS) {
      return BTC_VALIDATE_ERR_READ_STREAM;
    }
    sha256_Update(
        &hash_ctx, prev_transaction_hash, sizeof(prev_transaction_hash));

    uint8_t prev_tx_out_index[TX_IN_SEQ_NO_SIZE] = {0};
    status =
        read_byte_stream(stream, prev_tx_out_index, sizeof(prev_tx_out_index));
    if (status != BYTE_STREAM_SUCCESS) {
      return BTC_VALIDATE_ERR_READ_STREAM;
    }
    sha256_Update(&hash_ctx, prev_tx_out_index, sizeof(prev_tx_out_index));

    uint64_t tx_in_script_length = decode_varint(stream, &hash_ctx);
    while (tx_in_script_length > 0) {
      uint64_t length_to_parse = SLICE_SIZE;
      if (tx_in_script_length < length_to_parse) {
        length_to_parse = tx_in_script_length;
      }

      uint8_t tx_in_script_slice[SLICE_SIZE] = {0};

      status = read_byte_stream(stream, tx_in_script_slice, length_to_parse);
      if (status != BYTE_STREAM_SUCCESS) {
        return BTC_VALIDATE_ERR_READ_STREAM;
      }
      sha256_Update(&hash_ctx, tx_in_script_slice, length_to_parse);

      tx_in_script_length -= length_to_parse;
    }

    uint8_t sequence_no[TX_IN_SEQ_NO_SIZE] = {0};
    status = read_byte_stream(stream, sequence_no, sizeof(sequence_no));
    if (status != BYTE_STREAM_SUCCESS) {
      return BTC_VALIDATE_ERR_READ_STREAM;
    }
    sha256_Update(&hash_ctx, sequence_no, sizeof(sequence_no));
  }

  uint64_t out_counter = decode_varint(stream, &hash_ctx);
  // https://en.bitcoin.it/wiki/Transaction#General_format_.28inside_a_block.29_of_each_output_of_a_transaction_-_Txout
  uint64_t out_index = 0;
  while (out_index < out_counter) {
    uint8_t output_value_buffer[TX_OUT_VALUE_SIZE] = {0};
    status = read_byte_stream(
        stream, output_value_buffer, sizeof(output_value_buffer));
    if (status != BYTE_STREAM_SUCCESS) {
      return BTC_VALIDATE_ERR_READ_STREAM;
    }
    sha256_Update(&hash_ctx, output_value_buffer, sizeof(output_value_buffer));

    uint64_t output_value = U64_READ_LE_ARRAY(output_value_buffer);
    if (out_index == input->prev_output_index) {
      found_output_value_for_input_index = output_value;
    }

    uint64_t tx_out_script_length = decode_varint(stream, &hash_ctx);
    while (tx_out_script_length > 0) {
      uint64_t length_to_parse = SLICE_SIZE;
      if (tx_out_script_length < length_to_parse) {
        length_to_parse = tx_out_script_length;
      }

      uint8_t tx_out_script_slice[SLICE_SIZE] = {0};

      status = read_byte_stream(stream, tx_out_script_slice, length_to_parse);
      if (status != BYTE_STREAM_SUCCESS) {
        return BTC_VALIDATE_ERR_READ_STREAM;
      }
      sha256_Update(&hash_ctx, tx_out_script_slice, length_to_parse);

      tx_out_script_length -= length_to_parse;
    }

    out_index++;
  }

  // Optional witness data, not used in calculating hash
  // https://bitcoin.stackexchange.com/questions/113697/what-are-the-parts-of-a-bitcoin-transaction-in-segwit-format
  if (is_segwit) {
    uint64_t witness_count = decode_varint(stream, NULL);
    while (witness_count--) {
      uint64_t witness_component_length = decode_varint(stream, NULL);
      while (witness_component_length > 0) {
        uint64_t length_to_parse = SLICE_SIZE;
        if (witness_component_length < length_to_parse) {
          length_to_parse = witness_component_length;
        }

        status = skip_byte_stream(stream, length_to_parse);
        if (status != BYTE_STREAM_SUCCESS) {
          return BTC_VALIDATE_ERR_READ_STREAM;
        }

        witness_component_length -= length_to_parse;
      }
    }
  }

  uint8_t lock_time[TX_IN_SEQ_NO_SIZE] = {0};
  status = read_byte_stream(stream, lock_time, sizeof(lock_time));
  if (status != BYTE_STREAM_SUCCESS) {
    return BTC_VALIDATE_ERR_READ_STREAM;
  }
  sha256_Update(&hash_ctx, lock_time, sizeof(lock_time));

  uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
  sha256_Final(&hash_ctx, hash);
  sha256_Raw(hash, sizeof(hash), hash);

  if (memcmp(hash, input->prev_txn_hash, sizeof(input->prev_txn_hash)) != 0) {
    return BTC_VALIDATE_ERR_INVALID_TX_HASH;
  }

  if (found_output_value_for_input_index != input->value) {
    return BTC_VALIDATE_ERR_INVALID_OUTPUT_VALUE;
  }

  return BTC_VALIDATE_SUCCESS;
}
