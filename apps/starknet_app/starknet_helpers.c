/**
 * @file    starknet_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Starknet chains
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
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

#include "starknet_helpers.h"

#include <error.pb.h>
#include <starkcurve.h>

#include "coin_utils.h"
#include "mini-gmp-helpers.h"
#include "mini-gmp.h"
#include "starknet_api.h"
#include "starknet_context.h"
#include "starknet_crypto.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Grind starknet private from provided 32-byte seed
 */
static bool grind_key(const uint8_t *grind_seed, uint8_t *out);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

bool grind_key(const uint8_t *grind_seed, uint8_t *out) {
  uint8_t key[STARKNET_BIGNUM_SIZE] = {0};
  mpz_t strk_limit;
  mpz_t strk_key;

  mpz_init_set_str(strk_limit, STARKNET_LIMIT, SIZE_HEX);

  SHA256_CTX ctx = {0};
  mpz_init(strk_key);
  for (uint8_t itr = 0; itr < 200; itr++) {
    sha256_Init(&ctx);
    sha256_Update(&ctx, grind_seed, STARKNET_BIGNUM_SIZE);
    sha256_Update(&ctx, &itr, 1);
    sha256_Final(&ctx, key);

    byte_array_to_mpz(strk_key, key, STARKNET_BIGNUM_SIZE);
    if (mpz_cmp(strk_key, strk_limit) == -1) {
      mpz_t f_key;
      mpz_init(f_key);
      mpz_mod(f_key, strk_key, stark_curve->order);
      mpz_to_byte_array(f_key, out, STARKNET_BIGNUM_SIZE);

      // clear mpz variables
      mpz_clear(f_key);
      mpz_clear(strk_key);
      mpz_clear(strk_limit);
      return true;
    }
  }

  starknet_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 0);
  LOG_CRITICAL("ERROR: grind 200 iterations failed\n");

  // clear mpz variables
  mpz_clear(strk_key);
  mpz_clear(strk_limit);
  return false;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool starknet_derivation_path_guard(const uint32_t *path, uint8_t levels) {
  bool status = false;
  if (STARKNET_IMPLICIT_ACCOUNT_DEPTH != levels) {
    return status;
  }

  uint32_t purpose = path[0], layer = path[1], application = path[2],
           eth_1 = path[3], eth_2 = path[4], address = path[5];

  // m/2645'/1195502025'/1148870696'/0'/0'/i
  status =
      (STARKNET_PURPOSE_INDEX == purpose && STARKNET_LAYER_INDEX == layer &&
       STARKNET_APPLICATION_INDEX == application &&
       STARKNET_ETH_1_INDEX == eth_1 && STARKNET_ETH_2_INDEX == eth_2 &&
       is_non_hardened(address));

  return status;
}

bool starknet_derive_key_from_seed(const uint8_t *seed,
                                   const uint32_t *path,
                                   uint32_t path_length,
                                   uint8_t *key_priv,
                                   uint8_t *key_pub) {
  HDNode stark_child_node = {0};

  // derive node at m/2645'/1195502025'/1148870696'/0'/0'/i
  if (!derive_hdnode_from_path(
          path, path_length, SECP256K1_NAME, seed, &stark_child_node)) {
    // send unknown error; unknown failure reason
    starknet_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    memzero(&stark_child_node, sizeof(HDNode));
    return false;
  }

  uint8_t stark_private_key[STARKNET_BIGNUM_SIZE] = {0};
  mpz_curve_point p;
  mpz_curve_point_init(&p);
  if (!grind_key(stark_child_node.private_key, stark_private_key)) {
    mpz_curve_point_clear(&p);
    return false;
  }

  // copy stark priv key if required
  if (key_priv != NULL) {
    memzero(key_priv, STARKNET_BIGNUM_SIZE);
    memcpy(key_priv, stark_private_key, STARKNET_BIGNUM_SIZE);
  }

  // derive stark pub key from stark priv key
  mpz_t priv_key;
  mpz_init(priv_key);
  byte_array_to_mpz(priv_key, stark_private_key, STARKNET_BIGNUM_SIZE);
  mpz_curve_point_multiply(stark_curve, priv_key, &stark_curve->G, &p);
  mpz_clear(priv_key);    // clear priv key when no longer required

  uint8_t stark_public_key[STARKNET_BIGNUM_SIZE] = {0};
  mpz_to_byte_array(p.x, stark_public_key, STARKNET_PUB_KEY_SIZE);

  // copy stark pub key if required
  if (key_pub != NULL) {
    memzero(key_pub, STARKNET_PUB_KEY_SIZE);
    memcpy(key_pub, stark_public_key, STARKNET_PUB_KEY_SIZE);
  }

  // clear mpz variables
  mpz_curve_point_clear(&p);

  return true;
}
