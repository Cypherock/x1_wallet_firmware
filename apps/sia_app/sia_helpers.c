/**
 * @file    sia_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Sia chains
 * @copyright Copyright (c) 2025 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2025 by HODL TECH PTE LTD
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

#include "sia_helpers.h"

#include "sia_context.h"
#include "utils.h"

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

bool sia_derivation_path_guard(const uint32_t *path, uint8_t levels) {
  bool status = true;
  // Sia uses simple index-based derivation, just validate we have exactly 1
  // level
  if (levels != SIA_IMPLICIT_ACCOUNT_DEPTH) {
    status = false;
  }
  uint32_t index = path[0];
  // It should be a non-negative number
  if (index < 0) {
    status = false;
  }

  return status;
}

bool sia_full_address(const uint8_t *address_hash, char *full_address) {
  uint8_t checksum[32];
  if (blake2b(address_hash, 32, checksum, 32) != 0) {
    return false;
  }

  byte_array_to_hex_string(address_hash, 32, full_address, 77);
  byte_array_to_hex_string(checksum, 6, full_address + 64, 13);

  return true;
}

bool sia_generate_address(const uint8_t *public_key, char *address) {
  if (!public_key || !address) {
    return false;
  }

  // Pre-computed Sia unlock condition hashes
  uint8_t timelock_hash[32] = {0x51, 0x87, 0xb7, 0xa8, 0x02, 0x1b, 0xf4, 0xf2,
                               0xc0, 0x04, 0xea, 0x3a, 0x54, 0xcf, 0xec, 0xe1,
                               0x75, 0x4f, 0x11, 0xc7, 0x62, 0x4d, 0x23, 0x63,
                               0xc7, 0xf4, 0xcf, 0x4f, 0xdd, 0xd1, 0x44, 0x1e};

  uint8_t sigsrequired_hash[32] = {
      0xb3, 0x60, 0x10, 0xeb, 0x28, 0x5c, 0x15, 0x4a, 0x8c, 0xd6, 0x30,
      0x84, 0xac, 0xbe, 0x7e, 0xac, 0x0c, 0x4d, 0x62, 0x5a, 0xb4, 0xe1,
      0xa7, 0x6e, 0x62, 0x4a, 0x87, 0x98, 0xcb, 0x63, 0x49, 0x7b};

  // Encode public key in Sia format

  uint8_t pubkey_buf[57] = {0};
  size_t offset = 0;
  pubkey_buf[offset++] = 0x00;
  memcpy(pubkey_buf + offset, "ed25519", 7);
  offset += 7;
  memset(pubkey_buf + offset, 0, 9);
  offset += 9;
  pubkey_buf[offset++] = 32;
  memset(pubkey_buf + offset, 0, 7);
  offset += 7;
  memcpy(pubkey_buf + offset, public_key, 32);
  offset += 32;

  uint8_t pubkey_hash[32] = {0};
  if (blake2b(pubkey_buf, sizeof(pubkey_buf), pubkey_hash, 32) != 0) {
    return false;
  }

  // Build Merkle tree: timelock + pubkey
  uint8_t merkle_buf[65] = {0};
  merkle_buf[0] = 0x01;

  // Complete tree: (timelock+pubkey) + sigsrequired
  memcpy(merkle_buf + 1, timelock_hash, 32);
  memcpy(merkle_buf + 33, pubkey_hash, 32);

  uint8_t intermediate[32] = {0};
  if (blake2b(merkle_buf, sizeof(merkle_buf), intermediate, 32) != 0) {
    return false;
  }

  // Complete tree: (timelock + pubkey) + sigsrequired
  memset(merkle_buf, 0, sizeof(merkle_buf));
  merkle_buf[0] = 0x01;
  memcpy(merkle_buf + 1, intermediate, 32);
  memcpy(merkle_buf + 33, sigsrequired_hash, 32);

  uint8_t addr[32] = {0};
  if (blake2b(merkle_buf, sizeof(merkle_buf), addr, 32) != 0) {
    return false;
  }

  // Add checksum and convert to hex string
  return sia_full_address(addr, address);
}