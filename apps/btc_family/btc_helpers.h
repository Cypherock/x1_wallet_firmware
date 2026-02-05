/**
 * @file    btc_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Utilities api definitions for Bitcoin chain
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef BTC_HELPERS_H
#define BTC_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define PURPOSE_LEGACY 0x8000002C     // 44'
#define PURPOSE_SEGWIT 0x80000031     // 49'
#define PURPOSE_NSEGWIT 0x80000054    // 84'
#define PURPOSE_TAPROOT 0x80000056    // 86'

#define BTC_ACC_XPUB_DEPTH 3
#define BTC_ACC_ADDR_DEPTH 5

#define SATOSHI_PER_BTC 100000000

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
 * @brief Returns the segwit address string (result of Bech32 encoding).
 * @details
 *
 * @param [in] public_key   Byte array representation of public key.
 * @param [in] key_len      Length of public key byte array.
 * @param [in] hrp          HRP value for bech32 encoding
 * @param [out] address     char array to store segwit address.
 *
 * @return 0 if successful and 1 if failure.
 * @retval 0 Success
 * @retval 1 Failure
 */
int btc_get_segwit_addr(const uint8_t *public_key,
                        uint8_t key_len,
                        const char *hrp,
                        char *address);

/**
 * @brief Generates xpub for the passed purpose id, coin id and account id.
 * @details The function uses fingerprint of second last node and assumes first
 * two nodes are purpose and coin index.
 *
 * @param [in] path             Path of the node to derive xpub
 * @param [in] path_length      Length of the given path.
 * @param [in] curve            Curve to be used.
 * @param [in] seed             Seed to generate the master node of 64 bytes
 * @param [in] version          HD version for xpub encoding
 * @param [out] str              String to store the xpub of XPUB_SIZE
 *
 * @return bool Indicating if the derivation was successful
 * @retval true If the node derivation succeeded.
 * @retval false If the node derivation failed.
 */
bool btc_generate_xpub(const uint32_t *path,
                       size_t path_length,
                       const char *curve,
                       const uint8_t *seed,
                       uint32_t version,
                       char *str);

/**
 * @brief Returns the HD version for xpub encoding for the specified purpose
 * index.
 * @details The function simply fetches the respective value stored in the app
 * config instance. It is not responsible for verifying if the provided purpose
 * index is supported by the current Bitcoin family app.
 *
 * @param [in]  purpose_index   The purpose index to get the HD version for xpub
 * @param [out] xpub_ver        The output version for the xpub encoding
 *
 * @return bool Indicates if the provided purpose index is supported
 * @retval true if the purpose index is supported
 * @retval false otherwise
 *
 * @note The function does checks for validity of the provided purpose index.
 */
bool btc_get_version(uint32_t purpose_index, uint32_t *xpub_ver);

/**
 * @brief Verifies the derivation path for any inconsistent/unsupported values.
 * @details The function supports checking for multiple derivation paths. Also,
 * based on the provided depth value, the function can act as xpub or address
 * derivation path.
 * The only allowed depth count is either 3 (account node level) for xpub or 5
 * (address node level) for public address derivation.
 * Currently, following purpose indices are allowed: 0x8000002C, 0x80000031,
 * 0x80000054, 0x80000056.
 * The function accepts 0 & 1 for change indices. For the rest of the levels,
 * only correct hardness is checked.
 *
 * @param[in] path      The derivation path to be checked
 * @param[in] depth     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided path indices are valid
 * @retval true if the path values are valid
 * @retval false otherwise
 */
bool btc_derivation_path_guard(const uint32_t *path, uint32_t depth);

/**
 * @brief Formats the provided satoshi value to equivalent string formatted BTC
 * value
 * @details The function ensures appropriate decimal precision in the formatted
 * string. In the other words, the function can handle variable precision
 * on-the-fly based on the provided value. Refer:
 * https://developer.bitcoin.org/devguide/payment_processing.html#plain-text
 *
 * @param value_in_sat The value in Satoshi unit to be encoded to BTC
 * @param msg The output buffer for equivalent string formatted BTC value
 * @param msg_len The size of the output buffer
 */
void format_value(uint64_t value_in_sat, char *msg, size_t msg_len);

/**
 * @brief Returns the taproot key path address string (Bech32M encoding)
 * @details The functions assumes public key is compressed
 *
 * @param [in] public_key    33 Byte array representation of public key.
 * @param [in] hrp           HRP value for bech32M encoding "bc" for mainnet and
 * "tb" for testnet.
 * @param [out] address  char array to store taproot address.
 *
 * @return 1 if successful and 0 if failure.
 */
int btc_get_taproot_address(uint8_t *public_key,
                            const char *hrp,
                            char *address);

/**
 * @brief Tweaks a private key for Taproot key path spending.
 * @details This function implements BIP341 private key tweaking for Taproot.
 * The tweaked private key is computed as sk' = sk + t (mod n) where t is the
 * tweak hash.
 *
 * @param private_key The internal private key (32 bytes)
 * @param root_hash The Merkle root hash (32 bytes, can be NULL for key path
 * only)
 * @param public_key The public key (33 bytes, compressed format)
 * @param tweaked_private_key Output buffer for the tweaked private key (32
 * bytes)
 *
 * @return bool Status code
 * @retval true Success
 * @retval false Error (invalid inputs, tweaking failed)
 */
bool bip340_tweak_private_key(const uint8_t *private_key,
                              const uint8_t *public_key,
                              const uint8_t *root_hash,
                              uint8_t *tweaked_private_key);

/**
 * @brief Tweaks a public key for Taproot key path spending.
 * @details This function implements BIP341 public key tweaking for Taproot.
 * The tweaked public key is computed as P' = P + t*G where t is the tweak hash.
 *
 * @param public_key The internal public key (33 bytes, compressed format)
 * @param root_hash The Merkle root hash (32 bytes, can be NULL for key path
 * only)
 * @param tweaked_public_key Output buffer for the tweaked public key
 * x-coordinate (32 bytes)
 *
 * @return bool Status code
 * @retval true Success
 * @retval false Error (invalid inputs, tweaking failed)
 */
bool bip340_tweak_public_key(const uint8_t *public_key,
                             const uint8_t *root_hash,
                             uint8_t *tweaked_public_key);

/**
 * @brief BIP340 tagged hash implementation
 * @details This function implements BIP340 tagged hash for Taproot.
 *
 * @param tag The tag to be hashed
 * @param out The output buffer for the hashed data
 * @param data The data to be hashed
 * @param data_len The length of the data
 */
void bip340_tagged_hash(const char *tag,
                        uint8_t *out,
                        const uint8_t *data,
                        size_t data_len);
#endif
