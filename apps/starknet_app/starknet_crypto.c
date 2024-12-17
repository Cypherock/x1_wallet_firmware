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

  stark_curve = &stark256;
}

static void stark_pedersen_init() {
  // Ref: https://docs.starkware.co/starkex/crypto/pedersen-hash-function.html

  static mpz_pedersen pedersen;
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
      "04FA56F376C83DB33F9DAB2656558F3399099EC1DE5E3018B7A6932DBA8AA378",
      16);

  // Pedersen_point_2 y
  mpz_set_str(
      pedersen.P[2].y,
      "03FA0984C931C9E38113E0C0E47E4401562761F92A7A23B45168F4E80FF5B54D",
      16);

  // Pedersen_point_3 x
  mpz_set_str(
      pedersen.P[3].x,
      "04BA4CC166BE8DEC764910F75B45F74B40C690C74709E90F3AA372F0BD2D6997",
      16);

  // Pedersen_point_3 y
  mpz_set_str(pedersen.P[3].y,
              "040301CF5C1751F4B971E46C4EDE85FCAC5C59A5CE5AE7C48151F27B24B219C",
              16);

  // Pedersen_point_4 x
  mpz_set_str(
      pedersen.P[4].x,
      "054302DCB0E6CC1C6E44CCA8F61A63BB2CA65048D53FB325D36FF12C49A58202",
      16);

  // Pedersen_point_4 y
  mpz_set_str(
      pedersen.P[4].y,
      "01B77B3E37D13504B348046268D8AE25CE98AD783C25561A879DCC77E99C2426",
      16);

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
