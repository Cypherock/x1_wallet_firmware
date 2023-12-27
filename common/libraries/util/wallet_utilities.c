/**
 * @file    wallet_utilities.c
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
#include "wallet_utilities.h"

#include <string.h>

#include "bip32.h"
#include "bip39.h"
#include "curves.h"
#include "logger.h"
#include "sha2.h"
#include "utils.h"

void calculate_wallet_id(uint8_t wallet_id[WALLET_ID_SIZE],
                         const char *mnemonics) {
  HDNode node;
  uint8_t seed[64];
  char passphrase[256];

  memset(seed, 0, 64);
  memset(passphrase, 0, 256);
  mnemonic_to_seed(mnemonics, passphrase, seed, NULL);
  // m
  hdnode_from_seed(seed, 64, SECP256K1_NAME, &node);
  hdnode_fill_public_key(&node);
  // m/44'
  hdnode_private_ckd(&node, 0x800003E8);
  hdnode_private_ckd(&node, 0x80000000);
  hdnode_private_ckd(&node, 0x80000003);
  hdnode_private_ckd(&node, 0x00000001);    // m/1000'/0'/3'/1
  hdnode_fill_public_key(&node);

  sha256_Raw(node.public_key, sizeof(node.public_key), wallet_id);
  sha256_Raw(wallet_id, SHA256_DIGEST_LENGTH, wallet_id);
}

bool verify_wallet_id(const uint8_t wallet_id[WALLET_ID_SIZE],
                      const char *mnemonics) {
  uint8_t generated_wallet_id[WALLET_ID_SIZE] = {0};

  calculate_wallet_id(generated_wallet_id, mnemonics);
  if (0 == memcmp(wallet_id, generated_wallet_id, WALLET_ID_SIZE)) {
    return true;
  } else {
    log_hex_array("Expected wallet id: ", wallet_id, WALLET_ID_SIZE);
    log_hex_array("Generated wallet id: ", generated_wallet_id, WALLET_ID_SIZE);
    return false;
  }
}

void derive_beneficiary_key(
    uint8_t beneficiary_key[BENEFICIARY_KEY_SIZE],
    uint8_t iv_for_beneficiary_key[IV_FOR_BENEFICIARY_KEY_SIZE],
    const char *mnemonics) {
  HDNode node;
  uint8_t seed[64];
  char passphrase[256];

  memset(seed, 0, 64);
  memset(passphrase, 0, 256);
  mnemonic_to_seed(mnemonics, passphrase, seed, NULL);
  hdnode_from_seed(seed, 64, SECP256K1_NAME, &node);    // m
  hdnode_private_ckd(&node, 0x800003E8);
  hdnode_private_ckd(&node, 0x80000000);
  hdnode_private_ckd(&node, 0x80000004);
  hdnode_private_ckd(&node, 0x00000001);    // m/1000'/0'/4'/1

  uint8_t hash[64];
  sha512_Raw(node.private_key, 32, hash);

  uint8_t bytes_copied = 0;
  memcpy(beneficiary_key, hash + bytes_copied, BENEFICIARY_KEY_SIZE);
  bytes_copied += BENEFICIARY_KEY_SIZE;
  memcpy(
      iv_for_beneficiary_key, hash + bytes_copied, IV_FOR_BENEFICIARY_KEY_SIZE);
}

void derive_wallet_key(uint8_t key[KEY_SIZE], const char *mnemonics) {
  HDNode node;
  uint8_t seed[64];
  char passphrase[256];

  memset(seed, 0, 64);
  memset(passphrase, 0, 256);
  mnemonic_to_seed(mnemonics, passphrase, seed, NULL);
  hdnode_from_seed(seed, 64, SECP256K1_NAME, &node);    // m
  hdnode_private_ckd(&node, 0x800003E8);
  hdnode_private_ckd(&node, 0x80000000);
  hdnode_private_ckd(&node, 0x80000002);
  hdnode_private_ckd(&node, 0x00000001);    // m/1000'/0'/2'/1

  uint8_t hash[KEY_SIZE];
  sha256_Raw(node.private_key, sizeof(node.private_key), hash);
  memcpy(key, hash, KEY_SIZE);
}

Card_Data_errors_t validate_wallet(Wallet *wallet) {
  if (wallet == NULL)
    return true;
  if (!verify_checksum(wallet))
    return INVALID_CHECKSUM;
  if (strnlen((char *)wallet->wallet_name, NAME_SIZE) >= NAME_SIZE)
    return INVALID_NAME_LENGTH;
  if (wallet->wallet_info & 0xF8)
    return INVALID_WALLET_CONFIG;
  if (WALLET_IS_PIN_SET(wallet->wallet_info) &&
      is_zero(wallet->password_double_hash,
              sizeof(wallet->password_double_hash))) {
    return INVALID_WALLET_CONFIG;
  }
  if (wallet->number_of_mnemonics % 6 || wallet->number_of_mnemonics < 12 ||
      wallet->number_of_mnemonics > 24) {
    return INVALID_MNEMONIC_LENGTH;
  }
  if (wallet->minimum_number_of_shares != MINIMUM_NO_OF_SHARES)
    return INVALID_SHAMIR_CONFIG;
  if (wallet->total_number_of_shares != TOTAL_NUMBER_OF_SHARES)
    return INVALID_SHAMIR_CONFIG;
  if (wallet->xcor >= TOTAL_NUMBER_OF_SHARES)
    return INVALID_SHARE_INDEX;
  if (is_zero(wallet->wallet_id, sizeof(wallet->wallet_id)))
    return INVALID_WALLET_ID;
  return VALID_DATA;
}

void derive_wallet_nonce(
    uint8_t share_encryption_data[TOTAL_NUMBER_OF_SHARES]
                                 [PADDED_NONCE_SIZE + WALLET_MAC_SIZE]) {
  uint8_t wallet_nonce[NONCE_SIZE] = {0};
  random_generate(wallet_nonce, NONCE_SIZE);

  for (int i = 0; i < TOTAL_NUMBER_OF_SHARES; i++) {
    // First 12 bytes of share_encryption_data are wallet nonce.
    memcpy(share_encryption_data[i], wallet_nonce, NONCE_SIZE);
    // Skip next 3 bytes as RFU
    // Version byte
    share_encryption_data[i][15] = 0x01;
  }
}