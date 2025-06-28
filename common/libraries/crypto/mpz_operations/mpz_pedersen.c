
/*
 * @author  Cypherock X1 Team
 * @brief   pedersen hashing alogrithms
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
#include "../apps/starknet_app/starknet_api.h"
#include "../apps/starknet_app/starknet_context.h"
#include "../apps/starknet_app/starknet_crypto.h"
#include "../apps/starknet_app/starknet_helpers.h"
#include "../apps/starknet_app/starknet_pedersen.h"

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
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void process_single_element(mpz_t element,
                            mpz_curve_point *p1,
                            mpz_curve_point *p2,
                            mpz_curve_point *result) {
  ASSERT(mpz_cmp(element, stark_curve->prime) < 0);

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

  mpz_curve_point res1, res2;
  mpz_curve_point_init(&res1);
  mpz_curve_point_init(&res2);

  mpz_curve_point_multiply(
      stark_curve, low_part, p1, &res1);    // low_part * p1
  mpz_curve_point_multiply(
      stark_curve, high_nibble, p2, &res2);    // high_nibble * p2
  mpz_curve_point_add(stark_curve, &res1, &res2);

  mpz_curve_point_copy(&res2, result);

  // clear mpz vars
  mpz_clear(low_part);
  mpz_clear(high_nibble);
  mpz_clear(mask);

  mpz_curve_point_clear(&res1);
  mpz_curve_point_clear(&res2);
}

void pederson_hash(uint8_t *x, uint8_t *y, uint8_t size, uint8_t *hash) {
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
  mpz_curve_point HASH_SHIFT_POINT, P_1, P_2, P_3, P_4;
  mpz_curve_point_init(&HASH_SHIFT_POINT);
  mpz_curve_point_init(&P_1);
  mpz_curve_point_init(&P_2);
  mpz_curve_point_init(&P_3);
  mpz_curve_point_init(&P_4);

  mpz_curve_point_copy(&starknet_pedersen_points->P[0], &HASH_SHIFT_POINT);
  mpz_curve_point_copy(&starknet_pedersen_points->P[1], &P_1);
  mpz_curve_point_copy(&starknet_pedersen_points->P[2], &P_2);
  mpz_curve_point_copy(&starknet_pedersen_points->P[3], &P_3);
  mpz_curve_point_copy(&starknet_pedersen_points->P[4], &P_4);

  // Compute the hash using the Starkware Pedersen hash definition
  mpz_curve_point x_part, y_part, hash_point;
  mpz_curve_point_init(&x_part);
  mpz_curve_point_init(&y_part);
  mpz_curve_point_init(&hash_point);

  process_single_element(a, &P_1, &P_2, &x_part);
  process_single_element(b, &P_3, &P_4, &y_part);

  mpz_curve_point_add(stark_curve, &HASH_SHIFT_POINT, &x_part);
  mpz_curve_point_add(stark_curve, &x_part, &y_part);
  mpz_curve_point_copy(&y_part, &hash_point);

  memzero(hash, 32);
  mpz_to_byte_array(hash_point.x, hash, 32);

  // clear curve points
  mpz_curve_point_clear(&x_part);
  mpz_curve_point_clear(&y_part);
  mpz_curve_point_clear(&hash_point);

  mpz_clear(a);
  mpz_clear(b);
  mpz_clear(result);

  mpz_curve_point_clear(&HASH_SHIFT_POINT);
  mpz_curve_point_clear(&P_1);
  mpz_curve_point_clear(&P_2);
  mpz_curve_point_clear(&P_3);
  mpz_curve_point_clear(&P_4);
}
