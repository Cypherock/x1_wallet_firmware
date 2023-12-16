/**
 * @file    solana.c
 * @author  Cypherock X1 Team
 * @brief   Solana coin support.
 *          Stores solana coin related class.
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

#include "solana.h"

size_t sol_get_derivation_depth(const uint16_t tag) {
  switch (tag) {
    case SOL_ACC_TYPE1:
      return 2;
    case SOL_ACC_TYPE2:
      return 3;
    case SOL_ACC_TYPE3:
      return 4;
    default:
      return 3;
  }
}

void solana_sig_unsigned_byte_array(const uint8_t *unsigned_txn_byte_array,
                                    uint64_t unsigned_txn_len,
                                    const txn_metadata *transaction_metadata,
                                    const char *mnemonics,
                                    const char *passphrase,
                                    uint8_t *sig) {
  uint32_t path[] = {
      BYTE_ARRAY_TO_UINT32(transaction_metadata->purpose_index),
      BYTE_ARRAY_TO_UINT32(transaction_metadata->coin_index),
      BYTE_ARRAY_TO_UINT32(transaction_metadata->account_index),
      BYTE_ARRAY_TO_UINT32(transaction_metadata->input[0].change_index),
      BYTE_ARRAY_TO_UINT32(transaction_metadata->input[0].address_index)};
  size_t depth = sol_get_derivation_depth(transaction_metadata->address_tag);
  uint8_t seed[64] = {0};
  HDNode hdnode;
  mnemonic_to_seed(mnemonics, passphrase, seed, NULL);
  derive_hdnode_from_path(path, depth, ED25519_NAME, seed, &hdnode);

  ed25519_sign(unsigned_txn_byte_array,
               unsigned_txn_len,
               hdnode.private_key,
               hdnode.public_key + 1,
               sig);
  memzero(path, sizeof(path));
  memzero(seed, sizeof(seed));
  memzero(&hdnode, sizeof(hdnode));
}

int solana_update_blockhash_in_byte_array(uint8_t *byte_array,
                                          const uint8_t *blockhash) {
  uint8_t empty_array[SOLANA_BLOCKHASH_LENGTH] = {0};
  uint16_t offset = 0;
  int status = SOL_OK;
  uint16_t addresses_count = 0;

  if (byte_array == NULL || blockhash == NULL)
    return SOL_ERROR;

  if (memcmp(blockhash, empty_array, SOLANA_BLOCKHASH_LENGTH) == 0)
    return SOL_BU_INVALID_BLOCKHASH;
  // Message headers
  offset += 3;
  // Account addresses
  offset +=
      get_compact_array_size(byte_array + offset, &addresses_count, &status);
  if (status != SOL_OK)
    return status;
  if (addresses_count == 0)
    return SOL_D_MIN_LENGTH;
  offset += addresses_count * SOLANA_ACCOUNT_ADDRESS_LENGTH;
  // Blockhash
  memcpy(byte_array + offset, blockhash, SOLANA_BLOCKHASH_LENGTH);
  return SOL_OK;
}

bool sol_verify_derivation_path(const uint32_t *path, uint8_t levels) {
  bool status = false;
  if (levels < 2)
    return status;

  uint32_t purpose = path[0], coin = path[1];

  switch (levels) {
    case 2:    // m/44'/501'
      status = (purpose == NON_SEGWIT && coin == SOLANA);
      break;

    case 3: {    // m/44'/501'/i'
      uint32_t account = path[2];
      status =
          (purpose == NON_SEGWIT && coin == SOLANA && is_hardened(account));
    } break;

    case 4: {    // m/44'/501'/i'/0'
      uint32_t change = path[3];
      uint32_t account = path[2];
      status = (purpose == NON_SEGWIT && coin == SOLANA &&
                is_hardened(account) && change == SOLANA_CHANGE_INDEX);
    } break;

    default:
      break;
  }

  return status;
}
