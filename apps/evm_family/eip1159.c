/**
 * @file    eip1159.c
 * @author  Cypherock X1 Team
 * @brief   EVM eip1559 transactions
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

#include "eip1159.h"

#include "eth.h"
#include "evm_helpers.h"

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

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool evm_parse_eip2930(const uint8_t *data,
                       size_t data_size,
                       evm_txn_context_t *txn_context) {
  if (data == NULL || txn_context == NULL) {
    return false;
  }
  evm_unsigned_txn *utxn_ptr = &txn_context->transaction_info;
  memzero(utxn_ptr, sizeof(evm_unsigned_txn));

  seq_type type = NONE;
  int64_t offset = 0;
  uint64_t decoded_len = 0;
  uint64_t item_bytes_len = 0;

  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != LIST)
    return false;

  // chain-id
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->chain_id_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->chain_id, data, data_size, item_bytes_len, &offset);

  // nonce
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->nonce_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->nonce, data, data_size, item_bytes_len, &offset);

  // gasPrice
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->gas_price_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->gas_price, data, data_size, item_bytes_len, &offset);

  // gasLimit
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->gas_limit_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->gas_limit, data, data_size, item_bytes_len, &offset);

  // to
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  s_memcpy(utxn_ptr->to_address, data, data_size, item_bytes_len, &offset);

  // value
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->value_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->value, data, data_size, item_bytes_len, &offset);

  // data
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->data_size = item_bytes_len;
  utxn_ptr->data = &data[offset];
  offset += (int64_t)item_bytes_len;

  // accessList
  // access list is not important to user; don't store it
  // we don't know any validation rules; leave that for on-chain
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += (decoded_len + item_bytes_len);
  if (type != LIST)
    return false;

  return (offset == data_size);
}

bool evm_parse_eip1559(const uint8_t *data,
                       size_t data_size,
                       evm_txn_context_t *txn_context) {
  if (data == NULL || txn_context == NULL) {
    return false;
  }
  evm_unsigned_txn *utxn_ptr = &txn_context->transaction_info;
  memzero(utxn_ptr, sizeof(evm_unsigned_txn));

  seq_type type = NONE;
  int64_t offset = 0;
  uint64_t decoded_len = 0;
  uint64_t item_bytes_len = 0;

  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != LIST)
    return false;

  // chain-id
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->chain_id_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->chain_id, data, data_size, item_bytes_len, &offset);

  // nonce
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->nonce_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->nonce, data, data_size, item_bytes_len, &offset);

  // max_priority_fee_per_gas
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->max_priority_fee_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(
      utxn_ptr->max_priority_fee, data, data_size, item_bytes_len, &offset);

  // max_fee_per_gas
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->gas_price_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->gas_price, data, data_size, item_bytes_len, &offset);

  // gasLimit
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->gas_limit_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->gas_limit, data, data_size, item_bytes_len, &offset);

  // to
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  s_memcpy(utxn_ptr->to_address, data, data_size, item_bytes_len, &offset);

  // value
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->value_size[0] = CY_MAX(1, item_bytes_len);
  s_memcpy(utxn_ptr->value, data, data_size, item_bytes_len, &offset);

  // data
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += decoded_len;
  if (type != STRING)
    return false;
  utxn_ptr->data_size = item_bytes_len;
  utxn_ptr->data = &data[offset];
  offset += (int64_t)item_bytes_len;

  // accessList
  // access list is not important to user; don't store it
  // we don't know any validation rules; leave that for on-chain
  item_bytes_len =
      get_decode_length(data + offset, data_size - offset, &decoded_len, &type);
  offset += (decoded_len + item_bytes_len);
  if (type != LIST)
    return false;

  return (offset == data_size);
}
