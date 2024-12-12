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

#include "mini-gmp.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
#define STARK_BN_LEN 64

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

// curve point x and y
typedef struct {
  mpz_t x, y;
} stark_point;

typedef struct {
  mpz_t prime;         // prime order of the finite field
  stark_point G;       // initial curve point
  mpz_t order;         // order of G
  mpz_t order_half;    // order of G divided by 2
  mpz_t a;             // coefficient 'a' of the elliptic curve
  mpz_t b;             // coefficient 'b' of the elliptic curve
} stark_curve;

typedef struct {
  stark_point P[5];
} stark_pedersen;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/
extern stark_curve *starkCurve;
extern stark_pedersen *starkPts;

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Initialize a STARK point.
 * @details Allocates and initializes resources for a STARK point.
 *
 * @param p Pointer to the STARK point to initialize.
 */
void stark_point_init(stark_point *p);

/**
 * @brief Clear a STARK point.
 * @details Releases resources associated with the STARK point.
 *
 * @param p Pointer to the STARK point to clear.
 */
void stark_point_clear(stark_point *p);

/**
 * @brief Copy a STARK point.
 * @details Copies one STARK point to another.
 *
 * @param cp1 Source STARK point.
 * @param cp2 Destination STARK point.
 */
void stark_point_copy(const stark_point *cp1, stark_point *cp2);

/**
 * @brief Add two STARK points.
 * @details Computes the sum of two STARK points on the given curve.
 *
 * @param curve Curve defining the group operation.
 * @param cp1 First STARK point.
 * @param cp2 Second STARK point; result is stored here.
 */
void stark_point_add(const stark_curve *curve,
                     const stark_point *cp1,
                     stark_point *cp2);

/**
 * @brief Double a STARK point.
 * @details Computes the point doubling operation on the given curve.
 *
 * @param curve Curve defining the group operation.
 * @param cp STARK point to double; result is stored here.
 */
void stark_point_double(const stark_curve *curve, stark_point *cp);

/**
 * @brief Multiply a STARK point.
 * @details Computes scalar multiplication of a STARK point on the given curve.
 *
 * @param curve Curve defining the group operation.
 * @param k Scalar multiplier.
 * @param p STARK point to multiply.
 * @param res Resulting STARK point is stored here.
 */
void stark_point_multiply(const stark_curve *curve,
                          const mpz_t k,
                          const stark_point *p,
                          stark_point *res);

/**
 * @brief Set a STARK point to infinity.
 * @details Configures a STARK point to represent the point at infinity.
 *
 * @param p STARK point to modify.
 */
void stark_point_set_infinity(stark_point *p);

/**
 * @brief Check if a STARK point is at infinity.
 * @details Determines if the given STARK point represents the point at
 * infinity.
 *
 * @param p STARK point to check.
 * @return Non-zero if the point is at infinity, 0 otherwise.
 */
int stark_point_is_infinity(const stark_point *p);

/**
 * @brief Check if two STARK points are equal.
 * @details Compares two STARK points for equality.
 *
 * @param p First STARK point.
 * @param q Second STARK point.
 * @return Non-zero if points are equal, 0 otherwise.
 */
int stark_point_is_equal(const stark_point *p, const stark_point *q);

/**
 * @brief Check if one STARK point is the negative of another.
 * @details Determines if the given points are negatives of each other.
 *
 * @param p First STARK point.
 * @param q Second STARK point.
 * @return Non-zero if points are negatives, 0 otherwise.
 */
int stark_point_is_negative_of(const stark_point *p, const stark_point *q);

/**
 * @brief Clear Pedersen resources.
 * @details Releases all resources related to Pedersen hash operations.
 */
void stark_pedersen_clear();

/**
 * @brief Initialize STARKNET context.
 * @details Sets up resources and state for STARKNET operations.
 */
void starknet_init();

#endif    // STARKNET_CRYPTO_H