/**
 * @file    shamir_wrapper.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
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
#include "crypto_random.h"
#include "logger.h"
#include "string.h"
#include "utils.h"
#include "wallet.h"

#define print(ARR, SIZE)                                                       \
  {                                                                            \
    for (uint8_t i = 0; i < (SIZE); i++) {                                     \
      printf("%02x", ARR[i]);                                                  \
    }                                                                          \
    printf("\n");                                                              \
  }

/***********************************************/
/**GALOIS**/
// GF(2^8) with the polynomial x^8 + x^4 + x^3 + x + 1. The operations run in
// constant time: no branches or table lookups depend on the (secret) operands.

static uint8_t galois_add(const uint8_t a, const uint8_t b) {
  return a ^ b;
}

static uint8_t galois_diff(const uint8_t a, const uint8_t b) {
  return a ^ b;
}

static uint8_t galois_mul(uint8_t a, uint8_t b) {
  uint8_t result = 0;
  for (uint8_t i = 0; i < 8; i++) {
    result ^= (uint8_t)(-(b & 1)) & a;
    // reduce when the top bit is shifted out
    uint8_t carry = (uint8_t)(-(a >> 7));
    a = (uint8_t)(a << 1) ^ (carry & 0x1B);
    b >>= 1;
  }
  return result;
}

/// a^p for a public exponent p; 0^p is 0 for p > 0
static uint8_t galois_pow(const uint8_t a, const uint16_t p) {
  uint8_t result = 1;
  for (uint16_t i = 0; i < p; i++) {
    result = galois_mul(result, a);
  }
  return result;
}

/// a^-1 = a^254; the result for a == 0 is 0
static uint8_t galois_inv(const uint8_t a) {
  return galois_pow(a, 254);
}

/// a / b; b must not be 0
static uint8_t galois_div(const uint8_t a, const uint8_t b) {
  return galois_mul(a, galois_inv(b));
}

/***********************************************/

static uint8_t eval(const uint16_t n,
                    const uint8_t coeffs[n],
                    const uint8_t x) {
  // calculates  coeff[0]*x^n + ... + coeff[n-2]*x^2 + coeff[n-1]*x^1
  uint8_t ret = 0;
  for (uint16_t i = 0; i < n; i++) {
    ret = galois_add(ret, galois_mul(galois_pow(x, n - i), coeffs[i]));
  }
  return ret;
}

// TODO: To be reviewed function prototype
void convert_to_shares(const uint8_t secret_len,
                       const uint8_t secret[secret_len],
                       const uint8_t total_number_of_shares,
                       const uint8_t threshold_number_of_shares,
                       uint8_t shares_OUT[total_number_of_shares][secret_len]) {
  for (uint8_t j = 0; j < secret_len; j++) {
    uint8_t coeffs[threshold_number_of_shares - 1];
    // coefficients are uniform over the whole field, zero included, so a
    // single share reveals nothing about the secret
    random_generate(coeffs, sizeof(coeffs));
    for (uint8_t i = 0; i < total_number_of_shares; i++) {
      shares_OUT[i][j] = galois_add(
          secret[j],
          eval(threshold_number_of_shares - 1,
               coeffs,
               i + 1));    // galois_add(galois_mul(m, i+1), secret[j]);
    }
    memzero(coeffs, sizeof(coeffs));
  }
}

void recover_share_from_shares(
    const uint8_t number_of_bytes,    // visualise this as horizontal length
    const uint8_t num_shares,    // threshold. shares is a 2D array. visualise
                                 // this as vertical height
    const uint8_t shares[num_shares][number_of_bytes],
    const uint8_t x_coords[num_shares],
    uint8_t share_OUT[number_of_bytes],
    uint8_t out_x_cor) {
  for (uint8_t j = 0; j < number_of_bytes; j++) {
    uint8_t a0, a1;
    a1 = galois_div(galois_diff(shares[0][j], shares[1][j]),
                    galois_diff(x_coords[0], x_coords[1]));
    a0 = galois_diff(shares[0][j], galois_mul(x_coords[0], a1));
    //        share_OUT[j] = (out_x_cor == 0 ? a0 : galois_add(a0, eval(1, &a1,
    //        out_x_cor)));
    share_OUT[j] = galois_add(a0, out_x_cor == 0 ? 0 : eval(1, &a1, out_x_cor));
  }
}

void recover_secret_from_shares(
    const uint8_t number_of_bytes,    // visualise this as horizontal length
    const uint8_t num_shares,    // threshold. shares is a 2D array. visualise
                                 // this as vertical height
    const uint8_t shares[num_shares][number_of_bytes],
    const uint8_t x_coords[num_shares],
    uint8_t secret_OUT[number_of_bytes]) {
  recover_share_from_shares(
      number_of_bytes, num_shares, shares, x_coords, secret_OUT, 0);
}

int verify_shares_NC2(
    const uint8_t number_of_shares,
    const uint8_t secret_size,
    const uint8_t recovered_shamir_data_ver[number_of_shares][secret_size],
    const uint8_t x_coords[number_of_shares],
    uint8_t secret[secret_size]) {
  uint16_t combinations = 0, count = 0;
  combinations = number_of_shares * (number_of_shares - 1) / 2;
  uint8_t pairs[combinations][2];
  uint8_t two_shares[2][secret_size];
  uint8_t two_x_coords[2];
  uint8_t secret_calculated[secret_size];

  for (int i = 0; i < number_of_shares; i++) {
    for (int j = i + 1; j < number_of_shares; j++) {
      pairs[count][0] = i;
      pairs[count][1] = j;
      count++;
    }
  }

  for (int i = 0; i < combinations; i++) {
    memcpy(two_shares[0], recovered_shamir_data_ver[pairs[i][0]], secret_size);
    memcpy(two_shares[1], recovered_shamir_data_ver[pairs[i][1]], secret_size);

    two_x_coords[0] = x_coords[pairs[i][0]];
    two_x_coords[1] = x_coords[pairs[i][1]];

    recover_secret_from_shares(secret_size,
                               MINIMUM_NO_OF_SHARES,
                               two_shares,
                               two_x_coords,
                               secret_calculated);
    // TODO: Restructure if statements to return 0 by default
    if (i == 0) {
      memcpy(secret, secret_calculated, BLOCK_SIZE);
    }
    if (memcmp(secret, secret_calculated, BLOCK_SIZE) != 0) {
      LOG_ERROR("xxx0");
      return 0;
    }
  }
  return 1;
}

int generate_shares_5C2(const uint8_t recovered_shamir_data_ver[5][32],
                        const uint8_t x_coords[5],
                        uint8_t secret[32]) {
  return verify_shares_NC2(5, 32, recovered_shamir_data_ver, x_coords, secret);
}

int generate_data_5C2(const uint8_t length,
                      const uint8_t recovered_shamir_data_ver[5][length],
                      const uint8_t x_coords[5],
                      uint8_t secret[]) {
  return verify_shares_NC2(
      5, length, recovered_shamir_data_ver, x_coords, secret);
}
