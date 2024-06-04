/**
 * @file    ltc_app.c
 * @author  Cypherock X1 Team
 * @brief   Litecoin app configuration
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

#include "ltc_app.h"

#include "btc_helpers.h"
#include "btc_main.h"

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
 * @brief Check if the purpose index is supported by the Litecoin app.
 *
 * @param purpose_index The purpose index to be checked
 *
 * @return bool Indicates if the provided purpose index is supported
 * @retval true The provided purpose index is supported
 * @retval false The provided purpose index is not supported
 */
static bool is_purpose_supported(uint32_t purpose_index);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/**
 * Litecoin supports Segwit and Native-Segwit but not Taproot unlike Bitcoin
 * Values can be verified from the following references:
 * https://github.com/litecoin-project/litecoin/blob/5ac781487cc9589131437b23c69829f04002b97e/src/chainparams.cpp#L65-L177
 * Secondary reference for aggregated coin info:
 * https://github.com/trezor/trezor-firmware/blob/a4034097d6386fd938a23e888fc85b7d2ca13df3/common/defs/bitcoin/litecoin.json
 */
const btc_config_t ltc_app_config = {
    .coin_type = COIN_LTC,
    .p2pkh_addr_ver = 0x30,
    .p2sh_addr_ver = 0x32,
    .legacy_xpub_ver = 0x19da462,
    .segwit_xpub_ver = 0x1b26ef6,
    .nsegwit_xpub_ver = 0x4b24746,
    .bech32_hrp = "ltc",
    .lunit_name = "LTC",
    .name = "Litecoin",

    .max_fee = 67000000,

    .is_purpose_supported = is_purpose_supported,
};

static const cy_app_desc_t ltc_app_desc = {.id = 4,
                                           .version =
                                               {
                                                   .major = 1,
                                                   .minor = 1,
                                                   .patch = 0,
                                               },
                                           .app = btc_main,
                                           .app_config = &ltc_app_config};

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool is_purpose_supported(uint32_t purpose_index) {
  if (PURPOSE_LEGACY != purpose_index && PURPOSE_SEGWIT != purpose_index &&
      PURPOSE_NSEGWIT != purpose_index) {
    return false;
  }
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

const cy_app_desc_t *get_ltc_app_desc() {
  return &ltc_app_desc;
}
