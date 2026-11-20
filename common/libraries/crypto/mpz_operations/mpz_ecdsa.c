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

#include <bignum.h>
#include <../apps/starknet_app/starknet_context.h>

#include "mini-gmp-helpers.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "assert_conf.h"
#include "memzero.h"
#include "mini-gmp.h"
#include "rfc6979.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
/// Random bits of the multiple of the group order added to a secret scalar
#define SCALAR_BLINDING_BITS 64

/// Limbs that hold any value reduced modulo the (at most 256-bit) prime
#define REDUCED_LIMBS ((256 / (8 * sizeof(mp_limb_t))) + 1)

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
/// Point in Jacobian coordinates: (X, Y, Z) is the affine point (X/Z^2, Y/Z^3)
typedef struct {
  mpz_t X, Y, Z;
} mpz_jacobian_point;

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void *mpz_zeroizing_alloc(size_t size) {
  void *p = malloc(size);
  ASSERT(p != NULL);
  return p;
}

// mini-gmp passes the size of the block it allocated, so all of it is wiped
static void mpz_zeroizing_free(void *p, size_t size) {
  if (NULL == p) {
    return;
  }
  memzero(p, size);
  free(p);
}

// realloc could leave the old block in the heap, so it is copied by hand
static void *mpz_zeroizing_realloc(void *old, size_t old_size, size_t size) {
  void *p = mpz_zeroizing_alloc(size);
  if (NULL != old) {
    memcpy(p, old, old_size < size ? old_size : size);
    mpz_zeroizing_free(old, old_size);
  }
  return p;
}

static void mpz_to_bn(bignum256 *bn, const mpz_t mpz) {
  uint8_t out[STARKNET_BIGNUM_SIZE] = {0};
  mpz_to_byte_array(mpz, out, STARKNET_BIGNUM_SIZE);
  bn_read_be(out, bn);
}

static void bn_to_mpz(mpz_t mpz, const bignum256 *bn) {
  uint8_t in[STARKNET_BIGNUM_SIZE] = {0};
  bn_write_be(bn, in);
  mpz_import(mpz, STARKNET_BIGNUM_SIZE, 1, 1, 1, 0, in);
}

// generate K in a deterministic way, according to RFC6979
// http://tools.ietf.org/html/rfc6979
static void generate_k_rfc6979_mpz(mpz_t k, rfc6979_state *state) {
  uint8_t buf[STARKNET_BIGNUM_SIZE] = {0};
  generate_rfc6979(buf, state);
  mpz_import(k, sizeof(buf), 1, 1, 1, 0, buf);
  memzero(buf, sizeof(buf));
}

// generate random K for signing/side-channel noise
static void generate_k_random(bignum256 *k, const bignum256 *prime) {
  do {
    int i = 0;
    for (i = 0; i < 8; i++) {
      k->val[i] = random32() & 0x3FFFFFFF;
    }
    k->val[8] = random32() & 0xFFFF;
    // check that k is in range and not zero.
  } while (bn_is_zero(k) || !bn_is_less(k, prime));
}

static void generate_k_random_mpz(mpz_t k, const mpz_t prime) {
  bignum256 prime_bn, k_bn = {0};
  mpz_to_bn(&prime_bn, prime);
  mpz_to_bn(&k_bn, k);
  generate_k_random(&k_bn, &prime_bn);
  bn_to_mpz(k, &k_bn);
}

static void jacobian_init(mpz_jacobian_point *p) {
  mpz_init(p->X);
  mpz_init(p->Y);
  mpz_init(p->Z);
}

static void jacobian_clear(mpz_jacobian_point *p) {
  mpz_clear(p->X);
  mpz_clear(p->Y);
  mpz_clear(p->Z);
}

static void mod_mul(mpz_t r, const mpz_t a, const mpz_t b, const mpz_t m) {
  mpz_mul(r, a, b);
  mpz_mod(r, r, m);
}

static void mod_sub(mpz_t r, const mpz_t a, const mpz_t b, const mpz_t m) {
  mpz_sub(r, a, b);
  mpz_mod(r, r, m);
}

