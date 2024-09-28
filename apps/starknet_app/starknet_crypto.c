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

#include "bignum_internal.h"

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
stark_curve *starkCurve;
stark_pedersen *starkPts;

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void stark_curve_init();
static void stark_pedersen_init();
static void print_stark_curve();
static void print_stark_perdersen();

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void starknet_init() {
  stark_curve_init();
  stark_pedersen_init();
}

static void stark_curve_init() {
  static stark_curve stark256;
  char str[STARK_BN_LEN] = {0};

  /* stark_curve_params ref:
  https://github.com/xJonathanLEI/starknet-rs/blob/f31e426a65225b9830bbf3c148f7ea05bf9dc257/starknet-curve/src/curve_params.rs

  struct bn prime;         // prime order of the finite field
  stark_point G;           // initial curve point
  struct bn order;         // order of G
  struct bn order_half;    // order of G divided by 2
  struct bn a;             // coefficient 'a' of the elliptic curve OR alpha
  struct bn b;             // coefficient 'b' of the elliptic curve OR beta
  */

  // Prime
  bignum_from_string(
      &stark256.prime,
      "0800000000000011000000000000000000000000000000000000000000000001",
      STARK_BN_LEN);

  // Generator_point x
  bignum_from_string(
      &stark256.G.x,
      "01EF15C18599971B7BECED415A40F0C7DEACFD9B0D1819E03D723D8BC943CFCA",
      STARK_BN_LEN);

  // Generator_point y
  bignum_from_string(
      &stark256.G.y,
      "005668060AA49730B7BE4801DF46EC62DE53ECD11ABE43A32873000C36E8DC1F",
      STARK_BN_LEN);

  // Order
  bignum_from_string(
      &stark256.order,
      "0800000000000010ffffffffffffffffb781126dcae7b2321e66a241adc64d2f",
      STARK_BN_LEN);

  // Order half
  bignum_from_string(
      &stark256.order_half,
      "04000000000000087fffffffffffffffdbc08936e573d9190f335120d6e32697",
      STARK_BN_LEN);

  // Alpha
  bignum_from_string(
      &stark256.a,
      "0000000000000000000000000000000000000000000000000000000000000001",
      STARK_BN_LEN);

  // Beta
  bignum_from_string(
      &stark256.b,
      "06f21413efbe40de150e596d72f7a8c5609ad26c15c915c1f4cdfcb99cee9e89",
      STARK_BN_LEN);

  starkCurve = &stark256;
  print_stark_curve();
}

static void stark_pedersen_init() {
  // Ref: https://docs.starkware.co/starkex/crypto/pedersen-hash-function.html

  static stark_pedersen pedersen;
  char str[STARK_BN_LEN] = {0};

  // Shift_point
  // x
  bignum_from_string(
      &pedersen.P[0].x,
      "049ee3eba8c1600700ee1b87eb599f16716b0b1022947733551fde4050ca6804",
      STARK_BN_LEN);
  // y
  bignum_from_string(
      &pedersen.P[0].y,
      "3ca0cfe4b3bc6ddf346d49d06ea0ed34e621062c0e056c1d0405d266e10268a",
      STARK_BN_LEN);

  // Perderen_point_1
  // x
  bignum_from_string(
      &pedersen.P[1].x,
      "0234287dcbaffe7f969c748655fca9e58fa8120b6d56eb0c1080d17957ebe47b",
      STARK_BN_LEN);
  // y
  bignum_from_string(
      &pedersen.P[1].y,
      "03b056f100f96fb21e889527d41f4e39940135dd7a6c94cc6ed0268ee89e5615",
      STARK_BN_LEN);

  // Perderen_point_2
  // x
  bignum_from_string(
      &pedersen.P[2].x,
      "04ba4cc166be8dec764910f75b45f74b40c690c74709e90f3aa372f0bd2d6997",
      STARK_BN_LEN);
  // y
  bignum_from_string(
      &pedersen.P[2].y,
      "0040301cf5c1751f4b971e46c4ede85fcac5c59a5ce5ae7c48151f27b24b219c",
      STARK_BN_LEN);

  // Perderen_point_3
  // x
  bignum_from_string(
      &pedersen.P[3].x,
      "054302dcb0e6cc1c6e44cca8f61a63bb2ca65048d53fb325d36ff12c49a58202",
      STARK_BN_LEN);
  // y
  bignum_from_string(
      &pedersen.P[3].y,
      "01b77b3e37d13504b348046268d8ae25ce98ad783c25561a879dcc77e99c2426",
      STARK_BN_LEN);

  // Perderen_point_4
  // x
  bignum_from_string(
      &pedersen.P[4].x,
      "004FA56F376C83DB33F9DAB2656558F3399099EC1DE5E3018B7A6932DBA8AA378",
      STARK_BN_LEN);
  // y
  bignum_from_string(
      &pedersen.P[4].y,
      "003FA0984C931C9E38113E0C0E47E4401562761F92A7A23B45168F4E80FF5B54D",
      STARK_BN_LEN);

  starkPts = &pedersen;
  print_stark_perdersen();
}

