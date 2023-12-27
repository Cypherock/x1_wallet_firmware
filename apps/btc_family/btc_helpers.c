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

#include "btc_priv.h"
#include "coin_utils.h"
#include "flash_config.h"
#include "segwit_addr.h"

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