// p = 2p (dbl-2007-bl, any curve coefficient a)
static void jacobian_double(const mpz_curve *curve, mpz_jacobian_point *p) {
  const mpz_srcptr m = curve->prime;
  mpz_t xx, yy, yyyy, zz, s, t;
  mpz_init(xx);
  mpz_init(yy);
  mpz_init(yyyy);
  mpz_init(zz);
  mpz_init(s);
  mpz_init(t);

  mod_mul(xx, p->X, p->X, m);
  mod_mul(yy, p->Y, p->Y, m);
  mod_mul(yyyy, yy, yy, m);
  mod_mul(zz, p->Z, p->Z, m);

  // s = 2 * ((X + YY)^2 - XX - YYYY)
  mpz_add(s, p->X, yy);
  mod_mul(s, s, s, m);
  mpz_sub(s, s, xx);
  mpz_sub(s, s, yyyy);
  mpz_mul_2exp(s, s, 1);
  mpz_mod(s, s, m);

  // t = 3 * XX + a * ZZ^2 (named M in the formula)
  mod_mul(t, zz, zz, m);
  mod_mul(t, t, curve->a, m);
  mpz_addmul_ui(t, xx, 3);
  mpz_mod(t, t, m);

  // Z3 = (Y + Z)^2 - YY - ZZ
  mpz_add(p->Z, p->Y, p->Z);
  mod_mul(p->Z, p->Z, p->Z, m);
  mpz_sub(p->Z, p->Z, yy);
  mod_sub(p->Z, p->Z, zz, m);

  // X3 = M^2 - 2 * s
  mod_mul(p->X, t, t, m);
  mpz_submul_ui(p->X, s, 2);
  mpz_mod(p->X, p->X, m);

  // Y3 = M * (s - X3) - 8 * YYYY
  mod_sub(s, s, p->X, m);
  mod_mul(p->Y, t, s, m);
  mpz_submul_ui(p->Y, yyyy, 8);
  mpz_mod(p->Y, p->Y, m);

  mpz_clear(xx);
  mpz_clear(yy);
  mpz_clear(yyyy);
  mpz_clear(zz);
  mpz_clear(s);
  mpz_clear(t);
}

// q = p + q (add-2007-bl). p and q must differ and neither may be infinity;
// in the ladder q - p is always the input point, so this holds.
static void jacobian_add(const mpz_curve *curve,
                         const mpz_jacobian_point *p,
                         mpz_jacobian_point *q) {
  const mpz_srcptr m = curve->prime;
  mpz_t z1z1, z2z2, u1, u2, s1, s2, h, i, j, r, v;
  mpz_init(z1z1);
  mpz_init(z2z2);
  mpz_init(u1);
  mpz_init(u2);
  mpz_init(s1);
  mpz_init(s2);
  mpz_init(h);
  mpz_init(i);
  mpz_init(j);
  mpz_init(r);
  mpz_init(v);

  mod_mul(z1z1, p->Z, p->Z, m);
  mod_mul(z2z2, q->Z, q->Z, m);
  mod_mul(u1, p->X, z2z2, m);
  mod_mul(u2, q->X, z1z1, m);
  mod_mul(s1, p->Y, q->Z, m);
  mod_mul(s1, s1, z2z2, m);
  mod_mul(s2, q->Y, p->Z, m);
  mod_mul(s2, s2, z1z1, m);

  mod_sub(h, u2, u1, m);
  // i = (2h)^2, j = h * i, r = 2 * (s2 - s1), v = u1 * i
  mpz_mul_2exp(i, h, 1);
  mod_mul(i, i, i, m);
  mod_mul(j, h, i, m);
  mpz_sub(r, s2, s1);
  mpz_mul_2exp(r, r, 1);
  mpz_mod(r, r, m);
  mod_mul(v, u1, i, m);

  // Z3 = ((Z1 + Z2)^2 - Z1Z1 - Z2Z2) * h
  mpz_add(q->Z, p->Z, q->Z);
  mod_mul(q->Z, q->Z, q->Z, m);
  mpz_sub(q->Z, q->Z, z1z1);
  mpz_sub(q->Z, q->Z, z2z2);
  mod_mul(q->Z, q->Z, h, m);

  // X3 = r^2 - j - 2 * v
  mod_mul(q->X, r, r, m);
  mpz_sub(q->X, q->X, j);
  mpz_submul_ui(q->X, v, 2);
  mpz_mod(q->X, q->X, m);

  // Y3 = r * (v - X3) - 2 * s1 * j
  mod_sub(v, v, q->X, m);
  mod_mul(q->Y, r, v, m);
  mod_mul(s1, s1, j, m);
  mpz_submul_ui(q->Y, s1, 2);
  mpz_mod(q->Y, q->Y, m);

  mpz_clear(z1z1);
  mpz_clear(z2z2);
  mpz_clear(u1);
  mpz_clear(u2);
  mpz_clear(s1);
  mpz_clear(s2);
  mpz_clear(h);
  mpz_clear(i);
  mpz_clear(j);
  mpz_clear(r);
  mpz_clear(v);
}

