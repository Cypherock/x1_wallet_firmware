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

// #include "bignum_internal.h"
#include "mini-gmp.h"
#include <stdio.h>

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
static void print_stark_pedersen();

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void starknet_init() {
  stark_curve_init();
  stark_pedersen_init();
}

void stark_point_init(stark_point *p) {
    mpz_init(p->x);
    mpz_init(p->y);
}

void stark_point_clear(stark_point *p) {
    mpz_clear(p->x);
    mpz_clear(p->y);
}

void stark_pedersen_clear(stark_pedersen *pedersen) {
    for (int i = 0; i < 5; i++) {
        stark_point_clear(&pedersen->P[i]);
    }
}

static void stark_curve_init() {
  static stark_curve stark256;
  // char str[STARK_BN_LEN] = {0};

  /* stark_curve_params ref:
  https://github.com/xJonathanLEI/starknet-rs/blob/f31e426a65225b9830bbf3c148f7ea05bf9dc257/starknet-curve/src/curve_params.rs

  struct bn prime;         // prime order of the finite field
  stark_point G;           // initial curve point
  struct bn order;         // order of G
  struct bn order_half;    // order of G divided by 2
  struct bn a;             // coefficient 'a' of the elliptic curve OR alpha
  struct bn b;             // coefficient 'b' of the elliptic curve OR beta
  */

  // Initialize mpz_t variables in stark256
  mpz_init(stark256.prime);
  mpz_init(stark256.G.x);
  mpz_init(stark256.G.y);
  mpz_init(stark256.order);
  mpz_init(stark256.order_half);
  mpz_init(stark256.a);
  mpz_init(stark256.b);

  // Prime
  mpz_set_str(
    stark256.prime, 
    "0800000000000011000000000000000000000000000000000000000000000001", 
    16);

  // Generator_point x
  mpz_set_str(
    stark256.G.x, 
    "01EF15C18599971B7BECED415A40F0C7DEACFD9B0D1819E03D723D8BC943CFCA", 
    16);

  // Generator_point y
  mpz_set_str(
    stark256.G.y, 
    "005668060AA49730B7BE4801DF46EC62DE53ECD11ABE43A32873000C36E8DC1F", 
    16);

  // Order
  mpz_set_str(
    stark256.order, 
    "0800000000000010ffffffffffffffffb781126dcae7b2321e66a241adc64d2f", 
    16);

  // Order half
  mpz_set_str(
    stark256.order_half, 
    "04000000000000087fffffffffffffffdbc08936e573d9190f335120d6e32697", 
    16);

  // Alpha
  mpz_set_str(
    stark256.a, 
    "0000000000000000000000000000000000000000000000000000000000000001", 
    16);

  // Beta
  mpz_set_str(
    stark256.b, 
    "06f21413efbe40de150e596d72f7a8c5609ad26c15c915c1f4cdfcb99cee9e89", 
    16);

  starkCurve = &stark256;
  print_stark_curve();
}

static void stark_pedersen_init() {
  // Ref: https://docs.starkware.co/starkex/crypto/pedersen-hash-function.html

  static stark_pedersen pedersen;
  // char str[STARK_BN_LEN] = {0};
  // Initialize all mpz_t variables in the pedersen structure
  for (int i = 0; i < 5; i++) {
    mpz_init(pedersen.P[i].x);
    mpz_init(pedersen.P[i].y);
  }

  // Shift_point x
  mpz_set_str(
  pedersen.P[0].x, 
  "049EE3EBA8C1600700EE1B87EB599F16716B0B1022947733551FDE4050CA6804", 
  16);

  // Shift_point y
  mpz_set_str(
  pedersen.P[0].y, 
  "03CA0CFE4B3BC6DDF346D49D06EA0ED34E621062C0E056C1D0405D266E10268A", 
  16);

  // Pedersen_point_1 x
  mpz_set_str(
    pedersen.P[1].x, 
  "0234287DCBAFFE7F969C748655FCA9E58FA8120B6D56EB0C1080D17957EBE47B", 
  16);

  // Pedersen_point_1 y
  mpz_set_str(
  pedersen.P[1].y, 
  "03B056F100F96FB21E889527D41F4E39940135DD7A6C94CC6ED0268EE89E5615", 
  16);

  // Pedersen_point_2 x
  mpz_set_str(
  pedersen.P[2].x, 
  "04BA4CC166BE8DEC764910F75B45F74B40C690C74709E90F3AA372F0BD2D6997", 
  16);

  // Pedersen_point_2 y
  mpz_set_str(
  pedersen.P[2].y, 
  "0040301CF5C1751F4B971E46C4EDE85FCAC5C59A5CE5AE7C48151F27B24B219C", 
  16);

  // Pedersen_point_3 x
  mpz_set_str(
  pedersen.P[3].x, 
  "054302DCB0E6CC1C6E44CCA8F61A63BB2CA65048D53FB325D36FF12C49A58202", 
  16);

  // Pedersen_point_3 y
  mpz_set_str(
  pedersen.P[3].y, 
  "01B77B3E37D13504B348046268D8AE25CE98AD783C25561A879DCC77E99C2426", 
  16);

  // Pedersen_point_4 x
  mpz_set_str(
  pedersen.P[4].x, 
  "004FA56F376C83DB33F9DAB2656558F3399099EC1DE5E3018B7A6932DBA8AA378", 
  16);

  // Pedersen_point_4 y
  mpz_set_str(
  pedersen.P[4].y, 
  "003FA0984C931C9E38113E0C0E47E4401562761F92A7A23B45168F4E80FF5B54D", 
  16);

  starkPts = &pedersen;
  print_stark_pedersen();
}

