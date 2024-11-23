/**
 * @file    icp_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Icp chains
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2024 by HODL TECH PTE LTD
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

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "icp_helpers.h"

#include <stdint.h>

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool icp_derivation_path_guard(const uint32_t *path, uint8_t levels) {
  bool status = false;
  if (levels != ICP_IMPLICIT_ACCOUNT_DEPTH) {
    return status;
  }

  uint32_t purpose = path[0], coin = path[1], account = path[2],
           change = path[3], address = path[4];

  // m/44'/223'/0'/0/i
  status = (ICP_PURPOSE_INDEX == purpose && ICP_COIN_INDEX == coin &&
            ICP_ACCOUNT_INDEX == account && ICP_CHANGE_INDEX == change &&
            is_non_hardened(address));

  return status;
}

uint32_t update_crc32(uint32_t crc_in, uint8_t byte) {
  uint32_t crc = crc_in;
  uint32_t in =
      byte | 0x100;    // Initialize the input byte for the CRC calculation

  // Process each bit of the byte
  do {
    crc <<= 1;         // Shift the CRC register to the left
    in <<= 1;          // Shift the input byte to the left
    if (in & 0x100)    // Check if the 9th bit of the input byte is 1
      crc |= 1;        // If so, set the least significant bit of CRC to 1

    // If the CRC exceeds 32 bits, perform a modulo operation with the
    // polynomial 0x04C11DB7
    if (crc & 0x100000000) {
      crc ^= 0x04C11DB7;    // The CRC-32 polynomial is 0x04C11DB7
    }
  } while (
      in &
      0x10000);    // Continue until all bits in the input byte are processed

  return crc;
}

uint32_t get_crc32(const uint8_t *input, size_t input_size) {
  uint32_t crc = 0;
  for (size_t index = 0; index < input_size; index++) {
    crc = update_crc32(crc, input[index]);
  }
  return crc;
}