/**
 * @file    evm_txn_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Helper implementation for interpreting and signing EVM
 *          transactions
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
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

#include "evm_txn_helpers.h"

#include "evm_priv.h"
#include "int-util.h"

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
 */
static uint64_t get_decode_length(const uint8_t *seq,
                                  const uint64_t seq_len,
                                  uint64_t *decoded_len,
                                  seq_type *type);

/**
 * @brief The function identifies the payload type in the provided transaction
 *
 * @param eth_utxn_ptr Reference to const instance of evm_unsigned_txn
 * @param metadata_ptr Reference to txn_metadata
 * @return PAYLOAD_STATUS indicating the identified category of payload in the
 * transaction
 */
static PAYLOAD_STATUS evm_decode_txn_payload(
    const evm_sign_txn_initiate_request_t *init_info,
    const evm_unsigned_txn *utxn_ptr,
    const erc20_contracts_t **contract);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

bool evm_is_token_whitelisted = false;

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static uint64_t get_decode_length(const uint8_t *seq,
                                  const uint64_t seq_len,
                                  uint64_t *decoded_len,
                                  seq_type *type) {
  uint8_t first_byte = *seq;
  uint64_t item_bytes_len = 0;
  if (first_byte <= 0x7f) {
    item_bytes_len = 1;
    *type = STRING;
    *decoded_len = 0;
  } else if (first_byte <= 0xb7 && seq_len > (first_byte - 0x80)) {
    item_bytes_len = first_byte - 0x80;
    *type = STRING;
    *decoded_len = 1;
  } else if (first_byte <= 0xbf && seq_len > (first_byte - 0xb7)) {
    uint8_t len = first_byte - 0xb7;
    uint8_t buffer_len[len];
    char hex_len[len * 2 + 1];
    hex_len[len * 2] = '\0';
    *decoded_len = 1;
    memcpy(buffer_len, seq + *decoded_len, len);
    *decoded_len += len;
    byte_array_to_hex_string(buffer_len, len, hex_len, sizeof(hex_len));
    item_bytes_len = hex2dec(hex_len);
    *type = STRING;
  } else if (first_byte <= 0xf7 && seq_len > (first_byte - 0xc0)) {
    item_bytes_len = first_byte - 0xc0;
    *type = LIST;
    *decoded_len = 1;
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
    *type = LIST;
  } else {
    // Intentionally unimplemented...
  }

  return item_bytes_len;
}

