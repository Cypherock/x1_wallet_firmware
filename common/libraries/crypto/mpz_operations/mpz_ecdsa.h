/**
 * @author  Cypherock X1 Team
 * @brief   ec operations using mpz nums
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef MPZ_ECDSA_H
#define MPZ_ECDSA_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdint.h>

#include "mini-gmp.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

// curve point x and y
typedef struct {
  mpz_t x, y;
} mpz_curve_point;

typedef struct {
  mpz_t prime;          // prime order of the finite field
  mpz_curve_point G;    // initial curve point
  mpz_t order;          // order of G
  mpz_t order_half;     // order of G divided by 2
  mpz_t a;              // coefficient 'a' of the elliptic curve
  mpz_t b;              // coefficient 'b' of the elliptic curve
} mpz_curve;

typedef struct {
  mpz_curve_point P[5];
} mpz_pedersen;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Initialize a mpz curve point.
 * @details Allocates and initializes resources for a mpz curve point.
 *
 * @param p Pointer to the mpz curve point point to initialize.
 */
void mpz_curve_point_init(mpz_curve_point *p);

/**
 * @brief Clear a mpz curve point.
 * @details Releases resources associated with the mpz curve point.
 *
 * @param p Pointer to the mpz curve point to clear.
 */
void mpz_curve_point_clear(mpz_curve_point *p);

/**
 * @brief Copy a mpz curve point.
 * @details Copies one curve point to another.
 *
 * @param cp1 Source curve point.
 * @param cp2 Destination curve point.
 */
void mpz_curve_point_copy(const mpz_curve_point *cp1, mpz_curve_point *cp2);

/**
 * @brief Add two curve points.
 * @details Computes the sum of two curve points on the given curve.
 *
 * @param curve Curve defining the group operation.
 * @param cp1 First curve point.
 * @param cp2 Second curve point; result is stored here.
 */
void mpz_curve_point_add(const mpz_curve *curve,
                         const mpz_curve_point *cp1,
                         mpz_curve_point *cp2);

/**
 * @brief Double a curve point.
 * @details Computes the point doubling operation on the given curve.
 *
 * @param curve Curve defining the group operation.
 * @param cp curve point to double; result is stored here.
 */
void mpz_curve_point_double(const mpz_curve *curve, mpz_curve_point *cp);

/**
 * @brief Multiply a curve point.
 * @details Computes scalar multiplication of a curve point on the given curve.
 *
 * @param curve Curve defining the group operation.
 * @param k Scalar multiplier.
 * @param p curve point to multiply.
 * @param res Resulting curve point is stored here.
 */
void mpz_curve_point_multiply(const mpz_curve *curve,
                              const mpz_t k,
                              const mpz_curve_point *p,
                              mpz_curve_point *res);

/**
 * @brief Set a curve point to infinity.
 * @details Configures a curve point to represent the point at infinity.
 *
 * @param p curve point to modify.
 */
void mpz_curve_point_set_infinity(mpz_curve_point *p);

/**
 * @brief Check if a curve point is at infinity.
 * @details Determines if the given curve point represents the point at
 * infinity.
 *
 * @param p curve point to check.
 * @return Non-zero if the point is at infinity, 0 otherwise.
 */
int mpz_curve_point_is_infinity(const mpz_curve_point *p);

/**
 * @brief Check if two curve points are equal.
 * @details Compares two curve points for equality.
 *
 * @param p First curve point.
 * @param q Second curve point.
 * @return Non-zero if points are equal, 0 otherwise.
 */
int mpz_curve_point_is_equal(const mpz_curve_point *p,
                             const mpz_curve_point *q);

/**
 * @brief Check if one curve point is the negative of another.
 * @details Determines if the given points are negatives of each other.
 *
 * @param p First curve point.
 * @param q Second curve point.
 * @return Non-zero if points are negatives, 0 otherwise.
 */
int mpz_curve_point_is_negative_of(const mpz_curve_point *p,
                                   const mpz_curve_point *q);

/**
 * @brief Generates ecdsa signature on mpz curve; currently configured for stark
 * curves(f251)
 */
int starknet_sign_digest(const mpz_curve *curve,
                         const uint8_t *priv_key,
                         const uint8_t *digest,
                         uint8_t *sig);

#endif    // MPZ_ECDSA_H