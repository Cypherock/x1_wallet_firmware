/**
 * @file    pow_utilities.h
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
 * Contains utilities used in proof of work task
 */

#ifndef POW_UTILITIES_H
#define POW_UTILITIES_H

#include "pow.h"

uint16_t pow_count_set_bits(const uint8_t target[SHA256_SIZE]);

/**
 * @brief Given a target this function calculates the time required to
 * find a suitable nonce.
 * @details
 *
 * @param target The target provided by the card
 * @param time_in_secs_out Returns the calculated time to unlock in secs
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note This function assumes that when proof of work is started then
 * the nonce will reset to 0.
 *
 * Actual Formula : Time in secs = (2^256-1)/target * (Hash Rate)^(-1)
 *
 * For simplicity, the target is always a multiple of 2
 * If target = 2^240 then it will be represented as follow
 * uint8_t target[32] = {0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 * 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 * 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
 *
 * Example :
 * Target = 2^n
 *
 * Time in secs ~ 2^(256-n)/(Hash Rate)
 */
void pow_get_approx_time_in_secs(const uint8_t target[SHA256_SIZE],
                                 uint32_t *time_in_secs_out);

/**
 * @brief Convert secs to displayable string
 * @details
 *
 * @param [in] time_in_secs     Input time to be converted
 * @param [in] wallet_name      Wallet name to be displayed on screen
 * @param [out] out_string      The final output string that can be displayed.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note This function rounds off to nearest and largest exceptable time.
 * Exceptable time includes : 5 mins, 1 hour, 5 hours, 1 day...
 * @example 120 seconds --> 5 minutes
 */
void convert_secs_to_time(uint32_t time_in_secs,
                          const char wallet_name[NAME_SIZE],
                          char out_string[MAX_NUM_OF_CHARS_IN_A_SLIDE]);

#endif