// Swaps a and b when bit is 1, without branching on bit. Both values must be
// reduced (non-negative and below 2^256).
static void mpz_cswap(mpz_t a, mpz_t b, const mp_limb_t bit) {
  const mp_limb_t mask = (mp_limb_t)0 - bit;
  const unsigned int size_mask = (unsigned int)0 - (unsigned int)bit;

  // the whole limb range is swapped, so both must have it allocated
  if (a->_mp_alloc < (int)REDUCED_LIMBS) {
    mpz_realloc2(a, REDUCED_LIMBS * 8 * sizeof(mp_limb_t));
  }
  if (b->_mp_alloc < (int)REDUCED_LIMBS) {
    mpz_realloc2(b, REDUCED_LIMBS * 8 * sizeof(mp_limb_t));
  }

  for (size_t i = 0; i < REDUCED_LIMBS; i++) {
    mp_limb_t t = mask & (a->_mp_d[i] ^ b->_mp_d[i]);
    a->_mp_d[i] ^= t;
    b->_mp_d[i] ^= t;
  }
  unsigned int t =
      size_mask & ((unsigned int)a->_mp_size ^ (unsigned int)b->_mp_size);
  a->_mp_size = (int)((unsigned int)a->_mp_size ^ t);
  b->_mp_size = (int)((unsigned int)b->_mp_size ^ t);
}

