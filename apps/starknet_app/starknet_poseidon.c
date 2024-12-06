/**
 * @file    starknet_poseidon.c
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
#include "f251.h"
#include "mini-gmp-helpers.h"
#include "poseidon.h"

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
// Function to convert Big-Endian hex to Little-Endian felt_t
void hex_to_felt_t(const uint8_t hex[32], felt_t felt) {
  int offset = 0;
  for (int i = 0; i < 4; i++) {
    felt[3 - i] = U64_READ_BE_ARRAY(hex + offset);
    offset += 8;
  }
}

// Function to convert Little-Endian felt_t to Big-Endian hex
void felt_t_to_hex(const felt_t felt, uint8_t hex[32]) {
  int offset = 0;
  for (int i = 0; i < 4; i++) {
    uint64_t value = felt[3 - i];

    // Break the uint64_t value into 8 bytes and store in hex array in
    // Big-Endian order
    for (int j = 0; j < 8; j++) {
      hex[offset + j] = (uint8_t)((value >> (56 - j * 8)) & 0xFF);
    }

    offset += 8;
  }
}

static void clear_state(felt_t *state, int size) {
  int i;

  for (i = 0; i < size; i++) {
    state[i][0] = 0;
    state[i][1] = 0;
    state[i][2] = 0;
    state[i][3] = 0;
  }
}
static void print_state(felt_t *state, int size) {
  int i;

  for (i = 0; i < size; i++) {
    printf("%016" PRIx64, state[i][3]);
    printf("%016" PRIx64, state[i][2]);
    printf("%016" PRIx64, state[i][1]);
    printf("%016" PRIx64, state[i][0]);
    printf("\n");
  }
}
void poseidon_hash_many(felt_t state[], uint8_t state_size, felt_t res) {
  ASSERT(state_size + 2 < 16);

  const uint8_t m = 3, rate = 2;
  felt_t padded[16];    ///< TODO: Update with macro
  clear_state(padded, 16);

  for (int i = 0; i < state_size; i++) {
    f251_copy(padded[i], state[i]);
  }
  uint8_t padded_offset = state_size;

  // padd one to mark end of ip
  felt_t felt_one = {1, 0, 0, 0};
  f251_copy(padded[padded_offset++], felt_one);
  // padd with zeros to make multiple of rate
  felt_t felt_zero = {0, 0, 0, 0};
  while (padded_offset % rate != 0) {
    f251_copy(padded[padded_offset++], felt_zero);
  }

  felt_t result[m];
  clear_state(result, m);

  for (int i = 0; i < padded_offset; i += rate) {
    for (int j = 0; j < rate; j++) {
      f251_add(result[j], result[j], padded[i + j]);
    }
    permutation_3(result);
  }
  // copt result
  f251_copy(res, result[0]);
}