static void print_stark_curve() {
  char str[STARK_BN_LEN];

  bignum_to_string(&starkCurve->prime, str, STARK_BN_LEN);
  printf("\nPrime: %s", str);

  bignum_to_string(&starkCurve->G.x, str, STARK_BN_LEN);
  printf("\nG x  : %s", str);

  bignum_to_string(&starkCurve->G.y, str, STARK_BN_LEN);
  printf("\nG y  : %s", str);

  bignum_to_string(&starkCurve->order, str, STARK_BN_LEN);
  printf("\nOrder: %s", str);

  bignum_to_string(&starkCurve->order_half, str, STARK_BN_LEN);
  printf("\nOhalf: %s", str);

  bignum_to_string(&starkCurve->a, str, STARK_BN_LEN);
  printf("\nAlpha: %s", str);

  bignum_to_string(&starkCurve->b, str, STARK_BN_LEN);
  printf("\nBeta : %s\n", str);
}

static void print_stark_perdersen() {
  char str[STARK_BN_LEN];

  for (int i = 0; i < 5; i++) {
    bignum_to_string(&starkPts->P[i].x, str, STARK_BN_LEN);
    printf("P%d x: %s", i, str);

    bignum_to_string(&starkPts->P[i].y, str, STARK_BN_LEN);
    printf(", y: %s\n", str);
  }
}

// Set cp2 = cp1
void stark_point_copy(const stark_point *cp1, stark_point *cp2) {
  *cp2 = *cp1;
}

// internal-bignum
void stark_point_add(const stark_curve *curve,
                     const stark_point *cp1,
                     stark_point *cp2,
                     stark_point *res) {
  struct bn inv = {0}, xr = {0}, yr = {0};
  BigInt1024 prime = {0}, x1 = {0}, x2 = {0}, y1 = {0}, y2 = {0}, res_i = {0},
             lambda_i = {0}, inv_i = {0}, xr_i = {0}, yr_i = {0};

  char str[100] = {0};

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

  /// convert to internal-bignum struct
  bignum_to_string(&curve->prime, str, STARK_BN_LEN);
  bignumFromHexString(&prime, str, STARK_BN_LEN);

  bignum_to_string(&(cp2->x), str, STARK_BN_LEN);
  bignumFromHexString(&x1, str, STARK_BN_LEN);
  bignum_to_string(&(cp1->x), str, STARK_BN_LEN);
  bignumFromHexString(&x2, str, STARK_BN_LEN);

  bignum_to_string(&(cp2->y), str, STARK_BN_LEN);
  bignumFromHexString(&y1, str, STARK_BN_LEN);
  bignum_to_string(&(cp1->y), str, STARK_BN_LEN);
  bignumFromHexString(&y2, str, STARK_BN_LEN);

  // bignum_subtractmod(&(cp2->x), &(cp1->x), &inv, &curve->prime);
  inv_i = subtractBigInt(&x1, &x2);
  bn_mod_internal(&inv_i, &prime);

  /// convert to tiny-bignum struct
  bignumToHexString(&inv_i, str, STARK_BN_LEN);
  bignum_from_string(&inv, str, STARK_BN_LEN);

  bn_inverse(&inv, &curve->prime);    // TODO: In internal bignum lib

  // convert to internal-bignum struct
  bignum_to_string(&inv_i, str, STARK_BN_LEN);
  bignumFromHexString(&inv, str, STARK_BN_LEN);

  // bignum_subtractmod(&(cp2->y), &(cp1->y), &lambda, &curve->prime);
  lambda_i = subtractBigInt(&y1, &y2);
  bn_mod_internal(&lambda_i, &prime);

  // bignum_mul(&inv, &lambda, &curve->prime);
  res_i = multiplyFFT(&inv_i, &lambda_i);
  bn_mod_internal(&res, &prime);

  // [xr = lambda^2 - x1 - x2]
  xr_i = lambda_i;
  // bignum_mul(&xr_i, &xr_i, &curve->prime);
  res_i = multiplyFFT(&inv_i, &lambda_i);
  bn_mod_internal(&res, &prime);

  yr_i = x1;

  // bn_addmod(&yr, &(cp2->x), &curve->prime);
  res_i = addBigInt(&yr_i, &x2);
  bn_mod_internal(&lambda_i, &prime);

  // bignum_subtractmod(&xr, &yr, &xr, &curve->prime);
  xr_i = subtractBigInt(&xr_i, &y2);
  bn_mod_internal(&xr_i, &prime);

  /// convert to tiny-bignum struct
  bignumToHexString(&xr_i, str, STARK_BN_LEN);
  bignum_from_string(&xr, str, STARK_BN_LEN);

  bn_fast_mod(&xr, &curve->prime);    // TODO: In internal bignum lib

  /// convert to internal-bignum struct
  bignum_to_string(&inv, str, STARK_BN_LEN);
  bignumFromHexString(&xr_i, str, STARK_BN_LEN);

  bn_mod_internal(&xr_i, &prime);

  // [yr = lambda (x1 - xr) - y1]
  // bignum_subtractmod(&(cp1->x), &xr, &yr, &curve->prime);
  yr_i = subtractBigInt(&x1, &xr_i);
  bn_mod_internal(&yr_i, &prime);

  // bignum_mul(&lambda, &yr, &curve->prime);
  res_i = multiplyFFT(&lambda_i, &yr_i);
  bn_mod_internal(&res, &prime);

  // bignum_subtractmod(&yr, &(cp1->y), &yr, &curve->prime);
  yr_i = subtractBigInt(&xr_i, &y1);
  bn_mod_internal(&yr_i, &prime);

  /// convert to tiny-bignum struct
  bignumToHexString(&yr_i, str, STARK_BN_LEN);
  bignum_from_string(&yr, str, STARK_BN_LEN);

  bignum_mod(&yr, &curve->prime, &yr);    // TODO: In internal bignum lib

  cp2->x = xr;
  cp2->y = yr;

  stark_point_copy(cp2, res);
}

