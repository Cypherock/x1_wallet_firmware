/**
 * @file    wallet_utilities.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
/**
 * @file wallet_utilities.h
 * @author your name (you@domain.com)
 * @brief Utils related to a wallet
 * @version 0.1
 * @date 2020-08-19
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef WALLET_UTILITIES_H
#define WALLET_UTILITIES_H

#pragma once

#include "wallet.h"

/**
 * @brief Verify wallet id with wallet id generated from mnemonics
 *
 * @return true if all wallet id matches the wallet id generated from mnemonics,
 * else false
 *
 */
bool verify_wallet_id(const uint8_t wallet_id[WALLET_ID_SIZE],
                      const char *mnemonics);

/**
 * @brief Calculate wallet id from mnemonics
 * @details
 *
 * @param wallet_id
 * @param mnemonics
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void calculate_wallet_id(uint8_t wallet_id[WALLET_ID_SIZE],
                         const char *mnemonics);

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void derive_beneficiary_key(
    uint8_t beneficiary_key[BENEFICIARY_KEY_SIZE],
    uint8_t iv_for_beneficiary_key[IV_FOR_BENEFICIARY_KEY_SIZE],
    const char *mnemonics);

/**
 * @brief Derive wallet key. It is used to encrypt xPUB using chacha20 polly1305
 *
 * @details
 *
 * @param [out] key         32 byte key for chacha polly
 * @param       mnemonics   Mnemonics
 *
 * @return
 * @retval
 *
 * @see
 * https://docs.google.com/document/d/1-tXxQv1e6-QT58-K8dZFDWqKZB64pxRWUHFXk1ryCms/edit?usp=sharing
 * @since v1.0.0
 *
 * @note
 * node = m/190'/1' <br/>
 * key = sha256(node.private_key)
 */
void derive_wallet_key(uint8_t key[KEY_SIZE], const char *mnemonics);

/**
 * @brief Validates the contents of Wallet instance. If any check fails,
 * it logs an error code and returns false.
 * @details
 *
 * @param [in] wallet       Pointer to an instance of Wallet
 *
 * @return Card_Data_errors_t
 * @retval VALID_DATA             If all the content of the instance conforms to
 * constraints
 * @retval appropriate error code If any of the checks fail.
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
Card_Data_errors_t validate_wallet(Wallet *wallet);

/**
 * @brief The function `derive_wallet_nonce` generates a random wallet nonce and
 * assigns it to each share. On the share_encryption_data 2-D array, the wallet
 * nonce is stored at the first 12 bytes and next 4 bytes contain RFU bytes + 1
 * version byte.
 *
 * @param share_encryption_data This 2-D array is used to store the share
 * encryption data for each share.
 */
void derive_wallet_nonce(
    uint8_t share_encryption_data[TOTAL_NUMBER_OF_SHARES]
                                 [PADDED_NONCE_SIZE + WALLET_MAC_SIZE]);

#endif