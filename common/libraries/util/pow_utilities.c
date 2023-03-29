/**
 * @file    pow_utilities.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
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
#include "pow_utilities.h"

#include "constant_texts.h"

static const uint8_t EXPONENT_TABLE[] =
    {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};

uint16_t pow_count_set_bits(const uint8_t target[SHA256_SIZE]) {
  uint16_t number_bits_set = 0U;
  uint8_t index = SHA256_SIZE - 1U;

  for (; index >= 0U; index--) {
    if (target[index] == 0xffU)
      number_bits_set++;
    else
      break;
  }

  number_bits_set = number_bits_set * 8U;    // number of bits set in target

  if (index >= 0U) {
    uint16_t i = 0U;
    for (; i < 7U; i++) {
      if (EXPONENT_TABLE[i] < target[index] &&
          target[index] <= EXPONENT_TABLE[i + 1U]) {
        number_bits_set += i + 1U;
        break;
      }
    }
  }
  return number_bits_set;
}

void pow_get_approx_time_in_secs(const uint8_t target[SHA256_SIZE],
                                 uint32_t *time_in_secs_out) {
  // If target = 2^n then get n

  uint16_t number_bits_set = pow_count_set_bits(target);

  if ((256U - number_bits_set) > 63U) {
    // This is too long of a time to calculate, just return
    *time_in_secs_out = UINT32_MAX;
    return;
  }

  uint64_t time_in_secs = 1U;
  ASSERT(pow_hash_rate != 0);
  time_in_secs = (time_in_secs << (256U - number_bits_set)) / pow_hash_rate;

  *time_in_secs_out = CY_MIN(time_in_secs, UINT32_MAX);
}

void convert_secs_to_time(const uint32_t time_in_secs,
                          const char wallet_name[NAME_SIZE],
                          char out_string[MAX_NUM_OF_CHARS_IN_A_SLIDE]) {
  /*print 2x expected time. TBD: to be replaced with data points based
   * relation*/
  if (time_in_secs <= 60) {
    snprintf(out_string,
             MAX_NUM_OF_CHARS_IN_A_SLIDE,
             "%s is in process to unlock...",
             wallet_name);
  } else if (time_in_secs <= 300) {
    snprintf(out_string,
             MAX_NUM_OF_CHARS_IN_A_SLIDE,
             UI_TEXT_WALLET_LOCKED_WAIT_MSG,
             wallet_name,
             10,
             "Minutes");
  } else if (time_in_secs <= 3600) {
    snprintf(out_string,
             MAX_NUM_OF_CHARS_IN_A_SLIDE,
             UI_TEXT_WALLET_LOCKED_WAIT_MSG,
             wallet_name,
             2,
             "Hours");
  } else if (time_in_secs <= 18000) {
    snprintf(out_string,
             MAX_NUM_OF_CHARS_IN_A_SLIDE,
             UI_TEXT_WALLET_LOCKED_WAIT_MSG,
             wallet_name,
             10,
             "Hours");
  } else if (time_in_secs <= 86400) {
    snprintf(out_string,
             MAX_NUM_OF_CHARS_IN_A_SLIDE,
             UI_TEXT_WALLET_LOCKED_WAIT_MSG,
             wallet_name,
             2,
             "Days");
  } else if (time_in_secs <= 432000) {
    snprintf(out_string,
             MAX_NUM_OF_CHARS_IN_A_SLIDE,
             UI_TEXT_WALLET_LOCKED_WAIT_MSG,
             wallet_name,
             10,
             "Days");
  } else if (time_in_secs <= 2628000) {
    snprintf(out_string,
             MAX_NUM_OF_CHARS_IN_A_SLIDE,
             UI_TEXT_WALLET_LOCKED_WAIT_MSG,
             wallet_name,
             2,
             "Months");
  } else {
    snprintf(out_string,
             MAX_NUM_OF_CHARS_IN_A_SLIDE,
             "%s is currently locked\nWait for more than a year",
             wallet_name);
  }
}
