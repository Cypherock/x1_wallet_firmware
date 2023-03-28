/**
 * @file    shamir_wrapper.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef SHAMIR_WRAPPER_H
#define SHAMIR_WRAPPER_H

#include <stdint.h>
/**
 * @brief
 * @details size of shares_OUT must be
 * shares_OUT[total_number_of_shares][secret_len] shares_OUT's ith index will
 * have shares corresponding to x = (i+1) secret is considered to be
 * corresponding to x = 0
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note memory to be allocated by client
 */
void convert_to_shares(uint8_t secret_len,
                       const uint8_t secret[secret_len],
                       uint8_t total_number_of_shares,
                       uint8_t threshold_number_of_shares,
                       uint8_t shares_OUT[total_number_of_shares][secret_len]);

/**
 * @brief
 * @details size of secret_OUT must be equal to number_of_bytes
 * secret is considered to be corresponding to x = 0
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note memory to be allocated by client
 */
void recover_secret_from_shares(
    uint8_t number_of_bytes,    // visualise this as horizontal length
    uint8_t num_shares,    // threshold. shares is a 2D array. visualise this as
                           // vertical height
    const uint8_t shares[num_shares][number_of_bytes],
    uint8_t x_coords[num_shares],
    uint8_t secret_OUT[number_of_bytes]);

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
void recover_share_from_shares(
    uint8_t number_of_bytes,    // visualise this as horizontal length
    uint8_t num_shares,    // threshold. shares is a 2D array. visualise this as
                           // vertical height
    const uint8_t shares[num_shares][number_of_bytes],
    const uint8_t x_coords[num_shares],
    uint8_t share_OUT[number_of_bytes],
    uint8_t out_x_cor);

/**
 * @brief
 * @details
 *
 * @param number_of_shares
 * @param secret_size
 * @param recovered_shamir_data_ver
 * @param x_coords
 * @param secret
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
int verify_shares_NC2(
    uint8_t number_of_shares,
    uint8_t secret_size,
    uint8_t recovered_shamir_data_ver[number_of_shares][secret_size],
    uint8_t x_coords[number_of_shares],
    uint8_t secret[secret_size]);

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
int generate_shares_4C2(uint8_t recovered_shamir_data_ver[4][32],
                        uint8_t x_coords[4],
                        uint8_t secret[32]);

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
int generate_data_4C2(uint8_t length,
                      uint8_t recovered_shamir_data_ver[4][length],
                      uint8_t x_coords[4],
                      uint8_t secret[]);

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
int generate_shares_5C2(const uint8_t recovered_shamir_data_ver[5][32],
                        const uint8_t x_coords[5],
                        uint8_t secret[32]);

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
int generate_data_5C2(uint8_t length,
                      const uint8_t recovered_shamir_data_ver[5][length],
                      const uint8_t x_coords[5],
                      uint8_t secret[]);

#endif
