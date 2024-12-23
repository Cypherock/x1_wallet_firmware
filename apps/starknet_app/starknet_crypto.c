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

#include <starkcurve.h>
#include <stdio.h>

#include "mini-gmp.h"
#include "mpz_ecdsa.h"

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
/**
 * @brief Initializes @ref stark_curve instance based on star curve params
 ref:
  https://github.com/xJonathanLEI/starknet-rs/blob/f31e426a65225b9830bbf3c148f7ea05bf9dc257/starknet-curve/src/curve_params.rs

 */
static void stark_curve_init();

/**
 * @brief Initializes starknet pedersen points
 */
static void stark_pedersen_init();
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/
mpz_curve *stark_curve;
mpz_pedersen *starknet_pedersen_points;

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void stark_curve_init() {
  static mpz_curve stark256;

  // Initialize mpz_t variables in stark256
  mpz_init(stark256.prime);
  mpz_init(stark256.G.x);
  mpz_init(stark256.G.y);
  mpz_init(stark256.order);
  mpz_init(stark256.order_half);
  mpz_init(stark256.a);
  mpz_init(stark256.b);

  mpz_set_str(stark256.prime, STARKNET_CURVE_PRIME, SIZE_HEX);
  mpz_set_str(stark256.G.x, STARKNET_CURVE_GX, SIZE_HEX);
  mpz_set_str(stark256.G.y, STARKNET_CURVE_GY, SIZE_HEX);
  mpz_set_str(stark256.order, STARKNET_CURVE_ORDER, SIZE_HEX);
  mpz_set_str(stark256.order_half, STARKNET_CURVE_ORDER_HALF, SIZE_HEX);
  mpz_set_str(stark256.a, STARKNET_CURVE_A, SIZE_HEX);
  mpz_set_str(stark256.b, STARKNET_CURVE_B, SIZE_HEX);

  stark_curve = &stark256;
}

static void stark_pedersen_init() {
  static mpz_pedersen pedersen;
  // Initialize all mpz_t variables in the pedersen structure
  for (int i = 0; i < 5; i++) {
    mpz_init(pedersen.P[i].x);
    mpz_init(pedersen.P[i].y);
  }

  mpz_set_str(pedersen.P[0].x, STARKNET_PEDERSEN_POINT_0_X, SIZE_HEX);
  mpz_set_str(pedersen.P[0].y, STARKNET_PEDERSEN_POINT_0_Y, SIZE_HEX);
  mpz_set_str(pedersen.P[1].x, STARKNET_PEDERSEN_POINT_1_X, SIZE_HEX);
  mpz_set_str(pedersen.P[1].y, STARKNET_PEDERSEN_POINT_1_Y, SIZE_HEX);
  mpz_set_str(pedersen.P[2].x, STARKNET_PEDERSEN_POINT_2_X, SIZE_HEX);
  mpz_set_str(pedersen.P[2].y, STARKNET_PEDERSEN_POINT_2_Y, SIZE_HEX);
  mpz_set_str(pedersen.P[3].x, STARKNET_PEDERSEN_POINT_3_X, SIZE_HEX);
  mpz_set_str(pedersen.P[3].y, STARKNET_PEDERSEN_POINT_3_Y, SIZE_HEX);
  mpz_set_str(pedersen.P[4].x, STARKNET_PEDERSEN_POINT_4_X, SIZE_HEX);
  mpz_set_str(pedersen.P[4].y, STARKNET_PEDERSEN_POINT_4_Y, SIZE_HEX);

  starknet_pedersen_points = &pedersen;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void starknet_init() {
  stark_curve_init();
  stark_pedersen_init();
}

void stark_clear() {
  // clear pedersen points
  for (int i = 0; i < 5; i++) {
    mpz_curve_point_clear(&starknet_pedersen_points->P[i]);
  }
  // clear stark curve points
  mpz_clear(stark_curve->a);
  mpz_clear(stark_curve->b);
  mpz_curve_point_clear(&stark_curve->G);
  mpz_clear(stark_curve->order);
  mpz_clear(stark_curve->order_half);
  mpz_clear(stark_curve->prime);
}