// tiny-bignum
// cp2 = cp1 + cp2
// void stark_point_add(const stark_curve *curve,
//                      const stark_point *cp1,
//                      stark_point *cp2,
//                      stark_point *res) {
//   struct bn lambda = {0}, inv = {0}, xr = {0}, yr = {0};

//   if (stark_point_is_infinity(cp1)) {
//     return;
//   }
//   if (stark_point_is_infinity(cp2)) {
//     stark_point_copy(cp1, cp2);
//     return;
//   }
//   if (stark_point_is_negative_of(cp1, cp2)) {
//     stark_point_set_infinity(cp2);
//     return;
//   }

//   bignum_subtractmod(&(cp2->x), &(cp1->x), &inv, &curve->prime);
//   bn_inverse(&inv, &curve->prime);
//   bignum_subtractmod(&(cp2->y), &(cp1->y), &lambda, &curve->prime);
//   bignum_mul(&inv, &lambda, &curve->prime);

//   // xr = lambda^2 - x1 - x2
//   xr = lambda;
//   bignum_mul(&xr, &xr, &curve->prime);
//   yr = cp1->x;
//   bn_addmod(&yr, &(cp2->x), &curve->prime);
//   bignum_subtractmod(&xr, &yr, &xr, &curve->prime);
//   bn_fast_mod(&xr, &curve->prime);
//   bn_mod(&xr, &curve->prime);

//   // yr = lambda (x1 - xr) - y1
//   bignum_subtractmod(&(cp1->x), &xr, &yr, &curve->prime);
//   bignum_mul(&lambda, &yr, &curve->prime);
//   bignum_subtractmod(&yr, &(cp1->y), &yr, &curve->prime);
//   bignum_mod(&yr, &curve->prime, &yr);

//   cp2->x = xr;
//   cp2->y = yr;

//   stark_point_copy(cp2, res);
// }

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

void stark_point_multiply(const stark_curve *curve,
                          const struct bn *k,
                          const stark_point *p,
                          stark_point *res) {
  stark_point temp;
  stark_point R;
  stark_point_set_infinity(&R);    // Initialize R to the point at infinity
  stark_point_copy(p, &temp);      // Copy the input point p to temp

  // Iterate over each bit of k from the most significant to the least
  // significant
  for (int i = 256 - 1; i >= 0; i--) {
    // Double the current point temp
    stark_point_add(curve, &temp, &temp, &temp);

    // If the i-th bit of k is set, add temp to the result R
    if (bn_is_bit_set(k, i)) {
      stark_point_add(curve, &temp, &R, &R);
    }
  }

  // Copy the result R to the output parameter res
  stark_point_copy(&R, res);
}

int bn_bit_length(const struct bn *k) {
  int bit_length = 0;

  // Start from the most significant element of the array
  for (int i = BN_ARRAY_SIZE - 1; i >= 0; i--) {
    if (k->array[i] != 0) {
      DTYPE_TMP word = k->array[i];
      // Calculate the bit length of this word
      while (word) {
        word >>= 1;
        bit_length++;
      }
      // Add the offset of this word
      bit_length += i * (WORD_SIZE * 8);
      break;
    }
  }

  return bit_length;
}

int bn_is_bit_set(const struct bn *k, int bit_idx) {
  int word_idx =
      bit_idx / (WORD_SIZE * 8);    // Determine which word contains the bit
  int bit_in_word =
      bit_idx % (WORD_SIZE * 8);    // Determine which bit in the word

  // Ensure that word_idx is within bounds
  if (word_idx >= BN_ARRAY_SIZE) {
    return 0;    // Out of bounds, so the bit is not set
  }

  // Check if the specific bit is set in the corresponding word
  return (k->array[word_idx] & ((DTYPE)1 << bit_in_word)) != 0;
}
