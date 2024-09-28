/**
 * @file    xrp_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Xrp chains
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2024 by HODL TECH PTE LTD
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

#include "xrp_helpers.h"

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

const char xrp_b58digits_ordered[] =
    "rpshnaf39wBUDNEGHJKLM4PQRST7VWXYZ2bcdeCg65jkm8oFqi1tuvAxyz";

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool xrp_derivation_path_guard(const uint32_t *path, uint8_t levels) {
  bool status = false;
  if (levels != XRP_IMPLICIT_ACCOUNT_DEPTH) {
    return status;
  }

  uint32_t purpose = path[0], coin = path[1], account = path[2],
           change = path[3], address = path[4];

  // m/44'/144'/0'/0/i
  status = (XRP_PURPOSE_INDEX == purpose && XRP_COIN_INDEX == coin &&
            XRP_ACCOUNT_INDEX == account && XRP_CHANGE_INDEX == change &&
            is_non_hardened(address));

  return status;
}

bool xrp_b58enc(char *b58, size_t *b58sz, const void *data, size_t binsz) {
  const uint8_t *bin = data;
  int carry = 0;
  size_t i = 0, j = 0, high = 0, zcount = 0;
  size_t size = 0;

  while (zcount < binsz && !bin[zcount])
    ++zcount;

  size = (binsz - zcount) * 138 / 100 + 1;
  uint8_t buf[size];
  memzero(buf, size);

  for (i = zcount, high = size - 1; i < binsz; ++i, high = j) {
    for (carry = bin[i], j = size - 1; (j > high) || carry; --j) {
      carry += 256 * buf[j];
      buf[j] = carry % 58;
      carry /= 58;
      if (!j) {
        // Otherwise j wraps to maxint which is > high
        break;
      }
    }
  }

  for (j = 0; j < size && !buf[j]; ++j)
    ;

  if (*b58sz <= zcount + size - j) {
    *b58sz = zcount + size - j + 1;
    return false;
  }

  if (zcount)
    memset(b58, 'r', zcount);
  for (i = zcount; j < size; ++i, ++j)
    b58[i] = xrp_b58digits_ordered[buf[j]];
  b58[i] = '\0';
  *b58sz = i + 1;

  return true;
}

int xrp_base58_encode_check(const uint8_t *data,
                            int datalen,
                            HasherType hasher_type,
                            char *str,
                            int strsize) {
  if (datalen > 128) {
    return 0;
  }
  uint8_t buf[datalen + 32];
  memset(buf, 0, sizeof(buf));
  uint8_t *hash = buf + datalen;
  memcpy(buf, data, datalen);
  hasher_Raw(hasher_type, data, datalen, hash);
  size_t res = strsize;
  bool success = xrp_b58enc(str, &res, buf, datalen + 4);
  memzero(buf, sizeof(buf));
  return success ? res : 0;
}
