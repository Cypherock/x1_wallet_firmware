/**
 * @file    starknet_poseidon.h
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Starknet POSEIDON
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

#include <error.pb.h>
#include <starknet/sign_txn.pb.h>
#include <stdint.h>

#include "coin_utils.h"
#include "f251.h"
#include "mini-gmp-helpers.h"
#include "poseidon.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define DATA_AVAILABILITY_MODE_BITS 32    // 32 bits for data availability mode
#define MAX_AMOUNT_BITS 64                // 64 bits for max_amount
#define MAX_PRICE_PER_UNIT_BITS 128       // 128 bits for max_price_per_unit
#define RESOURCE_VALUE_OFFSET                                                  \
  (MAX_AMOUNT_BITS + MAX_PRICE_PER_UNIT_BITS)    // Combined offset
#define L1_GAS_NAME "4c315f474153"    // The constant value for L1_GAS_NAME
#define L2_GAS_NAME "4c325f474153"
#define INVOKE_TXN_PREFIX                                                      \
  { 0x69, 0x6e, 0x76, 0x6f, 0x6b, 0x65 }    // 0x696e766f6b65; 'INKVOKE'
#define DEPLOY_ACCOUNT_PREFIX                                                  \
  {                                                                            \
    0x64, 0x65, 0x70, 0x6c, 0x6f, 0x79, 0x5f, 0x61, 0x63, 0x63, 0x6f, 0x75,    \
        0x6e, 0x74                                                             \
  }    // 0x6465706c6f795f6163636f756e74 'DEPLOY_ACCOUNT'

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
 * GLOBAL FUNCTIONS
 *****************************************************************************/

/**
 * @brief Function to convert Little-Endian felt_t to Big-Endian hex.
 *
 */
void felt_t_to_hex(const felt_t felt, uint8_t hex[32]);

/**
 * @brief Calculates txn hash for sign based on txn type
 *
 * @param txn pointer to input txn
 * @param type type of txn (INVOKE or DEPLOY)
 * @param hash Calculated hash of txn
 *
 */
void calculate_txn_hash(void *txn, pb_size_t type, felt_t hash);
