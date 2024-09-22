/**
 * @file    starknet_crypto.h
 * @author  Cypherock X1 Team
 * @brief   Crypto api definitions for Starknet chain
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef STARKNET_CRYPTO_H
#define STARKNET_CRYPTO_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "bn.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

// curve point x and y
typedef struct {
  struct bn x, y;
} stark_point;

typedef struct {
  struct bn prime;         // prime order of the finite field
  stark_point G;           // initial curve point
  struct bn order;         // order of G
  struct bn order_half;    // order of G divided by 2
  struct bn a;             // coefficient 'a' of the elliptic curve
  struct bn b;             // coefficient 'b' of the elliptic curve
} stark_curve;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

extern const stark_curve stark256;

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
void stark_point_copy(const stark_point *cp1, stark_point *cp2);
void stark_point_add(const stark_curve *curve,
                     const stark_point *cp1,
                     stark_point *cp2);
void stark_point_double(const stark_curve *curve, stark_point *cp);
void stark_point_multiply(const stark_curve *curve,
                          const struct bn *k,
                          const stark_point *p,
                          stark_point *res);
void stark_point_set_infinity(stark_point *p);
int stark_point_is_infinity(const stark_point *p);
int stark_point_is_equal(const stark_point *p, const stark_point *q);

#endif    // STARKNET_CRYPTO_H