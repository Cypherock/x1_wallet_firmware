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
 * @brief Checks if the provided 32-bit value has its MSB not set.
 *
 * @return true   If the provided value has MSB set to 0.
 * @return false  If the provided value has MSB set to 1.
 */
static inline bool is_non_hardened(uint32_t x);

/**
 * @brief Checks if the provided 32-bit value has its MSB set.
 *
 * @return true   If the provided value has MSB set to 1.
 * @return false  If the provided value has MSB set to 0.
 */
static inline bool is_hardened(uint32_t x);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static inline bool is_non_hardened(uint32_t x) {
  return ((x & 0x80000000) == 0);
}

static inline bool is_hardened(uint32_t x) {
  return ((x & 0x80000000) == 0x80000000);
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool btc_derivation_path_guard(const uint32_t *path, uint32_t depth) {
  bool status = false;
  if (BTC_ACC_XPUB_DEPTH != depth && BTC_ACC_ADDR_DEPTH != depth) {
    return status;
  }
  status = true;

  // common checks for xpub/account and address nodes
  if (PURPOSE_LEGACY != path[0] && PURPOSE_SEGWIT != path[0] &&
      PURPOSE_NSEGWIT != path[0] && PURPOSE_TAPROOT != path[0]) {
    // purpose index mismatch
    status = false;
  }
  if (COIN_BTC != path[1] || is_non_hardened(path[2])) {
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