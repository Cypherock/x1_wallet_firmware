/**
 * @file    starknet_crypto.c
 * @author  Cypherock X1 Team
 * @brief   Crypto specific to Starknet chain
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

#include "starknet_crypto.h"

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

const stark_curve stark256 = {
    /* .prime */ {/*.val =*/{0x3ffffc2f,
                             0x3ffffffb,
                             0x3fffffff,
                             0x3fffffff,
                             0x3fffffff,
                             0x3fffffff,
                             0x3fffffff,
                             0x3fffffff,
                             0xffff}},

    /* G */
    {/*.x =*/{/*.val =*/{0x16f81798,
                         0x27ca056c,
                         0x1ce28d95,
                         0x26ff36cb,
                         0x70b0702,
                         0x18a573a,
                         0xbbac55a,
                         0x199fbe77,
                         0x79be}},
     /*.y =*/{/*.val =*/{0x3b10d4b8,
                         0x311f423f,
                         0x28554199,
                         0x5ed1229,
                         0x1108a8fd,
                         0x13eff038,
                         0x3c4655da,
                         0x369dc9a8,
                         0x483a}}},

    /* order */
    {/*.val =*/{0x10364141,
                0x3f497a33,
                0x348a03bb,
                0x2bb739ab,
                0x3ffffeba,
                0x3fffffff,
                0x3fffffff,
                0x3fffffff,
                0xffff}},

    /* order_half */
    {/*.val =*/{0x281b20a0,
                0x3fa4bd19,
                0x3a4501dd,
                0x15db9cd5,
                0x3fffff5d,
                0x3fffffff,
                0x3fffffff,
                0x3fffffff,
                0x7fff}},

    /* a */
    {},

    /* b */ {/*.val =*/{7}}};

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

// Set cp2 = cp1
void stark_point_copy(const stark_point *cp1, stark_point *cp2) {
  *cp2 = *cp1;
}

// cp2 = cp1 + cp2
void stark_point_add(const stark_curve *curve,
                     const stark_point *cp1,
                     stark_point *cp2) {
  struct bn lambda = {0}, inv = {0}, xr = {0}, yr = {0};

  if (stark_point_is_infinity(cp1)) {
    return;
  }
  if (stark_point_is_infinity(cp2)) {
    stark_point_copy(cp1, cp2);
    return;
  }
  if (stark_point_is_negative_of(cp1, cp2)) {
    stark_point_set_infinity(cp2);
    return;
  }

  bignum_subtractmod(&(cp2->x), &(cp1->x), &inv, &curve->prime);
  bn_inverse(&inv, &curve->prime);
  bignum_subtractmod(&(cp2->y), &(cp1->y), &lambda, &curve->prime);
  bignum_mod_mul(&inv, &lambda, &curve->prime);

  // xr = lambda^2 - x1 - x2
  xr = lambda;
  bignum_mod_mul(&xr, &xr, &curve->prime);
  yr = cp1->x;
  bn_addmod(&yr, &(cp2->x), &curve->prime);
  bignum_subtractmod(&xr, &yr, &xr, &curve->prime);
  bn_fast_mod(&xr, &curve->prime);
  bn_mod(&xr, &curve->prime);

  // yr = lambda (x1 - xr) - y1
  bignum_subtractmod(&(cp1->x), &xr, &yr, &curve->prime);
  bignum_mod_mul(&lambda, &yr, &curve->prime);
  bignum_subtractmod(&yr, &(cp1->y), &yr, &curve->prime);
  bignum_mod(&yr, &curve->prime, &yr);

  cp2->x = xr;
  cp2->y = yr;
}

// set point to internal representation of point at infinity
void stark_point_set_infinity(stark_point *p) {
  bignum_from_int(&(p->x), 0);
  bignum_from_int(&(p->y), 0);
}

// return true iff p represent point at infinity
// both coords are zero in internal representation
int stark_point_is_infinity(const stark_point *p) {
  return bignum_is_zero(&(p->x)) && bignum_is_zero(&(p->y));
}

// return true iff both points are equal
int stark_point_is_equal(const stark_point *p, const stark_point *q) {
  return (bignum_cmp(&(p->x), &(q->x)) == 0) &&
         (bignum_cmp(&(p->y), &(q->y)) == 0);
}

// returns true iff p == -q
// expects p and q be valid points on curve other than point at infinity
int stark_point_is_negative_of(const stark_point *p, const stark_point *q) {
  // if P == (x, y), then -P would be (x, -y) on this curve
  if (bignum_cmp(&(p->x), &(q->x)) != 0) {
    return 0;
  }

  // we shouldn't hit this for a valid point
  if (bignum_is_zero(&(p->y))) {
    return 0;
  }

  return (bignum_cmp(&(p->y), &(q->y)) != 0);
}

// ECDSA multiply; g^k; private to public
void private_to_public_key(const uint8_t *private, uint8_t *public_65) {
  stark_point R = {0}, temp = {0};

  const stark_curve *curve = &stark256;
  stark_point_set_infinity(&R);
  stark_point_copy(&curve->G, &temp);

  for (int i = 255; i >= 0; i--) {
    int offset = i / 8;
    int bit = 7 - (i % 8);
    if (private[offset] & (1 << bit)) {
      // bit is set; do add current doubled value to result
      stark_point_add(curve, &temp, &R);
    }
    stark_point_double(curve, &temp);
  }

  public_65[0] = 0x04;
  //   bn_write_be(&R.x, public_65 + 1);
  //   bn_write_be(&R.y, public_65 + 33);
}