static void jacobian_cswap(mpz_jacobian_point *a,
                           mpz_jacobian_point *b,
                           const mp_limb_t bit) {
  mpz_cswap(a->X, b->X, bit);
  mpz_cswap(a->Y, b->Y, bit);
  mpz_cswap(a->Z, b->Z, bit);
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void mpz_use_zeroizing_allocator(void) {
  mp_set_memory_functions(
      mpz_zeroizing_alloc, mpz_zeroizing_realloc, mpz_zeroizing_free);
}

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

void mpz_curve_point_multiply_secret(const mpz_curve *curve,
                                     const mpz_t k,
                                     const mpz_curve_point *p,
                                     mpz_curve_point *res) {
  mpz_t scalar, blind, lambda;
  mpz_init(scalar);
  mpz_init(blind);
  mpz_init(lambda);

  mpz_mod(scalar, k, curve->order);
  if (mpz_curve_point_is_infinity(p) || 0 == mpz_cmp_ui(scalar, 0)) {
    mpz_curve_point_set_infinity(res);
    mpz_clear(scalar);
    mpz_clear(blind);
    mpz_clear(lambda);
    return;
  }

  // scalar = k + blind * order, with the top blinding bit set so that the
  // ladder always has well over 256 steps
  generate_k_random_mpz(blind, curve->order);
  mpz_fdiv_r_2exp(blind, blind, SCALAR_BLINDING_BITS);
  mpz_setbit(blind, SCALAR_BLINDING_BITS - 1);
  mpz_addmul(scalar, blind, curve->order);

  // R0 = p in random Jacobian coordinates (lambda^2 x, lambda^3 y, lambda)
  mpz_jacobian_point r0, r1;
  jacobian_init(&r0);
  jacobian_init(&r1);
  generate_k_random_mpz(lambda, curve->prime);
  mpz_set(r0.Z, lambda);
  mod_mul(lambda, lambda, lambda, curve->prime);
  mod_mul(r0.X, p->x, lambda, curve->prime);
  mod_mul(lambda, lambda, r0.Z, curve->prime);
  mod_mul(r0.Y, p->y, lambda, curve->prime);

  // R1 = 2p; the top bit of the scalar is set, so the ladder starts at
  // (p, 2p) and never meets the point at infinity
  mpz_set(r1.X, r0.X);
  mpz_set(r1.Y, r0.Y);
  mpz_set(r1.Z, r0.Z);
  jacobian_double(curve, &r1);

  // Montgomery ladder: R1 - R0 == p throughout; every bit does one addition
  // and one doubling, and the bit only selects through a conditional swap
  for (int i = (int)mpz_sizeinbase(scalar, 2) - 2; i >= 0; i--) {
    const mp_limb_t bit = (mp_limb_t)mpz_tstbit(scalar, i);
    jacobian_cswap(&r0, &r1, bit);
    jacobian_add(curve, &r0, &r1);
    jacobian_double(curve, &r0);
    jacobian_cswap(&r0, &r1, bit);
  }

  // back to affine: x = X / Z^2, y = Y / Z^3. Z is 0 only in cases with
  // negligible probability; the result is then infinity (a signer retries)
  if (0 == mpz_invert(lambda, r0.Z, curve->prime)) {
    mpz_curve_point_set_infinity(res);
  } else {
    mpz_t lambda2;
    mpz_init(lambda2);
    mod_mul(lambda2, lambda, lambda, curve->prime);
    mod_mul(res->x, r0.X, lambda2, curve->prime);
    mod_mul(lambda2, lambda2, lambda, curve->prime);
    mod_mul(res->y, r0.Y, lambda2, curve->prime);
    mpz_clear(lambda2);
  }

  jacobian_clear(&r0);
  jacobian_clear(&r1);
  mpz_clear(scalar);
  mpz_clear(blind);
  mpz_clear(lambda);
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

int starknet_sign_digest(const mpz_curve *curve,
                         const uint8_t *priv_key,
                         const uint8_t *digest,
                         uint8_t *sig) {
  int i = 0;
  mpz_curve_point R = {0};
  mpz_t k, z, randk;
  mpz_curve_point_init(&R);
  mpz_t *s = &R.y;
  mpz_init(k);
  mpz_init(z);
  mpz_init(randk);

#if USE_RFC6979
  rfc6979_state rng = {0};
  init_rfc6979(priv_key, digest, &rng);
#endif
  mpz_import(z, STARKNET_BIGNUM_SIZE, 1, 1, 0, 0, digest);
  for (i = 0; i < 10000; i++) {
#if USE_RFC6979
    // generate K deterministically
    generate_k_rfc6979_mpz(k, &rng);

    // k >> 4
    mpz_fdiv_q_2exp(k, k, 4);

    // if k is too big or too small, we don't like it
    if ((mpz_cmp_ui(k, 0) == 0) || !(mpz_cmp(k, curve->order) < 0)) {
      continue;
    }
#else
    // generate random number k
    generate_k_random_mpz(k, curve->order);
#endif
    // compute k*G
    mpz_curve_point_multiply_secret(curve, k, &curve->G, &R);
    mpz_mod(R.x, R.x, curve->order);
    // r = (rx mod n)
    if (!(mpz_cmp(R.x, curve->order) < 0)) {
      mpz_sub(R.x, R.x, curve->order);
    }
    // if r is zero, we retry
    if (mpz_cmp_ui(R.x, 0) == 0) {
      continue;
    }

    // randomize operations to counter side-channel attacks
    generate_k_random_mpz(randk, curve->order);

    // k = k * rand mod n
    mpz_mul(k, k, randk);
    mpz_mod(k, k, curve->order);

    // k = (k * rand)^-1
    mpz_invert(k, k, curve->order);

    mpz_import(*s, STARKNET_BIGNUM_SIZE, 1, 1, 1, 0, priv_key);
    // R.x*priv
    mpz_mul(*s, *s, R.x);
    mpz_mod(*s, *s, curve->order);
    mpz_add(*s, *s, z);    // R.x*priv + z

    // (k*rand)^-1 (R.x*priv + z)
    mpz_mul(*s, *s, k);
    mpz_mod(*s, *s, curve->order);

    // k^-1 (R.x*priv + z)
    mpz_mul(*s, *s, randk);
    mpz_mod(*s, *s, curve->order);

    // if s is zero, we retry
    if ((mpz_cmp_ui(*s, 0) == 0)) {
      continue;
    }

    // if S > order/2 => S = -S
    // if ((mpz_cmp(curve->order_half, *s) < 0)) {
    //   mpz_sub(*s, curve->order, *s);
    // }
    // we are done, R.x and s is the result signature
    mpz_to_byte_array(R.x, sig, STARKNET_BIGNUM_SIZE);
    mpz_to_byte_array(*s, sig + STARKNET_BIGNUM_SIZE, STARKNET_BIGNUM_SIZE);

    // clear all the temporary variables; the limbs are wiped on free by the
    // zeroizing allocator (wiping the mpz_t header first would skip the free)
    mpz_clear(k);
    mpz_clear(randk);
    mpz_clear(z);
    mpz_curve_point_clear(&R);

#if USE_RFC6979
    memzero(&rng, sizeof(rng));
#endif
    return 0;
  }

  // Too many retries without a valid signature
  // -> fail with an error
  mpz_clear(k);
  mpz_clear(randk);
  mpz_clear(z);
  mpz_curve_point_clear(&R);

#if USE_RFC6979
  memzero(&rng, sizeof(rng));
#endif
  return -1;
}
