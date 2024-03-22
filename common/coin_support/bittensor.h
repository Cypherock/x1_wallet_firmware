/**
 * @file    bittensor.h
 * @author  Cypherock X1 Team
 * @brief   Header for bittensor coin.
 *          Stores declarations for bittensor coin functions.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef BITTENSOR_HEADER
#define BITTENSOR_HEADER

#include <stdint.h>

#include "bittensor_txn_helpers.h"
#include "coin_utils.h"

#define SOL_COIN_VERSION 0x00000000

// Derivation path reference :
// https://docs.bittensor.com/wallet-guide/paper-wallet#hierarchical-derivation
// Taking 3 levels of depth similar to ledger
#define BITTENSOR_PURPOSE_INDEX 0x8000002C
#define BITTENSOR_COIN_INDEX 0x800001F5
#define BITTENSOR_ACCOUNT_INDEX 0x80000000
#define BITTENSOR_CHANGE_INDEX 0x80000000

/// Ref: https://docs.bittensor.com/terminology#lamport
#define BITTENSOR_DECIMAL (9U)

#define BITTENSOR_ACCOUNT_ADDRESS_LENGTH 32
#define BITTENSOR_BLOCKHASH_LENGTH 32

typedef enum bittensor_account_type {
  UNUSED = 0,
  SOL_ACC_TYPE1 = 1,
  SOL_ACC_TYPE2 = 2,
  SOL_ACC_TYPE3 = 3,
} bittensor_account_type;

/**
 * @brief Returns the derivation depth for each account_type
 * If the account tag/type is invalid, this will return 3 as the default depth
 *
 * @param[in] tag   The account type/tag to find depth
 *
 * @since v1.0.0
 */
size_t bittensor_get_derivation_depth(uint16_t tag);

/**
 * @brief Signed unsigned byte array.
 * @details
 *
 * @param [in] unsigned_txn_byte_array      Byte array of unsigned transaction.
 * @param [in] unsigned_txn_len             length of unsigned transaction byte
 * array.
 * @param [in] transaction_metadata         Pointer to txn_metadata instance.
 * @param [in] mnemonics                    char array of mnemonics.
 * @param [in] passphrase                   char array of passphrase.
 * @param [out] sig                         Byte array of signature to store the
 * result of signing unsigned transaction byte array.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void bittensor_sig_unsigned_byte_array(const uint8_t *unsigned_txn_byte_array,
                                       uint64_t unsigned_txn_len,
                                       const txn_metadata *transaction_metadata,
                                       const char *mnemonics,
                                       const char *passphrase,
                                       uint8_t *sig);
/**
 * @brief Update given blockhash in serialized array
 *
 * @param byte_array    pointer to serialized array of unsigned transaction to
 * be modified
 * @param blockhash     pointer to array of latest blockhash
 * @return int
 */
int bittensor_update_blockhash_in_byte_array(uint8_t *byte_array,
                                             const uint8_t *blockhash);

/**
 * @brief Verifies the derivation path for any inconsistent/unsupported values.
 * If depth level < 2 this function returns false indicating invalid derivation
 * path. The function supports checks for `m/44'/501'`, `m/44'/501'/i'` &
 * `m/44'/501'/i'/0'` any other format would be considered as invalid. It is
 * important that the hardened derivation is used. Non-hardened derivation paths
 * will be invalid.
 *
 * @param[in] path          The address derivation path to be checked
 * @param[in] levels        The number of levels in the derivation path
 *
 * @return bool     Returns true if the path values are valid. False otherwise.
 *
 * @since v1.0.0
 */
bool bittensor_verify_derivation_path(const uint32_t *path, uint8_t levels);

#endif    // BITTENSOR_HEADER
