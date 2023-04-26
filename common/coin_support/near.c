/**
 * @file    near.c
 * @author  Cypherock X1 Team
 * @brief   Near coin support.
 *          Stores near coin related class.
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
#include "near.h"

void near_byte_array_to_unsigned_txn(uint8_t *byte_array,
                                     uint16_t byte_array_size,
                                     near_unsigned_txn *utxn) {
  if (byte_array == NULL || utxn == NULL)
    return;
  memzero(utxn, sizeof(near_unsigned_txn));

  uint16_t offset = 0;
  utxn->signer_id_length = U32_READ_LE_ARRAY(byte_array);
  offset += 4;
  utxn->signer = (byte_array + offset);
  offset += utxn->signer_id_length;
  utxn->signer_key.key_type = byte_array[offset++];
  utxn->signer_key.key = (byte_array + offset);
  offset += 32;
  memcpy(utxn->nonce, byte_array + offset, sizeof(utxn->nonce));
  offset += 8;
  utxn->receiver_id_length = U32_READ_LE_ARRAY(byte_array + offset);
  offset += 4;
  utxn->receiver = (byte_array + offset);
  offset += utxn->receiver_id_length;
  utxn->blockhash = (byte_array + offset);
  offset += 32;
  utxn->action_count = U32_READ_LE_ARRAY(byte_array + offset);
  offset += 4;
  utxn->actions_type = byte_array[offset++];

  // assuming action_count as 1
  switch (utxn->actions_type) {
    case NEAR_ACTION_TRANSFER:
      memcpy(utxn->action.transfer.amount,
             byte_array + offset,
             sizeof(utxn->action.transfer.amount));
      cy_reverse_byte_array(utxn->action.transfer.amount,
                            sizeof(utxn->action.transfer.amount));
      break;

    case NEAR_ACTION_FUNCTION_CALL:
      utxn->action.fn_call.method_name_length =
          U32_READ_LE_ARRAY(byte_array + offset);
      offset += 4;
      utxn->action.fn_call.method_name = (char *)(byte_array + offset);
      offset += utxn->action.fn_call.method_name_length;
      utxn->action.fn_call.args_length = U32_READ_LE_ARRAY(byte_array + offset);
      offset += 4;
      utxn->action.fn_call.args = (byte_array + offset);
      offset += utxn->action.fn_call.args_length;
      memcpy(utxn->action.fn_call.gas,
             byte_array + offset,
             sizeof(utxn->action.fn_call.gas));
      cy_reverse_byte_array(utxn->action.fn_call.gas,
                            sizeof(utxn->action.fn_call.gas));
      offset += 8;
      memcpy(utxn->action.fn_call.deposit,
             byte_array + offset,
             sizeof(utxn->action.fn_call.deposit));
      cy_reverse_byte_array(utxn->action.fn_call.deposit,
                            sizeof(utxn->action.fn_call.deposit));
      break;

    default:
      break;
  }
  // Reverse byte order
  cy_reverse_byte_array(utxn->nonce, sizeof(utxn->nonce));
  ASSERT(offset <= byte_array_size);
}

void near_sig_unsigned_byte_array(const uint8_t *unsigned_txn_byte_array,
                                  uint64_t unsigned_txn_len,
                                  const txn_metadata *transaction_metadata,
                                  const char *mnemonics,
                                  const char *passphrase,
                                  uint8_t *sig) {
  uint8_t digest[32] = {0};
  uint32_t path[] = {
      BYTE_ARRAY_TO_UINT32(transaction_metadata->purpose_index),
      BYTE_ARRAY_TO_UINT32(transaction_metadata->coin_index),
      BYTE_ARRAY_TO_UINT32(transaction_metadata->account_index),
      BYTE_ARRAY_TO_UINT32(transaction_metadata->input[0].change_index),
      BYTE_ARRAY_TO_UINT32(transaction_metadata->input[0].address_index)};
  uint8_t seed[64] = {0};
  HDNode hdnode;
  mnemonic_to_seed(mnemonics, passphrase, seed, NULL);
  sha256_Raw(unsigned_txn_byte_array, unsigned_txn_len, digest);
  derive_hdnode_from_path(path, 5, ED25519_NAME, seed, &hdnode);
  ed25519_sign(digest, 32, hdnode.private_key, hdnode.public_key + 1, sig);
  memzero(digest, sizeof(digest));
  memzero(path, sizeof(path));
  memzero(seed, sizeof(seed));
  memzero(&hdnode, sizeof(hdnode));
}

size_t near_get_new_account_id_from_fn_args(const char *args,
                                            uint32_t args_len,
                                            char *account_id) {
  const int start = 19;    // length of '{"new_account_id":"'
  const int end =
      args_len - 74;    // length of '","new_public_key":"ed25519:..."}'
  memcpy(account_id, args + start, end - start);
  return end - start;
}

void near_serialize_account_ids(const char **account_ids,
                                const size_t count,
                                uint8_t *data,
                                uint16_t *data_len) {
  *data_len = 0;
  data[(*data_len)++] = TAG_NEAR_DEFAULT_NETWORK;
  *data_len += 2;    // Leave 2 bytes for storing data length
  for (size_t i = 0; i < count; i++) {
    fill_flash_tlv(data,
                   data_len,
                   TAG_NEAR_REGISTERED_ACC,
                   strnlen(account_ids[i], NEAR_ACC_ID_MAX_LEN) + 1,
                   (const uint8_t *)account_ids[i]);
  }
  data[1] = (*data_len - 3);
  data[2] = (*data_len - 3) >> 8;
}

void near_deserialize_account_ids(const uint8_t *data,
                                  const uint16_t data_len,
                                  char **account_ids,
                                  size_t count) {
  uint16_t offset = 3;

  for (size_t i = 0; i < count; i++) {
    if (data[offset++] != TAG_NEAR_REGISTERED_ACC)
      return;

    uint16_t acc_id_len = U16_READ_LE_ARRAY(data + offset);
    offset += 2;

    account_ids[i] = (char *)(data + offset);
    offset += acc_id_len;

    if (offset > data_len)
      return;
  }
}

size_t near_get_account_ids_count(const uint8_t *data,
                                  const uint16_t data_len) {
  uint16_t offset = 3;
  size_t count = 0;
  while (offset < data_len) {
    if (data[offset++] != TAG_NEAR_REGISTERED_ACC)
      return count;

    uint16_t acc_id_len = 0;
    memcpy(&acc_id_len, data + offset, sizeof(acc_id_len));
    offset += 2;
    offset += acc_id_len;
    count++;
  }
  return count;
}

bool near_verify_derivation_path(const uint32_t *path, uint8_t levels) {
  bool status = false;
  if (levels < 5)
    return status;

  uint32_t purpose = path[0], coin = path[1], account = path[2],
           change = path[3], address = path[4];

  // m/44'/397'/0'/0'/i'
  status =
      (purpose == NON_SEGWIT && coin == NEAR && account == NEAR_ACCOUNT_INDEX &&
       change == NEAR_CHANGE_INDEX && is_hardened(address));

  return status;
}

uint8_t near_get_decimal() {
  return NEAR_DECIMAL;
}