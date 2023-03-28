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
static uint8_t galois_exp[] = {
    1,   3,   5,   15,  17,  51,  85,  255, 26,  46,  114, 150, 161, 248, 19,
    53,  95,  225, 56,  72,  216, 115, 149, 164, 247, 2,   6,   10,  30,  34,
    102, 170, 229, 52,  92,  228, 55,  89,  235, 38,  106, 190, 217, 112, 144,
    171, 230, 49,  83,  245, 4,   12,  20,  60,  68,  204, 79,  209, 104, 184,
    211, 110, 178, 205, 76,  212, 103, 169, 224, 59,  77,  215, 98,  166, 241,
    8,   24,  40,  120, 136, 131, 158, 185, 208, 107, 189, 220, 127, 129, 152,
    179, 206, 73,  219, 118, 154, 181, 196, 87,  249, 16,  48,  80,  240, 11,
    29,  39,  105, 187, 214, 97,  163, 254, 25,  43,  125, 135, 146, 173, 236,
    47,  113, 147, 174, 233, 32,  96,  160, 251, 22,  58,  78,  210, 109, 183,
    194, 93,  231, 50,  86,  250, 21,  63,  65,  195, 94,  226, 61,  71,  201,
    64,  192, 91,  237, 44,  116, 156, 191, 218, 117, 159, 186, 213, 100, 172,
    239, 42,  126, 130, 157, 188, 223, 122, 142, 137, 128, 155, 182, 193, 88,
    232, 35,  101, 175, 234, 37,  111, 177, 200, 67,  197, 84,  252, 31,  33,
    99,  165, 244, 7,   9,   27,  45,  119, 153, 176, 203, 70,  202, 69,  207,
    74,  222, 121, 139, 134, 145, 168, 227, 62,  66,  198, 81,  243, 14,  18,
    54,  90,  238, 41,  123, 141, 140, 143, 138, 133, 148, 167, 242, 13,  23,
    57,  75,  221, 124, 132, 151, 162, 253, 28,  36,  108, 180, 199, 82,  246,
    1};

static uint8_t galois_log[] = {
    0,   0,   25,  1,   50,  2,   26,  198, 75,  199, 27,  104, 51,  238, 223,
    3,   100, 4,   224, 14,  52,  141, 129, 239, 76,  113, 8,   200, 248, 105,
    28,  193, 125, 194, 29,  181, 249, 185, 39,  106, 77,  228, 166, 114, 154,
    201, 9,   120, 101, 47,  138, 5,   33,  15,  225, 36,  18,  240, 130, 69,
    53,  147, 218, 142, 150, 143, 219, 189, 54,  208, 206, 148, 19,  92,  210,
    241, 64,  70,  131, 56,  102, 221, 253, 48,  191, 6,   139, 98,  179, 37,
    226, 152, 34,  136, 145, 16,  126, 110, 72,  195, 163, 182, 30,  66,  58,
    107, 40,  84,  250, 133, 61,  186, 43,  121, 10,  21,  155, 159, 94,  202,
    78,  212, 172, 229, 243, 115, 167, 87,  175, 88,  168, 80,  244, 234, 214,
    116, 79,  174, 233, 213, 231, 230, 173, 232, 44,  215, 117, 122, 235, 22,
    11,  245, 89,  203, 95,  176, 156, 169, 81,  160, 127, 12,  246, 111, 23,
    196, 73,  236, 216, 67,  31,  45,  164, 118, 123, 183, 204, 187, 62,  90,
    251, 96,  177, 134, 59,  82,  161, 108, 170, 85,  41,  157, 151, 178, 135,
    144, 97,  190, 220, 252, 188, 149, 207, 205, 55,  63,  91,  209, 83,  57,
    132, 60,  65,  162, 109, 71,  20,  42,  158, 93,  86,  242, 211, 171, 68,
    17,  146, 217, 35,  32,  46,  137, 180, 124, 184, 38,  119, 153, 227, 165,
    103, 74,  237, 222, 197, 49,  254, 24,  13,  99,  140, 128, 192, 247, 112,
    7};

static uint8_t galois_add(const uint8_t a, const uint8_t b) {
  return a ^ b;
}

static uint8_t galois_diff(const uint8_t a, const uint8_t b) {
  return a ^ b;
}

static uint8_t galois_mul(const uint8_t a, const uint8_t b) {
  if (a == 0 || b == 0)
    return 0;
  uint8_t ans_log = ((uint16_t)galois_log[a] + (uint16_t)galois_log[b]) % 255;
  return galois_exp[ans_log];
}

static uint8_t galois_div(const uint8_t a, const uint8_t b) {
  //	assert(b != 0);
  uint8_t ans_log =
      ((uint16_t)galois_log[a] + (uint16_t)(255 - galois_log[b])) % 255;
  return galois_exp[ans_log];
}

static uint8_t galois_pow(const uint8_t a, const uint16_t p) {
  uint8_t ans_log = ((uint32_t)galois_log[a] * (uint32_t)p) % 255;
  return galois_exp[ans_log];
}

/***********************************************/

int GetOneRandomByte() {
  uint8_t random_byte = 0;
  random_generate(&random_byte, 1);
  return random_byte;
}

static void FillRandomVectorInARange(const uint16_t arr_size,
                                     uint8_t arr[arr_size],
                                     uint8_t l,
                                     uint8_t h) {
  for (uint16_t i = 0; i < arr_size; i++) {
    arr[i] = (GetOneRandomByte() % (h - l + 1)) + l;
  }
}

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
    FillRandomVectorInARange(threshold_number_of_shares - 1, coeffs, 1, 255);
    for (uint8_t i = 0; i < total_number_of_shares; i++) {
      shares_OUT[i][j] = galois_add(
          secret[j],
          eval(threshold_number_of_shares - 1,
               coeffs,
               i + 1));    // galois_add(galois_mul(m, i+1), secret[j]);
    }
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
