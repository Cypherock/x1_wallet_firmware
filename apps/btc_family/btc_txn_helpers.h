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

#include "btc/sign_txn.pb.h"
#include "btc_priv.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define EXPECTED_SCRIPT_SIG_SIZE 106

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
 * @brief Verifies the provided input with its related raw transaction byte
 * @details The function verifies if the input details match with the details in
 * the raw transaction. This is done by checking the output value against the
 * specified output index in the raw transaction and then finally matching the
 * specified hash with the calculated hash from the raw transactions bytes.
 *
 * @param [in] raw_txn  Byte array of a transaction.
 * @param [in] size     Size of the raw_txn.
 * @param [in] input    Immutable reference to the btc_txn_input_t.
 *
 * @return int Result of verification, 0 if verified otherwise error status.
 * @retval 0 Input verified successfully.
 * @retval -1 If function parameters are invalid
 * @retval 1 If specified output index (input->prev_output_index) is not present
 * @retval 2 If there is a hash (input->prev_txn_hash) mismatch
 * @retval 3 If there is a value (input->value) mismatch
 */
int btc_verify_input(const uint8_t *raw_txn,
                     uint32_t size,
                     const btc_sign_txn_input_t *input);

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

#endif    // BTC_TXN_HELPERS_H
