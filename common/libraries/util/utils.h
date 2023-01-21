/**
 * @file    utils.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */
/**
 * @file utils.h
 * @author your name (you@domain.com)
 * @brief Common util functions
 * @version 0.1
 * @date 2020-05-28
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef UTILS_H
#define UTILS_H

#pragma once
#include <inttypes.h>
#include "wallet.h"

/// Convert bit array of size 4 to uint32
#define BYTE_ARRAY_TO_UINT32(x) ((x)[0] << 24 | (x)[1] << 16 | (x)[2] << 8 | (x)[3])
/// Read 16-bit value from big-endian serialized byte-array
#define U16_READ_BE_ARRAY(x) ((x)[0] << 8 | (x)[1])
/// Read 16-bit value from little-endian serialized byte-array
#define U16_READ_LE_ARRAY(x) ((x)[1] << 8 | (x)[0])
/// Read 32-bit value from big-endian serialized byte-array
#define U32_READ_BE_ARRAY(x) (U16_READ_BE_ARRAY(x) << 16 | U16_READ_BE_ARRAY(x + 2))
/// Read 32-bit value from little-endian serialized byte-array
#define U32_READ_LE_ARRAY(x) (U16_READ_LE_ARRAY(x + 2) << 16 | U16_READ_LE_ARRAY(x))
/// Read 64-bit value from big-endian serialized byte-array
#define U64_READ_BE_ARRAY(x) (((uint64_t)U32_READ_BE_ARRAY(x) << 32) | U32_READ_BE_ARRAY(x + 4))
/// Read 64-bit value from little-endian serialized byte-array
#define U64_READ_LE_ARRAY(x) (((uint64_t)U32_READ_LE_ARRAY(x + 4) << 32) | U32_READ_LE_ARRAY(x))
/// Change little-endian value to big-endian ordering and vice-versa
#define U16_SWAP_ENDIANNESS(x) ((x) >> 8 | (x) << 8)
/// Change little-endian value to big-endian ordering and vice-versa
#define U32_SWAP_ENDIANNESS(x) ((x) << 24 | ((x)&0xff00) << 8 | ((x)&0xff0000) >> 8 | (x) >> 24)
/// Find maximum of two values
#define CY_MAX(a, b) ((a) > (b) ? (a) : (b))
/// Find minimum of two values
#define CY_MIN(a, b) ((a) < (b) ? (a) : (b))

#define UTIL_INVALID_ARGUMENTS      (0x11)
#define UTIL_OUT_OF_BOUNDS          (0x22)
#define UTIL_IN_BOUNDS              (0xAA)

/**
 * @brief Allocates memory via malloc and makes an entry for each allocation into its global list
 * of memory allocation.
 * @details
 * 
 * @param [in, out]  mem
 * @param [in]       mem_size
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void *cy_malloc(size_t mem_size);

/**
 * @brief Free entire list of dynamic allocations.
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void cy_free();

/**
 * @brief Checks if int value represented by byte array is zero.
 * @details
 *
 * @param [in] bytes Byte array.
 * @param [in] len Length of yte array.
 *
 * @return 0, 1
 * @retval 1 If the byte array is all zeroes or length of array is 0
 * @retval 0 Otherwise
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int is_zero(const uint8_t *bytes, uint8_t len);

/**
 * @brief Convert byte array to hex char array.
 * @details
 * 
 * @param [in] bytes Pointer to byte array.
 * @param [in] len Length of byte array.
 * @param [out] hex_char char array to store results of the conversion.
 * @param [out] out_len Declared length of out char array
 *
 * @return Actual length of bytes.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint32_t byte_array_to_hex_string(const uint8_t *bytes, uint32_t len, char *hex_char, size_t out_len);

/**
 * @brief  convert single-d mnemonics to multi-d array for listing ui
 * @details
 *
 * @param [in] input
 * @param [in] input_len
 * @param [out] output
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void __single_to_multi_line(const char *input, uint16_t input_len, char output[24][15]);

/**
 * @brief  convert multi-d mnemonics to single-d array for trezor crypto functions
 * @details
 *
 * @param [in] input
 * @param [in] number_of_mnemonics
 * @param [out] output
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void __multi_to_single_line(const char input[24][15], uint8_t number_of_mnemonics, char *output);

/**
 * @brief Converts a hex string to a byte array.
 * @details
 *
 * @param hex_string char string in with hex characters.
 * @param string_length Length of the string.
 * @param byte_array Byte array address to store the converted hex array.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void hex_string_to_byte_array(const char *hex_string, uint32_t string_length, uint8_t *byte_array);

/**
 * @brief printf hex array
 * @details
 *
 * @param [in] text
 * @param [in] arr
 * @param [in] length
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void print_hex_array(const char text[], const uint8_t *arr, uint8_t length);

/**
 * @brief Converts bit representation of card number into
 * integer representation.
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 * @example (0100)b is converted into 3
 */
