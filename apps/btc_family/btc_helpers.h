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

#endif
