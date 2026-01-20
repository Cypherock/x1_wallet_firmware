/**
 * @file    cardano_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Helpers for cardano app
 * @copyright Copyright (c) 2026 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2026 by HODL TECH PTE LTD
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

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "cardano_helpers.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "blake2b.h"
#include "cardano/get_public_key.pb.h"
#include "cardano_context.h"
#include "coin_utils.h"
#include "segwit_addr.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

#define ADDR_HEADER_LENGTH (1)
#define BLAKE2B_PUB_KEY_LENGTH (28)
#define RAW_STAKE_ADDR_LENGTH (ADDR_HEADER_LENGTH + BLAKE2B_PUB_KEY_LENGTH)
#define RAW_PAYMENT_ADDR_LENGTH                                                \
  (ADDR_HEADER_LENGTH + (2 * BLAKE2B_PUB_KEY_LENGTH))

#define FIVE_BIT_LENGTH_UNSAFE(bytes) (((bytes) * 8) / 5)

#define RAW_STAKE_5BIT_LENGTH                                                  \
  (FIVE_BIT_LENGTH_UNSAFE(RAW_STAKE_ADDR_LENGTH) + 14)

#define RAW_PAYMENT_5BIT_LENGTH                                                \
  ((FIVE_BIT_LENGTH_UNSAFE(RAW_PAYMENT_ADDR_LENGTH)) + 9)

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
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
static int convert_bits_bech32(uint8_t *out_buf,
                               size_t *out_buf_len,
                               const uint8_t *in_bytes,
                               size_t in_bytes_len);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static int convert_bits_bech32(uint8_t *out_buf,
                               size_t *out_buf_len,
                               const uint8_t *in_bytes,
                               size_t in_bytes_len) {
  /* convert_bits returns 1 on success */
  return 1 ==
         convert_bits(out_buf, out_buf_len, 5, in_bytes, in_bytes_len, 8, 1);
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool cardano_derivation_path_guard(const uint32_t *path, uint8_t levels) {
  bool status = false;
  if (levels != CARDANO_IMPLICIT_ACCOUNT_DEPTH) {
    return status;
  }

  uint32_t purpose = path[CARDANO_PURPOSE_INDEX_POS];
  uint32_t coin = path[CARDANO_COIN_INDEX_POS];
  uint32_t account = path[CARDANO_ACCOUNT_INDEX_POS];
  uint32_t change = path[CARDANO_CHANGE_INDEX_POS];
  uint32_t wallet = path[CARDANO_WALLET_INDEX_POS];

  status = (CARDANO_PURPOSE_INDEX == purpose && CARDANO_COIN_INDEX == coin &&
            CARDANO_ACCOUNT_INDEX == account &&
            CARDANO_CHANGE_INDEX_PAYMENT == change && is_non_hardened(wallet));

  return status;
}

void stake_derv_from_payment(
    const cardano_get_public_keys_derivation_path_t *payment_path,
    cardano_get_public_keys_derivation_path_t *out_stake_path) {
  memcpy(out_stake_path,
         payment_path,
         sizeof(cardano_get_public_keys_derivation_path_t));

  /*
   * from: root / 1852' / 1815' /0' / 0 / i
   *                                  |   |
   * to:   root / 1852' / 1815' /0' / 2 / 0
   */
  out_stake_path->path[CARDANO_CHANGE_INDEX_POS] = 2;
  out_stake_path->path[CARDANO_WALLET_INDEX_POS] = 0;
}

bool get_stake_addr(const uint8_t stake_pub_key[CARDANO_PUBLIC_KEY_SIZE],
                    uint8_t out_stake_addr[CARDANO_STAKE_ADDR_LENGTH]) {
  uint8_t raw_stake_addr[RAW_STAKE_ADDR_LENGTH];
  raw_stake_addr[0] = CARDANO_STAKE_ADDR_DRV_CONST | CARDANO_NETWORK_ID;
  blake2b(stake_pub_key,
          CARDANO_PUBLIC_KEY_SIZE,
          raw_stake_addr + ADDR_HEADER_LENGTH,
          BLAKE2B_PUB_KEY_LENGTH);

  uint8_t raw_stake_addr_5bit[RAW_STAKE_5BIT_LENGTH];
  size_t raw_stake_addr_5bit_len = 0;
  bool result = convert_bits_bech32(raw_stake_addr_5bit,
                                    &raw_stake_addr_5bit_len,
                                    raw_stake_addr,
                                    sizeof(raw_stake_addr));
  if (!result) {
    return false;
  }

  return 1 == bech32_encode((char *)out_stake_addr,
                            STAKE_BECH32_PREFIX,
                            raw_stake_addr_5bit,
                            raw_stake_addr_5bit_len);
}

bool get_payment_addr(const uint8_t stake_pub_key[CARDANO_PUBLIC_KEY_SIZE],
                      const uint8_t payment_pub_key[CARDANO_PUBLIC_KEY_SIZE],
                      uint8_t out_payment_addr[CARDANO_PAYMENT_ADDR_LENGTH]) {
  /* header + blake2b(payment_pub_key) + blake2b(stake_pub_key)*/
  uint8_t combined_blake2b_pub[RAW_PAYMENT_ADDR_LENGTH];

  /* header */
  combined_blake2b_pub[0] = CARDANO_NETWORK_ID;

  /* blake2b payment pub key */
  blake2b(payment_pub_key,
          CARDANO_PUBLIC_KEY_SIZE,
          combined_blake2b_pub + ADDR_HEADER_LENGTH,
          BLAKE2B_PUB_KEY_LENGTH);

  /* blake2b stake pub key */
  blake2b(stake_pub_key,
          CARDANO_PUBLIC_KEY_SIZE,
          combined_blake2b_pub + (ADDR_HEADER_LENGTH + BLAKE2B_PUB_KEY_LENGTH),
          BLAKE2B_PUB_KEY_LENGTH);

  uint8_t payment_addr_5bit[RAW_PAYMENT_5BIT_LENGTH];
  size_t payment_addr_5bit_len = 0;
  bool result = convert_bits_bech32(payment_addr_5bit,
                                    &payment_addr_5bit_len,
                                    combined_blake2b_pub,
                                    sizeof(combined_blake2b_pub));
  if (!result) {
    return false;
  }

  return 1 == bech32_encode((char *)out_payment_addr,
                            PAYMENT_BECH32_PREFIX,
                            payment_addr_5bit,
                            payment_addr_5bit_len);
}
