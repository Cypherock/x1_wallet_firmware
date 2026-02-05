/**
 * @file    btc_txn_helper.h
 * @author  Cypherock X1 Team
 * @brief   Bitcoin family transaction helper APIs
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BTC_TXN_HELPERS_H
#define BTC_TXN_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "btc_priv.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define EXPECTED_SCRIPT_SIG_SIZE 106
#define CHUNK_SIZE 2048

#define DEFAULT_SEQUENCE 0xffffffff
#define SIGHASH_DEFAULT 0x00
#define SIGHASH_ALL 0x01
#define SIGHASH_NONE 0x02
#define SIGHASH_SINGLE 0x03
#define SIGHASH_ANYONECANPAY 0x80
#define SIGHASH_OUTPUT_MASK 0x03
#define SIGHASH_INPUT_MASK 0x80
#define ADVANCED_TRANSACTION_MARKER 0x00
#define ADVANCED_TRANSACTION_FLAG 0x01

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Calculates an estimated upper cap on the transaction fee.
 * @details The function calculates the fee according to the assumed upper cap
 * of coin.max_fee per kb. This function is only providing the threshold value
 * of transaction fee above which, user should be prompted with a warning.
 *
 * @param txn_ctx Instance of btc_txn_context_t
 *
 * @return uint64_t Value for the transaction fee
 * @retval Upper limit on the transaction fee in satoshi (smallest unit for BTC)
 */
uint64_t get_transaction_fee_threshold(const btc_txn_context_t *txn_ctx);

/**
 * @brief Get the transaction fee of a transaction as the difference of the
 * inputs and outputs.
 * @details The function calculates transaction fee in its smallest
 * unit/denomination (satoshi). If the case of overspending is observed, then
 * the result UINT64_MAX is stored at the location. In such a case, the value
 * should not be used as this is an errornous transaction.
 *
 * @param [in] utxn_ptr     Immutable reference to btc_txn_context_t instance.
 * @param [out] txn_ctx     Storage for the calculated fee
 *
 * @return bool Indicating if the process failed with errors
 * @retval true If calculation succeeded
 * @retval false In case of any inconsistency
 */
bool btc_get_txn_fee(const btc_txn_context_t *txn_ctx, uint64_t *fee);

/**
 * @brief The function populates the cache of hashes for signig segwit
 * transaction.
 * @details The function fills the calculated cache. The cache consists of the
 * common piece of information to be serialized for signing a segwit input.
 * These are 3 digests calculated over different pieces of information
 * serialized from an unsigned transaction; namely hashPrevouts, hashSequence,
 * hashOutputs. For examples refer:
 * https://github.com/bitcoin/bips/blob/master/bip-0143.mediawiki#user-content-Native_P2WPKH
 *
 * @param context Reference to the context for the current transaction to be
 * signed
 */
void btc_segwit_init_cache(btc_txn_context_t *context);

/**
 * @brief Prepares digest for the specified input to be signed.
 * @details The function prepares digest in conformation to the BIP definitions
 * for each of the input type. Currently, the function supports only 2 types of
 * input namely, P2PKH & P2WPKH. The prepared digest can be signed by a valid
 * private key to spend the input.
 *
 * @param context Reference to the bitcoin transaction context
 * @param index The index for the input to digest
 * @param digest Reference to a buffer to hold the calculated digest
 *
 * @return bool Indicating if the specified input was digested or not
 * @retval true If the digest was calculated successfully
 * @retval false If the digest was not calculated. This could be because the
 * segwit cache is not filed or the input type is other than P2PKH and P2WPKH.
 */
bool btc_digest_input(const btc_txn_context_t *context,
                      uint32_t index,
                      uint8_t *digest);

/**
 * @brief Initializes the taproot cache for signing taproot transactions.
 * @details The function fills the calculated cache for taproot transactions.
 * The cache consists of pre-computed hashes needed for signing taproot inputs.
 *
 * @param context Reference to the context for the current transaction to be
 * signed
 */
void btc_taproot_init_cache(btc_txn_context_t *context);

/**
 * @brief Generates a BIP340 Schnorr signature for Taproot transactions.
 * @details This function implements BIP340 Schnorr signature generation
 * specifically for Taproot key path spending with tweaked private keys.
 *
 * @param private_key The tweaked private key (32 bytes)
 * @param public_key The corresponding public key x-coordinate (32 bytes)
 * @param digest The transaction digest to sign (32 bytes)
 * @param signature_bytes Output buffer for the signature (64 bytes)
 *
 * @return int Status code
 * @retval 0 Success
 * @retval -1 Error (invalid inputs, signature generation failed)
 */
int schnorrsig_sign32_taproot(const uint8_t *private_key,
                              const uint8_t *public_key,
                              const uint8_t *digest,
                              uint8_t *signature_bytes);

#endif    // BTC_TXN_HELPERS_H