static void print_stark_curve() {
  char str[STARK_BN_LEN];

  mpz_get_str(str, 16, starkCurve->prime);
  printf("\nPrime: %s", str);

  mpz_get_str(str, 16, starkCurve->G.x);
  printf("\nG x  : %s", str);

  mpz_get_str(str, 16, starkCurve->G.y);
  printf("\nG y  : %s", str);

  mpz_get_str(str, 16, starkCurve->order);
  printf("\nOrder: %s", str);

  mpz_get_str(str, 16, starkCurve->order_half);
  printf("\nOhalf: %s", str);

  mpz_get_str(str, 16, starkCurve->a);
  printf("\nAlpha: %s", str);

  mpz_get_str(str, 16, starkCurve->b);
  printf("\nBeta : %s\n", str);
}

static void print_stark_pedersen() {
    char str[STARK_BN_LEN];

    for (int i = 0; i < 5; i++) {
        mpz_get_str(str, 16, starkPts->P[i].x);
        printf("P%d x: %s", i, str);

        mpz_get_str(str, 16, starkPts->P[i].y);
        printf(", y: %s\n", str);
    }
}

// Set cp2 = cp1
void stark_point_copy(const stark_point *cp1, stark_point *cp2) {
    mpz_set(cp2->x, cp1->x);  
    mpz_set(cp2->y, cp1->y);  
}

void stark_point_add(const stark_curve *curve,
                     const stark_point *cp1,
                     stark_point *cp2) {
    mpz_t lambda, inv, xr, yr;

    mpz_init(lambda);
    mpz_init(inv);
    mpz_init(xr);
    mpz_init(yr);

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

void stark_point_double(const stark_curve *curve, stark_point *cp) {
  
    // Ref: https://github.com/starkware-libs/starkex-for-spot-trading/blob/607f0b4ce507e1d95cd018d206a2797f6ba4aab4/src/starkware/crypto/starkware/crypto/signature/math_utils.py
    if (mpz_cmp_ui(cp->y, 0) == 0) {
      return;
    }

    mpz_t lambda, xr, yr, inv;
    mpz_init(lambda);
    mpz_init(xr);
    mpz_init(yr);
    mpz_init(inv);

    // lambda = (3 * cp->x^2 + curve->a) * (2 * cp->y)^-1 mod prime
    mpz_mul(lambda, cp->x, cp->x);
    mpz_mul_ui(lambda, lambda, 3);
    mpz_add(lambda, lambda, curve->a);
    mpz_mul_ui(inv, cp->y, 2);  // using inv to store 2 * y
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
void stark_point_set_infinity(stark_point *p) {
  mpz_set_ui(p->x, 0);  
  mpz_set_ui(p->y, 0);
}

// return true iff p represent point at infinity
// both coords are zero in internal representation
int stark_point_is_infinity(const stark_point *p) {
  return mpz_cmp_ui(p->x, 0) == 0 && mpz_cmp_ui(p->y, 0) == 0;
}

// return true iff both points are equal
int stark_point_is_equal(const stark_point *p, const stark_point *q) {
    return (mpz_cmp(p->x, q->x) == 0) &&
           (mpz_cmp(p->y, q->y) == 0);
}

// returns true iff p == -q
// expects p and q be valid points on curve other than point at infinity
int stark_point_is_negative_of(const stark_point *p, const stark_point *q) {
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

void stark_point_multiply(const stark_curve *curve,
                          const mpz_t k,
                          const stark_point *p,
                          stark_point *res) {
    // Ref: https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication

    stark_point temp;
    stark_point R;
    stark_point_init(&temp);
    stark_point_init(&R);
    stark_point_set_infinity(&R);    // Initialize R to the point at infinity
    stark_point_copy(p, &temp);      // Copy the input point p to temp

    // Iterate over each bit of k from the least significant to the most significant
    for (int i = 0; i < 256; i++) {
        // If the i-th bit of k is set, add temp to the result R
        if (mpz_tstbit(k, i)) {
            stark_point_add(curve, &temp, &R);
        }

        // Double the current point temp
        stark_point_double(curve, &temp);
    }

    // Copy the result R to the output parameter res
    stark_point_copy(&R, res);

    stark_point_clear(&temp);
    stark_point_clear(&R);
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
