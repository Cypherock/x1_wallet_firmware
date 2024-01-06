/**
 * @file    byte_utilities.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
/**
 * @file
 * Utility functions for byte arrays.
 *
 * Assumes that the byte array is in big endian form.
 * For example 0x1234 will be stored as
 * uint8_t arr[] = {0x12, 0x34}
 */

#ifndef BYTE_UTILITIES_H
#define BYTE_UTILITIES_H

#include <stdint.h>

/**
 * @brief Does arr++
 * @details
 *
 * @param [in, out] arr     Input array to increament. Also the output array.
 * @param [in] size         Size of input array
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void increment_byte_array(uint8_t *arr, uint8_t size);

/**
 * @brief Adds two byte array of the same size (dest_arr = arr1 + arr2)
 * @details
 *
 * @param dest_arr Destination Array
 * @param arr1 Array 1
 * @param arr2 Array 2
 * @param size Size of Array 1 and Array 2
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void add_byte_array(uint8_t *dest_arr,
                    const uint8_t *arr1,
                    const uint8_t *arr2,
                    uint8_t size);

#endif