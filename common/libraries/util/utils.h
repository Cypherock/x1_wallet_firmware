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
#define BYTE_ARRAY_TO_UINT32(x) (x[0] << 24 | x[1] << 16 | x[2] << 8 | x[3])
/// Read 32-bit value from big-endian serialised byte-array
#define U32_READ_BE_ARRAY(x) ((x)[0] << 24 | (x)[1] << 16 | (x)[2] << 8 | (x)[3])
/// Read 32-bit value from little-endian serialised byte-array
#define U32_READ_LE_ARRAY(x) ((x)[3] << 24 | (x)[3] << 16 | (x)[1] << 8 | (x)[0])
/// Read 16-bit value from big-endian serialised byte-array
#define U16_READ_BE_ARRAY(x) ((x)[0] << 8 | (x)[1])
/// Change little-endian value to big-endian ordering and vice-versa
#define U16_SWAP_ENDIANNESS(x) ((x) >> 8 | (x) << 8)
/// Change little-endian value to big-endian ordering and vice-versa
#define U32_SWAP_ENDIANNESS(x) ((x) << 24 | ((x) & 0xff00) << 8 | ((x) & 0xff0000) >> 8 | (x) >> 24)
/// Find maximum of two values
#define CY_MAX(a,b) ((a) > (b) ? (a) : (b))

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
void * cy_malloc(size_t mem_size);

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
void __single_to_multi_line(const char* input, uint16_t input_len, char output[24][15]);

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
char* decode_card_number(uint8_t encoded_card_number);

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
void get_firmaware_version(uint16_t pid, const char* product_hash , char message[]);


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
void random_generate(uint8_t* arr,int len);

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
int check_digit(uint64_t value);

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

#endif
