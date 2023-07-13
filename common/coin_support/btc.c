/**
 * @file    btc.c
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
#include "btc.h"

#include "assert_conf.h"
#include "base58.h"
#include "logger.h"
#include "segwit_addr.h"
#include "sha2.h"
#include "string.h"
#include "utils.h"

int get_address(const char *hrp,
                const uint8_t *script_pub_key,
                uint8_t version,
                char *address_output) {
  if (script_pub_key == NULL || address_output == NULL)
    return 0;

  if (script_pub_key[0] == 0) {
    // Segwit address
    return segwit_addr_encode(
        address_output, hrp, 0x00, script_pub_key + 2, script_pub_key[1]);
  }

  uint8_t address[SHA3_256_DIGEST_LENGTH] = {0};
  uint8_t offset = 1, script_offset = 0;

  // refer https://learnmeabitcoin.com/technical/script for script type
  // explaination
  if (script_pub_key[0] == 0x41) {
    // hash160 P2PK
    hasher_Raw(HASHER_SHA2_RIPEMD,
               script_pub_key + 1,
               65,
               address + offset);    // overwrite with RIPEMD160
    offset += RIPEMD160_DIGEST_LENGTH;
  } else if (script_pub_key[0] == 0x76) {
    script_offset = 3;    // P2PKH
  } else if (script_pub_key[0] == 0xa9) {
    script_offset = 2;    // P2SH upto 15 pub keys
    version = 5;          // Version for P2SH: 5
  } else if (script_pub_key[1] > 0x50 && script_pub_key[1] <= 0x53)
    return -1;    // P2MS upto 3 pub keys
  else
    return -2;    // Unknown script type

  if (script_pub_key[0] != 0x41) {
    memcpy(address + offset,
           script_pub_key + script_offset,
           RIPEMD160_DIGEST_LENGTH);
    offset += RIPEMD160_DIGEST_LENGTH;
  }
  address[0] = version;

  return base58_encode_check(address, offset, HASHER_SHA2D, address_output, 35);
}

bool validate_change_address(const unsigned_txn *utxn_ptr,
                             const txn_metadata *txn_metadata_ptr,
                             const char *mnemonic,
                             const char *passphrase) {
  if (txn_metadata_ptr->change_count[0] == 0)
    return true;
  uint8_t index = utxn_ptr->output_count[0] - 1, *change_address;
  uint8_t digest[SHA256_DIGEST_LENGTH];
  uint8_t rip[RIPEMD160_DIGEST_LENGTH];
  HDNode hdnode;

  if (utxn_ptr->output[index].script_public_key[0] == OP_RETURN)
    return false;

  if (utxn_ptr->output[index].script_length[0] == 22)
    change_address = utxn_ptr->output[index].script_public_key + 2;
  else if (utxn_ptr->output[index].script_length[0] == 25)
    change_address = utxn_ptr->output[index].script_public_key + 3;
  else
    return false;

  get_address_node(txn_metadata_ptr, -1, mnemonic, passphrase, &hdnode);
  memzero(hdnode.chain_code, sizeof(hdnode.chain_code));
  memzero(hdnode.private_key, sizeof(hdnode.private_key));
  memzero(hdnode.private_key_extension, sizeof(hdnode.private_key_extension));
  sha256_Raw(hdnode.public_key, sizeof(hdnode.public_key), digest);
  ripemd160(digest, SHA256_DIGEST_LENGTH, rip);
  return (memcmp(rip, change_address, sizeof(rip)) == 0);
}