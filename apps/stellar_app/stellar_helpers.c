/**
 * @file    stellar_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Stellar chains
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

#include "stellar_helpers.h"

#include "base32.h"
#include "stellar_context.h"

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

/**
 * @brief Calculate CRC16 checksum for Stellar address encoding
 * @param data Input data buffer
 * @param len Length of input data
 * @return CRC16 checksum
 */
static uint16_t crc16(const uint8_t *data, size_t len);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static uint16_t crc16(const uint8_t *data, size_t len) {
  uint16_t crc = 0x0000;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i] << 8;
    for (int j = 0; j < 8; j++) {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }
  return crc;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool stellar_derivation_path_guard(const uint32_t *path, uint8_t levels) {
  bool status = false;
  if (levels != STELLAR_IMPLICIT_ACCOUNT_DEPTH) {
    return status;
  }

  uint32_t purpose = path[0], coin = path[1], account = path[2];

  // m/44'/148'/n' - support any hardened account index
  status = (STELLAR_PURPOSE_INDEX == purpose && STELLAR_COIN_INDEX == coin &&
            is_hardened(account));

  return status;
}

bool stellar_generate_address(const uint8_t *public_key, char *address) {
  if (!public_key || !address) {
    return false;
  }

  // Stellar address encoding (StrKey format)
  // See
  // https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0023.md
  uint8_t payload[35] = {0};
  payload[0] = 0x30;    // Account ID version byte (6 << 3 | 0 = STRKEY_PUBKEY
                        // OR STRKEY_ALG_ED25519)
  memcpy(payload + 1, public_key, STELLAR_PUBKEY_RAW_SIZE);

  // CRC16-XModem checksum calculation
  // See
  // https://stellar.stackexchange.com/questions/255/which-cryptographic-algorithm-is-used-to-generate-the-secret-and-public-keys
  uint16_t checksum = crc16(payload, 33);
  payload[33] = checksum & 0xFF;
  payload[34] = checksum >> 8;

  // RFC4648 base32 encoding without padding
  base32_encode(
      payload, 35, address, STELLAR_ADDRESS_LENGTH, BASE32_ALPHABET_RFC4648);
  return true;
}