/**
 * @file    wallet.c
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
#include "wallet.h"

#include <string.h>

#include "chacha20poly1305.h"
#include "options.h"
#include "rfc7539.h"
#include "sha2.h"

/// Global Wallet instance.
Wallet CONFIDENTIAL wallet = {
    .wallet_name = {0},
    .wallet_info = 0,
    .password_double_hash = {0},
    .wallet_share_with_mac_and_nonce = {0},
    .arbitrary_data_share = {0},
    .number_of_mnemonics = 0,
    .minimum_number_of_shares = 0,
    .total_number_of_shares = 0,
    .arbitrary_data_size = 0,
    .xcor = 0,
    .checksum = {0},
    .key = {0},
    .beneficiary_key = {0},
    .iv_for_beneficiary_key = {0},
    .wallet_id = {0},
};

/// Global instance to store confidential data
Wallet_credential_data CONFIDENTIAL wallet_credential_data = {
    .mnemonics = {{'\0'}},
    .passphrase = {'\0'},
    .password_single_hash = {0}};

/// Global instance to store shamir data
Wallet_shamir_data CONFIDENTIAL wallet_shamir_data = {
    {.arbitrary_data_shares = {{0}}},
    .share_x_coords = {0},
    .share_encryption_data = {{0}}};

void clear_wallet_data() {
  memzero(&wallet, sizeof(wallet));
  memzero(&wallet_shamir_data, sizeof(wallet_shamir_data));
  memzero(&wallet_credential_data, sizeof(wallet_credential_data));
}

bool encrypt_shares() {
  uint8_t share[BLOCK_SIZE];
  chacha20poly1305_ctx ctx;
  for (int i = 0; i < wallet.total_number_of_shares; i++) {
    rfc7539_init(&ctx,
                 wallet_credential_data.password_single_hash,
                 wallet_shamir_data.share_encryption_data[i]);
    rfc7539_auth(&ctx, wallet_shamir_data.mnemonic_shares[i], BLOCK_SIZE);
    chacha20poly1305_encrypt(
        &ctx, wallet_shamir_data.mnemonic_shares[i], share, BLOCK_SIZE);
    chacha20poly1305_finish(
        &ctx,
        (uint8_t *)(wallet_shamir_data.share_encryption_data[i] +
                    PADDED_NONCE_SIZE));
    memcpy(wallet_shamir_data.mnemonic_shares[i], share, BLOCK_SIZE);
  }

  return true;
}

bool decrypt_shares() {
  // always consider 4 shares for decryption
  uint8_t share[BLOCK_SIZE];
  chacha20poly1305_ctx ctx;
  for (int i = 0; i < wallet.total_number_of_shares; i++) {
    rfc7539_init(&ctx,
                 wallet_credential_data.password_single_hash,
                 wallet_shamir_data.share_encryption_data[i]);
    rfc7539_auth(&ctx, wallet_shamir_data.mnemonic_shares[i], BLOCK_SIZE);
    chacha20poly1305_decrypt(
        &ctx, wallet_shamir_data.mnemonic_shares[i], share, BLOCK_SIZE);
    chacha20poly1305_finish(
        &ctx,
        (uint8_t *)(wallet_shamir_data.share_encryption_data[i] +
                    PADDED_NONCE_SIZE));
    // TODO: Add mac comparison for decryption verification
    memcpy(wallet_shamir_data.mnemonic_shares[i], share, BLOCK_SIZE);
  }
  memzero(wallet_credential_data.password_single_hash,
          sizeof(wallet_credential_data.password_single_hash));
  memzero(wallet_shamir_data.share_encryption_data,
          sizeof(wallet_shamir_data.share_encryption_data));

  return true;
}

void calculate_checksum(const Wallet *wallet, uint8_t *checksum) {
  if (wallet == NULL || checksum == NULL)
    return;

  uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
  SHA256_CTX context = {0};
  sha256_Init(&context);
  sha256_Update(&context, wallet->wallet_name, sizeof(wallet->wallet_name));
  sha256_Update(&context, &wallet->xcor, sizeof(wallet->xcor));
  sha256_Update(&context,
                &wallet->number_of_mnemonics,
                sizeof(wallet->number_of_mnemonics));
  sha256_Update(&context,
                &wallet->total_number_of_shares,
                sizeof(wallet->total_number_of_shares));
  sha256_Update(&context,
                wallet->wallet_share_with_mac_and_nonce,
                sizeof(wallet->wallet_share_with_mac_and_nonce));
  sha256_Update(&context,
                &wallet->minimum_number_of_shares,
                sizeof(wallet->minimum_number_of_shares));
  sha256_Update(&context, &wallet->wallet_info, sizeof(wallet->wallet_info));
  sha256_Update(&context, wallet->key, sizeof(wallet->key));
  sha256_Update(&context, wallet->wallet_id, sizeof(wallet->wallet_id));
  sha256_Update(
      &context, wallet->arbitrary_data_share, wallet->arbitrary_data_size);
  sha256_Final(&context, digest);
  memcpy(checksum, digest, sizeof(wallet->checksum));
  // set the bits for indicating usability of the checksum
  // refer doc of the Wallet.checksum
  checksum[3] &= 0xFC;
  checksum[3] |= 0x01;
}

bool verify_checksum(const Wallet *wallet) {
  if (wallet == NULL)
    return false;

  // check if the checksum is usable i.e. `01`
  // always return true if value is not; refer doc of Wallet.checksum
  if ((wallet->checksum[3] & 0x03) != 0x01)
    return true;

  uint8_t checksum[sizeof(wallet->checksum)] = {0};

  calculate_checksum(wallet, checksum);
  return (memcmp(wallet->checksum, checksum, sizeof(wallet->checksum)) == 0);
}
