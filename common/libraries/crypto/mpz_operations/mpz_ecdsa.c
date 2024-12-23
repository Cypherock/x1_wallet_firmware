/*
 * @author  Cypherock X1 Team
 * @brief   ec operations using mpz nums
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
#include "mpz_ecdsa.h"

#include <stdbool.h>

#include "assert_conf.h"
#include "mini-gmp.h"

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

void mpz_curve_point_init(mpz_curve_point *p) {
  mpz_init(p->x);
  mpz_init(p->y);
}

void mpz_curve_point_clear(mpz_curve_point *p) {
  mpz_clear(p->x);
  mpz_clear(p->y);
}

// Set cp2 = cp1
void mpz_curve_point_copy(const mpz_curve_point *cp1, mpz_curve_point *cp2) {
  mpz_set(cp2->x, cp1->x);
  mpz_set(cp2->y, cp1->y);
}

void mpz_curve_point_add(const mpz_curve *curve,
                         const mpz_curve_point *cp1,
                         mpz_curve_point *cp2) {
  mpz_t lambda, inv, xr, yr;

  mpz_init(lambda);
  mpz_init(inv);
  mpz_init(xr);
  mpz_init(yr);

  if (mpz_curve_point_is_infinity(cp1)) {
    mpz_clear(lambda);
    mpz_clear(inv);
    mpz_clear(xr);
    mpz_clear(yr);
    return;
  }
  if (mpz_curve_point_is_infinity(cp2)) {
    mpz_curve_point_copy(cp1, cp2);
    // clear mpz vars
    mpz_clear(lambda);
    mpz_clear(inv);
    mpz_clear(xr);
    mpz_clear(yr);
    return;
  }
  if (mpz_curve_point_is_equal(cp1, cp2)) {
    mpz_curve_point_double(curve, cp2);
    // clear mpz vars
    mpz_clear(lambda);
    mpz_clear(inv);
    mpz_clear(xr);
    mpz_clear(yr);
    return;
  }
  if (mpz_curve_point_is_negative_of(cp1, cp2)) {
    mpz_curve_point_set_infinity(cp2);
    // clear mpz vars
    mpz_clear(lambda);
    mpz_clear(inv);
    mpz_clear(xr);
    mpz_clear(yr);
    return;
  }

  // inv = (cp2->x - cp1->x) mod prime
  mpz_sub(inv, cp2->x, cp1->x);
  mpz_mod(inv, inv, curve->prime);

  // inv = inv^-1 mod prime
  mpz_invert(inv, inv, curve->prime);

  // lambda = (cp2->y - cp1->y) mod prime
  mpz_sub(lambda, cp2->y, cp1->y);
  mpz_mod(lambda, lambda, curve->prime);

  // lambda = lambda * inv mod prime
  mpz_mul(lambda, lambda, inv);
  mpz_mod(lambda, lambda, curve->prime);

  // xr = lambda^2 - cp1->x - cp2->x mod prime
  mpz_mul(xr, lambda, lambda);
  mpz_sub(xr, xr, cp1->x);
  mpz_sub(xr, xr, cp2->x);
  mpz_mod(xr, xr, curve->prime);

  // yr = lambda * (cp1->x - xr) - cp1->y mod prime
  mpz_sub(yr, cp1->x, xr);
  mpz_mul(yr, yr, lambda);
  mpz_sub(yr, yr, cp1->y);
  mpz_mod(yr, yr, curve->prime);

  // Set cp2 to the result
  mpz_set(cp2->x, xr);
  mpz_set(cp2->y, yr);

  mpz_clear(lambda);
  mpz_clear(inv);
  mpz_clear(xr);
  mpz_clear(yr);
}

void mpz_curve_point_double(const mpz_curve *curve, mpz_curve_point *cp) {
  // Ref:
  // https://github.com/starkware-libs/starkex-for-spot-trading/blob/607f0b4ce507e1d95cd018d206a2797f6ba4aab4/src/starkware/crypto/starkware/crypto/signature/math_utils.py
  ASSERT(mpz_cmp_ui(cp->y, 0) != 0);

  mpz_t lambda, xr, yr, inv;
  mpz_init(lambda);
  mpz_init(xr);
  mpz_init(yr);
  mpz_init(inv);

  // lambda = (3 * cp->x^2 + curve->a) * (2 * cp->y)^-1 mod prime
  mpz_mul(lambda, cp->x, cp->x);
  mpz_mul_ui(lambda, lambda, 3);
  mpz_add(lambda, lambda, curve->a);
  mpz_mul_ui(inv, cp->y, 2);    // using inv to store 2 * y
  mpz_invert(inv, inv, curve->prime);
  mpz_mul(lambda, lambda, inv);
  mpz_mod(lambda, lambda, curve->prime);

  // xr = lambda^2 - 2 * cp->x mod prime
  mpz_mul(xr, lambda, lambda);
  mpz_submul_ui(xr, cp->x, 2);
  mpz_mod(xr, xr, curve->prime);

  // yr = lambda * (cp->x - xr) - cp->y mod prime
  mpz_sub(yr, cp->x, xr);
  mpz_mul(yr, yr, lambda);
  mpz_sub(yr, yr, cp->y);
  mpz_mod(yr, yr, curve->prime);

  mpz_set(cp->x, xr);
  mpz_set(cp->y, yr);

  mpz_clear(lambda);
  mpz_clear(xr);
  mpz_clear(yr);
  mpz_clear(inv);
}

// set point to internal representation of point at infinity
void mpz_curve_point_set_infinity(mpz_curve_point *p) {
  mpz_set_ui(p->x, 0);
  mpz_set_ui(p->y, 0);
}

// return true iff p represent point at infinity
// both coords are zero in internal representation
int mpz_curve_point_is_infinity(const mpz_curve_point *p) {
  return mpz_cmp_ui(p->x, 0) == 0 && mpz_cmp_ui(p->y, 0) == 0;
}

// return true iff both points are equal
int mpz_curve_point_is_equal(const mpz_curve_point *p,
                             const mpz_curve_point *q) {
  return (mpz_cmp(p->x, q->x) == 0) && (mpz_cmp(p->y, q->y) == 0);
}

// returns true iff p == -q
// expects p and q be valid points on curve other than point at infinity
int mpz_curve_point_is_negative_of(const mpz_curve_point *p,
                                   const mpz_curve_point *q) {
  // if P == (x, y), then -P would be (x, -y) on this curve
  if (mpz_cmp(p->x, q->x) != 0) {
    return 0;
  }

  // we shouldn't hit this for a valid point
  if (mpz_cmp_ui(p->y, 0) == 0) {
    return 0;
  }

  return mpz_cmp(p->y, q->y) != 0;
}

void mpz_curve_point_multiply(const mpz_curve *curve,
                              const mpz_t k,
                              const mpz_curve_point *p,
                              mpz_curve_point *res) {
  // Ref: https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication

  mpz_curve_point temp;
  mpz_curve_point R;
  mpz_curve_point_init(&temp);
  mpz_curve_point_init(&R);
  mpz_curve_point_set_infinity(&R);    // Initialize R to the point at infinity
  mpz_curve_point_copy(p, &temp);      // Copy the input point p to temp

  // Iterate over each bit of k from the least significant to the most
  // significant
  for (int i = 0; i < 256; i++) {
    // If the i-th bit of k is set, add temp to the result R
    if (mpz_tstbit(k, i)) {
      mpz_curve_point_add(curve, &temp, &R);
    }

    // Double the current point temp
    mpz_curve_point_double(curve, &temp);
  }

  // Copy the result R to the output parameter res
  mpz_curve_point_copy(&R, res);

  mpz_curve_point_clear(&temp);
  mpz_curve_point_clear(&R);
}

int bn_bit_length(const mpz_t k) {
  if (mpz_cmp_ui(k, 0) == 0) {
    return 0;
  }
  return mpz_sizeinbase(k, 2);
}

int bn_is_bit_set(const mpz_t k, int bit_idx) {
  return mpz_tstbit(k, bit_idx);
}
