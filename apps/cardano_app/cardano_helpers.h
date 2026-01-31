/**
 * @file    cardano_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Headers for helpers for cardano app
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef CARDANO_HELPERS_H
#define CARDANO_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bip32.h"
#include "cardano/get_public_key.pb.h"
#include "cardano_context.h"
#include "cardano_priv.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

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
 * @brief Converts 8 bit wide byte buffer to 5 bit wide byte buffer as required
 * in cardano.
 *
 * @param out_buf        Out buffer to write to
 * @param out_buf_len    Written bytes count
 * @param in_bytes       Input bytes to convert
 * @param in_bytes_len   Input bytes len
 */
bool convert_bits_bech32(uint8_t *out_buf,
                         size_t *out_buf_len,
                         const uint8_t *in_bytes,
                         size_t in_bytes_len);

/**
 * @brief Verifies the derivation path for cardano
 * @details The derivation depth is fixed at level 5. So if the depth level !=
 * 5, then this function return false indicating invalid derivation path. The
 * function supports checking derivation paths for HD wallets Types of
 * derivations:
 * payment : m/1852'/1815'/0'/0/i
 * stake   : m/1852'/1815'/0'/2/0
 *
 * but since the device only ever sends payment derivation path, we need to
 * verify that.
 *
 * @param[in] path      The derivation path as an uint32 array
 * @param[in] levels    The number of levels in the derivation path
 *
 * @return bool  Indicates if the provided derivation path is valid
 * @retval true  Derivation path is valid
 * @retval false Derivation path is invalid
 *
 */
bool cardano_derivation_path_guard(const uint32_t *path, uint8_t levels)
    __attribute__((warn_unused_result));

/**
 * @brief Converts payment derivation path into stake derivation path
 *
 * @param[in] payment_path     Payment address derivation path
 * @param[out] out_stake_path  Out Stake address derivation path
 */
void stake_derv_from_payment(
    const cardano_get_public_keys_derivation_path_t *payment_path,
    cardano_get_public_keys_derivation_path_t *out_stake_path);

/**
 * @brief Gets stake public ADDRESS from stake public key
 * @details
 * Reward Address
 * addr = 0xe0 | network_id + blake2b_hash(stake_wallet_public_key)
 *
 * network id:
 *    0x01 = mainnet
 *    0x02 = testnet
 *
 * @param[in] public_key            public key derived for stake addr
 * @param[out] out_stake_addr       Stake addr buffer
 *
 * @return bool Indicates whether successfully computed address or not.
 */
bool get_stake_addr(const uint8_t stake_pub_key[CARDANO_PUBLIC_KEY_SIZE],
                    uint8_t out_stake_addr[CARDANO_STAKE_ADDR_LENGTH]);

/**
 * @brief Computes payment address
 * @details
 *
 * @param[in]  stake_pub_key    Stake public key
 * @param[in]  payment_pub_key  External/payment public key
 * @param[out] out_payment_addr Buf where final address will be written
 *
 * @return bool Indicates whether successfully computed address or not
 */
bool get_payment_addr(const uint8_t stake_pub_key[CARDANO_PUBLIC_KEY_SIZE],
                      const uint8_t payment_pub_key[CARDANO_PUBLIC_KEY_SIZE],
                      uint8_t out_payment_addr[CARDANO_PAYMENT_ADDR_LENGTH]);

/**
 * @brief Converts Lovelace quantity into Ada
 *
 * @param[in] lovelace Quantity in lovelace
 * @return double Quantity in Ada
 */
double ada_from_lovelace(uint64_t lovelace);

#endif    // CARDANO_HELPERS_H
