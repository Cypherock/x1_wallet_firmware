/**
 * @file    utils.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
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
#define BYTE_ARRAY_TO_UINT32(x)                                                \
  ((x)[0] << 24 | (x)[1] << 16 | (x)[2] << 8 | (x)[3])
/// Read 16-bit value from big-endian serialized byte-array
#define U16_READ_BE_ARRAY(x) ((uint16_t)(x)[0] << 8 | (x)[1])
/// Read 16-bit value from little-endian serialized byte-array
#define U16_READ_LE_ARRAY(x) ((uint16_t)(x)[1] << 8 | (x)[0])
/// Read 32-bit value from big-endian serialized byte-array
#define U32_READ_BE_ARRAY(x)                                                   \
  ((uint32_t)U16_READ_BE_ARRAY(x) << 16 | U16_READ_BE_ARRAY(x + 2))
/// Read 32-bit value from little-endian serialized byte-array
#define U32_READ_LE_ARRAY(x)                                                   \
  ((uint32_t)U16_READ_LE_ARRAY(x + 2) << 16 | U16_READ_LE_ARRAY(x))
/// Read 64-bit value from big-endian serialized byte-array
#define U64_READ_BE_ARRAY(x)                                                   \
  (((uint64_t)U32_READ_BE_ARRAY(x) << 32) | U32_READ_BE_ARRAY(x + 4))
/// Read 64-bit value from little-endian serialized byte-array
#define U64_READ_LE_ARRAY(x)                                                   \
  (((uint64_t)U32_READ_LE_ARRAY(x + 4) << 32) | U32_READ_LE_ARRAY(x))
/// Change little-endian value to big-endian ordering and vice-versa
#define U16_SWAP_ENDIANNESS(x) ((x) >> 8 | (x) << 8)
/// Change little-endian value to big-endian ordering and vice-versa
#define U32_SWAP_ENDIANNESS(x)                                                 \
  ((x) << 24 | ((x)&0xff00) << 8 | ((x)&0xff0000) >> 8 | (x) >> 24)
/// Find maximum of two values
#define CY_MAX(a, b) ((a) > (b) ? (a) : (b))
/// Find minimum of two values
#define CY_MIN(a, b) ((a) < (b) ? (a) : (b))

#define UTIL_INVALID_ARGUMENTS (0x11)
#define UTIL_OUT_OF_BOUNDS (0x22)
#define UTIL_IN_BOUNDS (0xAA)

#define IS_Ith_BIT_SET(x, i) (((x) & (1 << (i))) != 0)
#define SET_Ith_BIT(x, i) ((x) |= (1 << (i)))
#define RESET_Ith_BIT(x, i) ((x) &= ~(1 << (i)))

/**
 * @brief Generic return codes for functions
 */
typedef enum FUNC_RETURN_CODES {
  /**
   * Signifies everything went as expected.
   */
  FRC_SUCCESS = 0x0,
  /**
   * Generic error occurred should ideally be never be used.
   */
  FRC_ERROR,
  /**
   * Signifies passed arguments was not expected by the function.
   */
  FRC_INVALID_ARGUMENTS,
  /**
   * Signifies a NULL pointer was found at runtime.
   */
  FRC_NULL_POINTER,
  /**
   * Signifies some variable size was exceeded during
   * function execution at runtime.
   */
  FRC_SIZE_EXCEEDED,
  /**
   * Signifies an unexpected value was encountered
   * during the execution of the function at runtime.
   */
  FRC_UNEXPECTED_VALUE,
} FUNC_RETURN_CODES;

/**
 * @brief Allocates memory via malloc and makes an entry for each allocation
 * into its global list of memory allocation.
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
uint32_t byte_array_to_hex_string(const uint8_t *bytes,
                                  uint32_t len,
                                  char *hex_char,
                                  size_t out_len);

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
void __single_to_multi_line(
    const char *input,
    uint16_t input_len,
    char output[MAX_NUMBER_OF_MNEMONIC_WORDS][MAX_MNEMONIC_WORD_LENGTH]);

/**
 * @brief  convert multi-d mnemonics to single-d array for trezor crypto
 * functions
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
void __multi_to_single_line(
    const char input[MAX_NUMBER_OF_MNEMONIC_WORDS][MAX_MNEMONIC_WORD_LENGTH],
    uint8_t number_of_mnemonics,
    char *output);

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
void hex_string_to_byte_array(const char *hex_string,
                              uint32_t string_length,
                              uint8_t *byte_array);

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
void get_firmaware_version(uint16_t pid,
                           const char *product_hash,
                           char message[]);

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
 * decimal precision depth required to decide the exact decimal precision needed
 * to show the precise value to the user.
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
void convertbase16tobase10(const uint8_t size_inp,
                           const char *u_Inp,
                           uint8_t *Out,
                           const uint8_t size_out);

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
 * 2. If size is greater than 8, then value will overflow. Returned value is
 * wrong
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
bool convert_byte_array_to_decimal_string(
    const uint8_t len,
    const uint8_t decimal,
    char *amount_string,
    char *amount_decimal_string,
    const size_t amount_decimal_string_size);

/**
 * @brief Checks if reading n bytes from a memory chunk of m bytes is safe or
 * not
 *
 * @param pBaseAddr: Base address of the memory chunk
 * @param totalSizeOfChunk: Total size of memory chunk in bytes
 * @param pCurrentSrcAddr: Base address of the chunk to read from
 * @param readSize: Size of data to be read in bytes starting from
 * pCurrentSrcAddr
 * @return uint8_t: Depicting return value of the operation
 * UTIL_INVALID_ARGUMENTS: If any argument is invalid
 * UTIL_OUT_OF_BOUNDS: If the memory access is out of bounds
 * UTIL_IN_BOUNDS: If the memory access is within bounds
 */
uint8_t UTIL_CheckBound(const uint8_t *pBaseAddr,
                        const uint32_t totalSizeOfChunk,
                        const uint8_t *pCurrentSrcAddr,
                        const uint32_t readSize);

/**
 * @brief The function takes utf-8 string as input and returns a escaped utf-8
 * string. Escaping the characters that are not supported by the used lvgl-font.
 * @details The function references lvgl library for the font in use and checks
 * if the utf-8 bytes have any valid glyphs for redering. All such utf-8 encoded
 * characters whose glyph is not available will be escaped. The escaping happens
 * on the utf-8 encoded bytes. Any invalid utf-8 encoding will be escaped with
 * `\x00`. The function ensures that result buffer is always NULL terminated.
 *
 * @param input           The input utf-8 encoded null-terminated string to be
 * escaped. Should not be NULL.
 * @param escaped_string  The output buffer which will hold the escaped string.
 * Should not be NULL.
 * @param out_len         The size of the output buffer that will hold the.
 * Should not be 0. result
 *
 * @return uint8_t Indicating if the string was processed without errors.
 * @retval 0 Indicating the input text was processed w/o any change
 * @retval 1 Indicating wrong arguments
 * @retval 2 Indicating presence of at least 1 non-printable character
 * @retval 3 Indicating presence of at least 1 invalid utf-8 encoding
 * @retval 4 Indicating at least 1 instance of both non-printable character &
 * invalid utf-8 encoding.
 *
 * @note Input & output buffers should not overlap.
 */
uint8_t string_to_escaped_string(const char *input,
                                 char *escaped_string,
                                 size_t out_len);
#endif
