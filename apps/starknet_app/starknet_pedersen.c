/**
 * @file    starknet_pedersen.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Starknet pedersen hashing
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

#include "starknet_pedersen.h"

#include <error.pb.h>
#include <stdint.h>

#include "coin_utils.h"
#include "mini-gmp-helpers.h"
#include "starknet_context.h"
#include "starknet_helpers.h"

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
void starknet_uli_to_bn_byte_array(const unsigned long int ui,
                                   uint8_t *bn_array) {
  mpz_t bn;
  mpz_init(bn);
  mpz_set_ui(bn, ui);

  memzero(bn_array, STARKNET_BIGNUM_SIZE);
  mpz_to_byte_array(bn, bn_array, STARKNET_BIGNUM_SIZE);

  // clear mpz vars
  mpz_clear(bn);
}

void compute_hash_on_elements(uint8_t data[][STARKNET_BIGNUM_SIZE],
                              uint8_t num_elem,
                              uint8_t *hash) {
  uint8_t result[STARKNET_BIGNUM_SIZE];
  memzero(result, STARKNET_BIGNUM_SIZE);

  for (uint8_t index = 0; index < num_elem; index++) {
    pederson_hash(result, data[index], STARKNET_BIGNUM_SIZE, result);
  }

  uint8_t num_elem_bn[32];
  starknet_uli_to_bn_byte_array(num_elem, num_elem_bn);

  pederson_hash(result, num_elem_bn, STARKNET_BIGNUM_SIZE, result);

  memcpy(hash, result, STARKNET_BIGNUM_SIZE);

  return;
}