uint8_t decode_card_number(uint8_t encoded_card_number);

/**
 * @brief Converts integer representation of card number into
 * bit representation.
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 * @example 3 is converted into (0100)b
 */
uint8_t encode_card_number(uint8_t decoded_card_number);

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void get_firmaware_version(uint16_t pid, const char *product_hash, char message[]);

/**
 * @brief Genrate random 32 byte using BSP and atecc random generator function's
 * @details
 *
 * @param arr
 * @param len
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void random_generate(uint8_t *arr, int len);

/**
 * @brief Calculates the number of precision places after decimal point.
 * @details The function takes the numerator and denominator for calculating the
 * decimal precision depth required to decide the exact decimal precision needed to
 * show the precise value to the user.
 *
 * @param num    Numerator value for the decimal precision calculation
 * @param den    Denominator for the decimal place calculation
 *
 * @return uint8_t
 * @retval precision The number of decimal precision depth required
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint8_t get_floating_precision(uint64_t num, uint64_t den);

/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void der_to_sig(const uint8_t *der, uint8_t *sig);

/**
 * @brief conversion base16 to base10
 * @details
 *
 * @param size_inp size of input hex string
 * @param u_Inp input hex string
 * @param Out output reference.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void convertbase16tobase10(const uint8_t size_inp, const char *u_Inp, uint8_t *Out, const uint8_t size_out);

/**
 * @brief Convert decimal to byte array
 * @details
 *
 * @param dec Decimal uint64_t to convert
 * @param hex Pointer to byte array
 * @param len Length of the byte array
 *
 * @return Number of bytes after conversion
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
uint8_t dec_to_hex(const uint64_t dec, uint8_t *hex, uint8_t len);

/**
 * @brief Reverse byte array
 */
uint8_t cy_reverse_byte_array(uint8_t *byte_data, uint16_t len);

/**
 * Reads bytes and returns big-endian unsigned integer.
 *
 * NOTE:
 * 1. If bytes is NULL or size is 0, then '0' will be returned.
 * 2. If size is greater than 8, then value will overflow. Returned value is wrong
 *
 * @param bytes     The byte array to read
 * @param size      The number of bytes to read
 * @return          uint64_t big endian value
 */
uint64_t cy_read_be(const uint8_t *bytes, uint8_t size);

/**
 * @brief Convert byte array to decimal string with given decimal place
 * 
 * @param len 
 * @param amount_string 
 * @param amount_decimal_string 
 * @param amount_decimal_string_size 
 * @return true if success
 * @return false if fails
 */
bool convert_byte_array_to_decimal_string(const uint8_t len,
                                          const uint8_t decimal,
                                          char *amount_string,
                                          char *amount_decimal_string,
                                          const size_t amount_decimal_string_size);

/**
 * @brief Checks if reading n bytes from a memory chunk of m bytes is safe or not
 * 
 * @param pBaseAddr: Base address of the memory chunk
 * @param totalSizeOfChunk: Total size of memory chunk in bytes
 * @param pCurrentSrcAddr: Base address of the chunk to read from
 * @param readSize: Size of data to be read in bytes starting from pCurrentSrcAddr
 * @return uint8_t: Depicting return value of the operation
 * UTIL_INVALID_ARGUMENTS: If any argument is invalid
 * UTIL_OUT_OF_BOUNDS: If the memory access is out of bounds
 * UTIL_IN_BOUNDS: If the memory access is within bounds
 */
uint8_t UTIL_CheckBound(
                        const uint8_t *pBaseAddr,
                        const uint32_t totalSizeOfChunk,
                        const uint8_t *pCurrentSrcAddr,
                        const uint32_t readSize
                       );
#endif
