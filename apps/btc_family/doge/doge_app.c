/**
 * @file    doge_app.c
 * @author  Cypherock X1 Team
 * @brief   Dogecoin app configuration
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

#include "doge_app.h"

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
 * @brief Check if the purpose index is supported by the Dogecoin app.
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
 * Dogecoin only supports legacy addresses. No support for Segwit and
 * Native-Segwit. Hence, bech32_hrp is empty and segwit, native-segwit xpub
 * version is set to 0.
 * Secondary reference for aggregated coin info:
 * https://github.com/trezor/trezor-firmware/blob/f5983e7843f381423f30b8bc2ffc46e496775e5a/common/defs/bitcoin/dogecoin.json
 */
const btc_config_t doge_app_config = {
    .coin_type = COIN_DOGE,
    .p2pkh_addr_ver = 0x1e,
    .p2sh_addr_ver = 0x16,
    .legacy_xpub_ver = 0x2facafd,
    .segwit_xpub_ver = 0x00,
    .nsegwit_xpub_ver = 0x00,
    .bech32_hrp = "",
    .lunit_name = "DOGE",
    .name = "Dogecoin",

    .max_fee = 1200000000000,

    .is_purpose_supported = is_purpose_supported,
};

static const cy_app_desc_t doge_app_desc = {.id = 5,
                                            .version =
                                                {
                                                    .major = 1,
                                                    .minor = 1,
                                                    .patch = 0,
                                                },
                                            .app = btc_main,
                                            .app_config = &doge_app_config};

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool is_purpose_supported(uint32_t purpose_index) {
  if (PURPOSE_LEGACY != purpose_index) {
    return false;
  }
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

const cy_app_desc_t *get_doge_app_desc() {
  return &doge_app_desc;
}
