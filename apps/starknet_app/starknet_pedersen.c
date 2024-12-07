/**
 * @file    starknet_padersen.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Starknet chains
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
#include <stdint.h>

#include "coin_utils.h"
#include "mini-gmp-helpers.h"
#include "starknet_api.h"
#include "starknet_context.h"
#include "starknet_crypto.h"
#include "starknet_helpers.h"

void process_single_element(mpz_t element,
                            stark_point *p1,
                            stark_point *p2,
                            stark_point *result);
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

// void mpz_to_byte_array(mpz_t num, uint8_t *out, size_t out_size) {
//     size_t countp;

//     // Export the mpz_t value to the byte array
//     mpz_export(out, &countp, 1, 1, 1, 0, num);

//     // Ensure that the output is padded with leading zeros if necessary
//     // If the exported size is smaller than the desired output size, fill in
//     leading zeros
//     if (countp < out_size) {
//         size_t diff = out_size - countp;
//         memmove(out + diff, out, countp);
//         memset(out, 0, diff);
//     }
// }

bool pederson_hash(uint8_t *x, uint8_t *y, uint8_t size, uint8_t *hash) {
  ASSERT(NULL != x);
  ASSERT(NULL != y);
  ASSERT(0 < size);

  // Convert to bn
  mpz_t a, b, result;
  mpz_init(a);
  mpz_init(b);
  mpz_init(result);

  mpz_import(a, size, 1, 1, 1, 0, x);    // Convert x to mpz_t a
  mpz_import(b, size, 1, 1, 1, 0, y);    // Convert y to mpz_t b

  // Get shift point
  stark_point HASH_SHIFT_POINT, P_1, P_2, P_3, P_4;
  stark_point_copy(&starkPts->P[0], &HASH_SHIFT_POINT);
  stark_point_copy(&starkPts->P[1], &P_1);
  stark_point_copy(&starkPts->P[2], &P_2);
  stark_point_copy(&starkPts->P[3], &P_3);
  stark_point_copy(&starkPts->P[4], &P_4);

  // Compute the hash using the Starkware Pedersen hash definition
  stark_point x_part, y_part, hash_point;
  stark_point_init(&x_part);
  stark_point_init(&y_part);
  stark_point_init(&hash_point);

  process_single_element(a, &P_1, &P_2, &x_part);
  process_single_element(b, &P_3, &P_4, &y_part);

  stark_point_add(starkCurve, &HASH_SHIFT_POINT, &x_part);
  stark_point_add(starkCurve, &x_part, &y_part);
  stark_point_copy(&y_part, &hash_point);

  memzero(hash, 32);
  mpz_to_byte_array(hash_point.x, hash, 32);

  return true;
}
void starknet_uli_to_bn_byte_array(const unsigned long int ui,
                                   uint8_t *bn_array) {
  mpz_t bn;
  mpz_init(bn);
  mpz_set_ui(bn, ui);

  memzero(bn_array, STARKNET_BIGNUM_SIZE);
  mpz_to_byte_array(bn, bn_array, STARKNET_BIGNUM_SIZE);
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

void process_single_element(mpz_t element,
                            stark_point *p1,
                            stark_point *p2,
                            stark_point *result) {
  ASSERT(mpz_cmp(element, starkCurve->prime) < 0);

  mpz_t low_part, high_nibble;
  mpz_init(low_part);
  mpz_init(high_nibble);

  // Extract the low 248 bits and high bits from the element
  mpz_t mask;
  mpz_init(mask);
  // Set mask to (1 << 248) - 1
  mpz_ui_pow_ui(mask, 2, 248);    // mask = 2^248
  mpz_sub_ui(mask, mask, 1);      // mask = 2^248 - 1
  // Extract the low 248 bits and high bits from the element
  mpz_and(low_part, element, mask);
  mpz_fdiv_q_2exp(high_nibble, element, LOW_PART_BITS);

  stark_point res1, res2;
  stark_point_init(&res1);
  stark_point_init(&res2);

  stark_point_multiply(starkCurve, low_part, p1, &res1);    // low_part * p1
  stark_point_multiply(
      starkCurve, high_nibble, p2, &res2);    // high_nibble * p2
  stark_point_add(starkCurve, &res1, &res2);

  stark_point_copy(&res2, result);
  // TODO:clear mpz vars
}