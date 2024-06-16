/**
 * @file    TRON_contracts.c
 * @author  Cypherock X1 Team
 * @brief   TRON whitelisted contracts list
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

#include <tron_contracts.h>

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

// a9059cbb -> transfer(address,uint256)
const uint8_t selector[TRC20_FUNCTION_SELECTOR_COUNT][4] = {
    {0xA9, 0x05, 0x9C, 0xBB}};

const trc20_contracts_t trc20_contracts[TRC20_WHITELISTED_CONTRACTS_COUNT] = {
    {{0x41, 0xA6, 0x14, 0xF8, 0x03, 0xB6, 0xFD, 0x78, 0x09, 0x86, 0xA4,
      0x2C, 0x78, 0xEC, 0x9C, 0x7F, 0x77, 0xE6, 0xDE, 0xD1, 0x3C},
     "USDT",
     6},
    {{0x41, 0x34, 0x87, 0xb6, 0x3d, 0x30, 0xb5, 0xb2, 0xc8, 0x7f, 0xb7,
      0xff, 0xa8, 0xbc, 0xfa, 0xde, 0x38, 0xea, 0xac, 0x1a, 0xbe},
     "USDC",
     6},
    {{0x41, 0x94, 0xf2, 0x4e, 0x99, 0x2c, 0xa0, 0x4b, 0x49, 0xc6, 0xf2,
      0xa2, 0x75, 0x30, 0x76, 0xef, 0x89, 0x38, 0xed, 0x4d, 0xaa},
     "USDD",
     18},
    {{0x41, 0x03, 0x20, 0x17, 0x41, 0x1f, 0x46, 0x63, 0xb3, 0x17, 0xfe,
      0x77, 0xc2, 0x57, 0xd2, 0x8d, 0x5c, 0xd1, 0xb2, 0x6e, 0x3d},
     "BTT",
     18},
    {{0x41, 0xce, 0xbd, 0xe7, 0x10, 0x77, 0xb8, 0x30, 0xb9, 0x58, 0xc8,
      0xda, 0x17, 0xbc, 0xdd, 0xee, 0xb8, 0x5d, 0x0b, 0xcf, 0x25},
     "TUSD",
     18},
    {{0x41, 0x3d, 0xfe, 0x63, 0x7b, 0x2b, 0x9a, 0xe4, 0x19, 0x0a, 0x45,
      0x8b, 0x5f, 0x3e, 0xfc, 0x19, 0x69, 0xaf, 0xe2, 0x78, 0x19},
     "NFT",
     6},
    {{0x41, 0x18, 0xfd, 0x06, 0x26, 0xda, 0xf3, 0xaf, 0x02, 0x38, 0x9a,
      0xef, 0x3e, 0xd8, 0x7d, 0xb9, 0xc3, 0x3f, 0x63, 0x8f, 0xfa},
     "JST",
     18},
    {{0x41, 0x74, 0x47, 0x2E, 0x7D, 0x35, 0x39, 0x5A, 0x6B, 0x5A, 0xDD,
      0x42, 0x7E, 0xEC, 0xB7, 0xF4, 0xB6, 0x2A, 0xD2, 0xB0, 0x71},
     "WIN",
     6},
    {{0x41, 0x89, 0x1c, 0xdb, 0x91, 0xd1, 0x49, 0xf2, 0x3b, 0x1a, 0x45,
      0xd9, 0xc5, 0xca, 0x78, 0xa8, 0x8d, 0x0c, 0xb4, 0x4c, 0x18},
     "WTRX",
     6},
};

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
