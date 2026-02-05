/**
 * @file    canton_instruments.c
 * @author  Cypherock X1 Team
 * @brief   Canton whitelisted instrument list
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

#include "canton_instruments.h"

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

const canton_instrument_data_t
    canton_instrument_data[CANTON_WHITELISTED_INSTRUMENT_COUNT] = {
        {{"Amulet",
          "DSO::"
          "1220b1431ef217342db44d516bb9befde802be7d8899637d290895fa58880f19acc"
          "c"},
         "CC",
         10},
        {{"CBTC",
          "cbtc-network::"
          "12205af3b949a04776fc48cdcc05a060f6bda2e470632935f375d1049a8546a3b26"
          "2"},
         "CBTC",
         10},
        {{"USDCx",
          "decentralized-usdc-interchain-rep::"
          "12208115f1e168dd7e792320be9c4ca720c751a02a3053c7606e1c1cd3dad9bf60e"
          "f"},
         "USDCx",
         10},
        {{"USDC",
          "circleDemo_0040circle::"
          "12201ee3da1c10a0b6ca16692ebe08801476ae2a903435e1719435cbe466e72bbd2"
          "6"},
         "CUSD",
         10},
        {{"f29bdd7a-1469-498a-ba2a-796bf5387b31",
          "party-28dc4516-b5ca-44ff-86c7-2107e90a6807::"
          "1220b8301e18aa8a401d6e34e6c20f8b0243183c514373bca8f1b6b9270246341a9"
          "e"},
         "SBC",
         10}};

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
