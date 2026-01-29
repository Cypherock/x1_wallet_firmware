/**
 * @file    btc_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Bitcoin blockchain
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

#include "btc_helpers.h"

#include <stdint.h>
#include <string.h>

#include "bignum.h"
#include "btc_priv.h"
#include "coin_utils.h"
#include "ecdsa.h"
#include "flash_config.h"
#include "secp256k1.h"
#include "segwit_addr.h"
#include "sha2.h"
#include "utils.h"

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

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

// Computes sha256(tap_tweak_hash + tap_tweak_hash + x_only_public_key +
// root_hash) and stores the result in tweak_key_hash.
static bool bip340_tweak_key_hash(const uint8_t *x_only_public_key,
                                  const uint8_t *root_hash,
                                  uint8_t tweak_key_hash[32]) {
  if (NULL == x_only_public_key || NULL == tweak_key_hash) {
    return false;
  }

  size_t payload_size = 32;    // x_only_public_key
  if (root_hash != NULL) {
    payload_size += 32;
  }

  uint8_t payload[payload_size];
  memzero(payload, payload_size);
  // Prepare the data for hashing
  memcpy(payload, x_only_public_key, 32);
  if (root_hash != NULL) {
    memcpy(payload + 32, root_hash, 32);
  }

  bip340_tagged_hash("TapTweak", tweak_key_hash, payload, payload_size);
  return true;
}

// Calculates result = (public_key_point + tweak_key_hash * G).x
static bool bip340_point_add_tweak(const ecdsa_curve *curve,
                                   const uint8_t *public_key,
                                   const uint8_t *tweak_key_hash,
                                   uint8_t result[32]) {
  if (NULL == public_key || NULL == tweak_key_hash || NULL == result) {
    return false;
  }

  curve_point public_key_point = {0};
  if (!ecdsa_read_pubkey(curve, public_key, &public_key_point)) {
    return false;
  }

  // Negate y-coordinate if it's odd
  if (bn_is_odd(&public_key_point.y)) {
    bn_subtract(&curve->prime, &public_key_point.y, &public_key_point.y);
    bn_mod(&public_key_point.y, &curve->prime);
  }

  bignum256 tweak_hash_bn = {0};
  bn_read_be(tweak_key_hash, &tweak_hash_bn);
  bn_mod(&tweak_hash_bn, &curve->order);

  if (bn_is_zero(&tweak_hash_bn)) {
    return false;
  }

  curve_point result_point = {0};
  point_multiply(curve,
                 &tweak_hash_bn,
                 &curve->G,
                 &result_point);    // result_point = tweak_hash_bn * G
  point_add(curve,
            &public_key_point,
            &result_point);    // result_point = public_key_point + result_point

  // take the x-coordinate of the result point
  bn_write_be(&result_point.x, result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

int btc_get_segwit_addr(const uint8_t *public_key,
                        uint8_t key_len,
                        const char *hrp,
                        char *address) {
  // output of Hash160 (sha256 + ripemd160), use size of bigger hash
  uint8_t rip[SHA256_DIGEST_LENGTH] = {0};
  if (!public_key || !address) {
    return 1;
  }

  if (key_len != 33 && key_len != 65) {
    return 1;
  }

  if (key_len == 65) {
    bignum256 y_ordinate;
    bn_read_be(public_key + 33, &y_ordinate);
    ((uint8_t *)public_key)[0] = bn_is_odd(&y_ordinate) ? 0x03 : 0x02;
    key_len = 33;
  }

  hasher_Raw(HASHER_SHA2_RIPEMD, public_key, key_len, rip);
  return segwit_addr_encode(address, hrp, 0x00, rip, RIPEMD160_DIGEST_LENGTH);
}

bool btc_generate_xpub(const uint32_t *path,
                       const size_t path_length,
                       const char *curve,
                       const uint8_t *seed,
                       const uint32_t version,
                       char *str) {
  uint32_t fingerprint = 0x0;
  HDNode t_node = {0};
  bool status = true;

  status &=
      derive_hdnode_from_path(path, path_length - 1, curve, seed, &t_node);
  fingerprint = hdnode_fingerprint(&t_node);
  if (0 == hdnode_private_ckd(&t_node, path[path_length - 1])) {
    // hdnode_private_ckd returns 1 when the derivation succeeds
    status &= false;
  }
  hdnode_fill_public_key(&t_node);
  if (0 ==
      hdnode_serialize_public(&t_node, fingerprint, version, str, XPUB_SIZE)) {
    status &= false;
  }
  memzero(&t_node, sizeof(HDNode));
  return status;
}

bool btc_get_version(uint32_t purpose_index, uint32_t *xpub_ver) {
  bool status = true;
  switch (purpose_index) {
    case PURPOSE_LEGACY:
      *xpub_ver = g_btc_app->legacy_xpub_ver;
      break;
    case PURPOSE_SEGWIT:
      *xpub_ver = g_btc_app->segwit_xpub_ver;
      break;
    case PURPOSE_NSEGWIT:
      *xpub_ver = g_btc_app->nsegwit_xpub_ver;
      break;
    case PURPOSE_TAPROOT:
      *xpub_ver = g_btc_app->taproot_xpub_ver;
      break;
    default:
      status = false;
  }
  return status;
}

bool btc_derivation_path_guard(const uint32_t *path, uint32_t depth) {
  bool status = false;
  if (BTC_ACC_XPUB_DEPTH != depth && BTC_ACC_ADDR_DEPTH != depth) {
    return status;
  }
  status = true;

  // common checks for xpub/account and address nodes
  if (NULL == g_btc_app->is_purpose_supported ||
      !g_btc_app->is_purpose_supported(path[0])) {
    // unsupported purpose index
    status = false;
  }
  if (g_btc_app->coin_type != path[1] || is_non_hardened(path[2])) {
    // coin index or account hardness mismatch
    status = false;
  }

  if (BTC_ACC_ADDR_DEPTH == depth) {
    // address node specific checks
    if (is_hardened(path[3]) || is_hardened(path[4])) {
      // change or address index must be non-hardened
      status = false;
    }
    if (0 != path[3] && 1 != path[3]) {
      // invalid change address
      status = false;
    }
  }
  return status;
}

void format_value(const uint64_t value_in_sat,
                  char *msg,
                  const size_t msg_len) {
  uint8_t precision = get_floating_precision(value_in_sat, SATOSHI_PER_BTC);
  double fee_in_btc = 1.0 * value_in_sat / (SATOSHI_PER_BTC);
  snprintf(
      msg, msg_len, "%0.*f %s", precision, fee_in_btc, g_btc_app->lunit_name);
}

// BIP340 tagged hash implementation
void bip340_tagged_hash(const char *tag,
                        uint8_t *out,
                        const uint8_t *data,
                        size_t data_len) {
  uint8_t tag_hash[32];
  Hasher hasher;

  // Hash the tag
  hasher_Init(&hasher, HASHER_SHA2);
  hasher_Update(&hasher, (const uint8_t *)tag, strlen(tag));
  hasher_Final(&hasher, tag_hash);

  // tagged_hash = SHA256(SHA256(tag) || SHA256(tag) || data)
  hasher_Init(&hasher, HASHER_SHA2);
  hasher_Update(&hasher, tag_hash, 32);
  hasher_Update(&hasher, tag_hash, 32);
  hasher_Update(&hasher, data, data_len);
  hasher_Final(&hasher, out);
}

// implementation of BIP-340 tweak public key for taproot without using
// secp256k1 library
bool bip340_tweak_public_key(const uint8_t *public_key,
                             const uint8_t *root_hash,
                             uint8_t *tweaked_public_key) {
  if (NULL == public_key || NULL == tweaked_public_key) {
    return false;
  }

  uint8_t tweak_key_hash[32] = {0};
  if (!bip340_tweak_key_hash(public_key + 1, root_hash, tweak_key_hash)) {
    return false;
  }

  if (!bip340_point_add_tweak(
          &secp256k1, public_key, tweak_key_hash, tweaked_public_key)) {
    return false;
  }

  return true;
}

// BIP341 private key tweaking for Taproot
bool bip340_tweak_private_key(const uint8_t *private_key,
                              const uint8_t *public_key,
                              const uint8_t *root_hash,
                              uint8_t *tweaked_private_key) {
  if (NULL == private_key || NULL == tweaked_private_key) {
    return false;
  }

  // Compute the tweak hash
  uint8_t tweak_hash[32] = {0};
  if (!bip340_tweak_key_hash(public_key + 1, root_hash, tweak_hash)) {
    return false;
  }

  bignum256 sk = {0}, t = {0}, result = {0};
  bn_read_be(private_key, &sk);
  const ecdsa_curve *curve = &secp256k1;
  curve_point public_key_point = {0};
  if (!ecdsa_read_pubkey(curve, public_key, &public_key_point)) {
    return false;
  }

  // Negate private key if public key y is odd
  if (bn_is_odd(&public_key_point.y)) {
    bn_subtract(&curve->order, &sk, &sk);
    bn_mod(&sk, &curve->order);
  }

  // Compute tweaked private key: sk' = sk + t (mod n)
  bn_read_be(tweak_hash, &t);
  bn_mod(&t, &curve->order);
  bn_copy(&sk, &result);
  bn_add(&result, &t);
  bn_mod(&result, &curve->order);

  bn_write_be(&result, tweaked_private_key);

  // Clear sensitive data
  memzero(&sk, sizeof(sk));
  memzero(&t, sizeof(t));
  memzero(&result, sizeof(result));
  memzero(tweak_hash, sizeof(tweak_hash));

  return true;
}

int btc_get_taproot_address(uint8_t *public_key,
                            const char *hrp,
                            char *address) {
  if (NULL == public_key || NULL == hrp || NULL == address) {
    return 0;
  }

  uint8_t tweaked_public_key[32] = {0};
  if (!bip340_tweak_public_key(public_key, NULL, tweaked_public_key)) {
    return 0;
  }

  return segwit_addr_encode(address, hrp, 1, tweaked_public_key, 32);
}
