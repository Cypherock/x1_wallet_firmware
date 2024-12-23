/**
 * @file    starkcurve.h
 * @author  Cypherock X1 Team
 * @brief   Constants for Starknet curve.
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

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define SIZE_HEX 16

// Starknet curve constants
#define STARKNET_CURVE_PRIME                                                   \
  "0800000000000011000000000000000000000000000000000000000000000001"
#define STARKNET_CURVE_GX                                                      \
  "01EF15C18599971B7BECED415A40F0C7DEACFD9B0D1819E03D723D8BC943CFCA"
#define STARKNET_CURVE_GY                                                      \
  "005668060AA49730B7BE4801DF46EC62DE53ECD11ABE43A32873000C36E8DC1F"
#define STARKNET_CURVE_ORDER                                                   \
  "0800000000000010ffffffffffffffffb781126dcae7b2321e66a241adc64d2f"
#define STARKNET_CURVE_ORDER_HALF                                              \
  "04000000000000087fffffffffffffffdbc08936e573d9190f335120d6e32697"
#define STARKNET_CURVE_A                                                       \
  "0000000000000000000000000000000000000000000000000000000000000001"
#define STARKNET_CURVE_B                                                       \
  "06f21413efbe40de150e596d72f7a8c5609ad26c15c915c1f4cdfcb99cee9e89"

// starknet pedersen points
// Ref: https://docs.starkware.co/starkex/crypto/pedersen-hash-function.html
#define STARKNET_PEDERSEN_POINT_0_X                                            \
  "049EE3EBA8C1600700EE1B87EB599F16716B0B1022947733551FDE4050CA6804"
#define STARKNET_PEDERSEN_POINT_0_Y                                            \
  "03CA0CFE4B3BC6DDF346D49D06EA0ED34E621062C0E056C1D0405D266E10268A"
#define STARKNET_PEDERSEN_POINT_1_X                                            \
  "0234287DCBAFFE7F969C748655FCA9E58FA8120B6D56EB0C1080D17957EBE47B"
#define STARKNET_PEDERSEN_POINT_1_Y                                            \
  "03B056F100F96FB21E889527D41F4E39940135DD7A6C94CC6ED0268EE89E5615"
#define STARKNET_PEDERSEN_POINT_2_X                                            \
  "04FA56F376C83DB33F9DAB2656558F3399099EC1DE5E3018B7A6932DBA8AA378"
#define STARKNET_PEDERSEN_POINT_2_Y                                            \
  "03FA0984C931C9E38113E0C0E47E4401562761F92A7A23B45168F4E80FF5B54D"
#define STARKNET_PEDERSEN_POINT_3_X                                            \
  "04BA4CC166BE8DEC764910F75B45F74B40C690C74709E90F3AA372F0BD2D6997"
#define STARKNET_PEDERSEN_POINT_3_Y                                            \
  "040301CF5C1751F4B971E46C4EDE85FCAC5C59A5CE5AE7C48151F27B24B219C"
#define STARKNET_PEDERSEN_POINT_4_X                                            \
  "054302DCB0E6CC1C6E44CCA8F61A63BB2CA65048D53FB325D36FF12C49A58202"
#define STARKNET_PEDERSEN_POINT_4_Y                                            \
  "01B77B3E37D13504B348046268D8AE25CE98AD783C25561A879DCC77E99C2426"

// starknet limit
#define STARKNET_LIMIT                                                         \
  "F80000000000020EFFFFFFFFFFFFFFF738A13B4B920E9411AE6DA5F40B0358B1"