static PAYLOAD_STATUS evm_decode_txn_payload(
    const evm_sign_txn_initiate_request_t *init_info,
    const evm_unsigned_txn *utxn_ptr,
    const erc20_contracts_t **contract) {
  PAYLOAD_STATUS result = PAYLOAD_ABSENT;
  evm_is_token_whitelisted = false;

  if (utxn_ptr->payload_size > 0) {
    if ((utxn_ptr->payload_size >= 4) &&
        (U32_READ_BE_ARRAY(utxn_ptr->payload) == TRANSFER_FUNC_SIGNATURE) &&
        (init_info->is_token_transfer) &&
        (init_info->chain_id == ETHEREUM_MAINNET_CHAIN)) {
      for (int16_t i = 0; i < g_evm_app->whitelist_count; i++) {
        if (strncmp(init_info->token_symbol,
                    g_evm_app->whitelisted_contracts[i].symbol,
                    ETHEREUM_TOKEN_SYMBOL_LENGTH) == 0) {
          evm_is_token_whitelisted = true;
          result = (memcmp(utxn_ptr->to_address,
                           g_evm_app->whitelisted_contracts[i].address,
                           EVM_ADDRESS_LENGTH) == 0)
                       ? PAYLOAD_WHITELISTED
                       : PAYLOAD_CONTRACT_INVALID;

          if (PAYLOAD_WHITELISTED == result) {
            *contract = &g_evm_app->whitelisted_contracts[i];
          }
          break;
        }
      }
    }
    if (!evm_is_token_whitelisted) {
      // TODO: detecting clear sign or blind sign transaction using the function
      // tag value along with function signature
      result =
          (ETH_ExtractArguments(utxn_ptr->payload, utxn_ptr->payload_size) ==
           ETH_UTXN_ABI_DECODE_OK)
              ? PAYLOAD_WHITELISTED
              : PAYLOAD_SIGNATURE_NOT_WHITELISTED;
    }
  }
  return result;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

int evm_byte_array_to_unsigned_txn(const uint8_t *evm_utxn_byte_array,
                                   size_t byte_array_len,
                                   evm_txn_context_t *txn_context) {
  if (evm_utxn_byte_array == NULL || txn_context == NULL) {
    return -1;
  }
  evm_unsigned_txn *utxn_ptr = &txn_context->transaction_info;
  memzero(utxn_ptr, sizeof(evm_unsigned_txn));

  seq_type type = NONE;
  int64_t offset = 0;
  uint64_t decoded_len = 0;
  uint64_t item_bytes_len = 0;

  item_bytes_len = get_decode_length(evm_utxn_byte_array + offset,
                                     byte_array_len - offset,
                                     &decoded_len,
                                     &type);
  offset += decoded_len;
  if (type != LIST)
    return -1;

  // nonce
  item_bytes_len = get_decode_length(evm_utxn_byte_array + offset,
                                     byte_array_len - offset,
                                     &decoded_len,
                                     &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  utxn_ptr->nonce_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->nonce,
           evm_utxn_byte_array,
           byte_array_len,
           item_bytes_len,
           &offset);

  // gas price
  item_bytes_len = get_decode_length(evm_utxn_byte_array + offset,
                                     byte_array_len - offset,
                                     &decoded_len,
                                     &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  utxn_ptr->gas_price_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->gas_price,
           evm_utxn_byte_array,
           byte_array_len,
           item_bytes_len,
           &offset);

  // gas limit
  item_bytes_len = get_decode_length(evm_utxn_byte_array + offset,
                                     byte_array_len - offset,
                                     &decoded_len,
                                     &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  utxn_ptr->gas_limit_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->gas_limit,
           evm_utxn_byte_array,
           byte_array_len,
           item_bytes_len,
           &offset);

  // to address
  item_bytes_len = get_decode_length(evm_utxn_byte_array + offset,
                                     byte_array_len - offset,
                                     &decoded_len,
                                     &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  s_memcpy(utxn_ptr->to_address,
           evm_utxn_byte_array,
           byte_array_len,
           item_bytes_len,
           &offset);

  // value
  item_bytes_len = get_decode_length(evm_utxn_byte_array + offset,
                                     byte_array_len - offset,
                                     &decoded_len,
                                     &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  utxn_ptr->value_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->value,
           evm_utxn_byte_array,
           byte_array_len,
           item_bytes_len,
           &offset);

  // payload
  item_bytes_len = get_decode_length(evm_utxn_byte_array + offset,
                                     byte_array_len - offset,
                                     &decoded_len,
                                     &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  utxn_ptr->payload_size = item_bytes_len;
  utxn_ptr->payload = &evm_utxn_byte_array[offset];
  offset += (int64_t)item_bytes_len;
  utxn_ptr->payload_status = evm_decode_txn_payload(
      &txn_context->init_info, utxn_ptr, &txn_context->contract);

  // chain id
  item_bytes_len = get_decode_length(evm_utxn_byte_array + offset,
                                     byte_array_len - offset,
                                     &decoded_len,
                                     &type);
  offset += decoded_len;
  if (type != STRING)
    return -1;
  utxn_ptr->chain_id_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->chain_id,
           evm_utxn_byte_array,
           byte_array_len,
           item_bytes_len,
           &offset);
  return (offset > 0 ? 0 : -1);
}

bool evm_validate_unsigned_txn(const evm_txn_context_t *txn_context) {
  const evm_unsigned_txn *utxn_ptr = &txn_context->transaction_info;
  return !(
      // Check if the chain id size or nonce size is non-zero
      (utxn_ptr->chain_id_size[0] == 0 || utxn_ptr->nonce_size[0] == 0) ||
      // Check if the gas limit is non-zero
      (is_zero(utxn_ptr->gas_limit, utxn_ptr->gas_limit_size[0])) ||
      // Check if the gas price is non-zero
      (is_zero(utxn_ptr->gas_price, utxn_ptr->gas_price_size[0])) ||
      // Check if the chain id from app matches with the chain id from the
      // unsigned transaction
      (cy_read_be(utxn_ptr->chain_id, utxn_ptr->chain_id_size[0]) !=
       g_evm_app->chain_id) ||
      // Check if token transfer is triggered with whitelisted token and amount
      // is non-zero
      (txn_context->init_info.is_token_transfer && evm_is_token_whitelisted &&
       !is_zero(utxn_ptr->value, utxn_ptr->value_size[0])) ||
      // ensure the payload status is valid
      (utxn_ptr->payload_status == PAYLOAD_CONTRACT_INVALID));
}

void eth_get_to_address(const evm_unsigned_txn *utxn_ptr,
                        const uint8_t **address) {
  if (evm_is_token_whitelisted) {
    *address = &utxn_ptr->payload[16];
  } else {
    *address = &utxn_ptr->to_address[0];
  }
}

uint32_t eth_get_value(const evm_unsigned_txn *utxn_ptr, char *value) {
  if (evm_is_token_whitelisted) {
    byte_array_to_hex_string(utxn_ptr->payload + EVM_FUNC_SIGNATURE_LENGTH +
                                 EVM_FUNC_PARAM_BLOCK_LENGTH,
                             EVM_FUNC_PARAM_BLOCK_LENGTH,
                             value,
                             2 * EVM_FUNC_PARAM_BLOCK_LENGTH + 1);
    return 2 * EVM_FUNC_PARAM_BLOCK_LENGTH;
  } else {
    byte_array_to_hex_string(utxn_ptr->value,
                             utxn_ptr->value_size[0],
                             value,
                             2 * utxn_ptr->value_size[0] + 1);
    return 2 * utxn_ptr->value_size[0];
  }
}

void eth_get_fee_string(const evm_unsigned_txn *utxn_ptr,
                        char *fee_decimal_string,
                        uint8_t size,
                        uint8_t decimal) {
  uint8_t fee[16] = {0};
  uint64_t txn_fee, carry;
  char fee_hex_string[33] = {'\0'};

  // make sure we do not process over the current capacity (i.e., 8-byte limit
  // for gas limit and price each)
  ASSERT(utxn_ptr->gas_price_size[0] <= 8 && utxn_ptr->gas_limit_size[0] <= 8);
  // Capacity to multiply 2 numbers upto 8-byte value and store the result in 2
  // separate 8-byte variables
  txn_fee = mul128(
      bendian_byte_to_dec(utxn_ptr->gas_price, utxn_ptr->gas_price_size[0]),
      bendian_byte_to_dec(utxn_ptr->gas_limit, utxn_ptr->gas_limit_size[0]),
      &carry);
  // prepare the whole 128-bit little-endian representation of fee
  memcpy(fee, &txn_fee, sizeof(txn_fee));
  memcpy(fee + sizeof(txn_fee), &carry, sizeof(carry));
  // outputs 128-bit (16-byte) big-endian representation of fee
  cy_reverse_byte_array(fee, sizeof(fee));
  byte_array_to_hex_string(
      fee, sizeof(fee), fee_hex_string, sizeof(fee_hex_string));
  convert_byte_array_to_decimal_string(sizeof(fee_hex_string) - 1,
                                       decimal,
                                       fee_hex_string,
                                       fee_decimal_string,
                                       size);
}

uint8_t evm_get_decimal(const evm_txn_context_t *txn_context) {
  // TODO: in case
  return (PAYLOAD_WHITELISTED == txn_context->transaction_info.payload_status &&
          NULL != txn_context->contract)
             ? txn_context->contract->decimal
             : ETH_DECIMAL;
}

const char *evm_get_asset_symbol(const evm_txn_context_t *txn_context) {
  if (evm_is_token_whitelisted)
    return txn_context->init_info.token_symbol;
  else
    return g_evm_app->lunit_name;
}

const char *eth_get_address_title(const evm_unsigned_txn *utxn_ptr) {
  return (
      (utxn_ptr->payload_status != PAYLOAD_ABSENT && !evm_is_token_whitelisted)
          ? ui_text_verify_contract
          : ui_text_verify_address);
}
