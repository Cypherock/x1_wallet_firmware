/**
 * @file    sia_txn_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the SIA app for txn signing flow
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

#include "sia_txn_helpers.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "blake2b.h"
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
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool sia_format_sc(uint64_t lo, uint64_t hi, char *out, size_t out_size) {
  // the 128-bit value as 32-bit limbs, most significant last
  uint32_t limbs[4] = {
      (uint32_t)lo, (uint32_t)(lo >> 32), (uint32_t)hi, (uint32_t)(hi >> 32)};
  // decimal digits, least significant first (2^128 has 39 digits)
  char digits[39 + 24 + 1] = {0};
  size_t count = 0;
  do {
    uint64_t remainder = 0;
    for (int i = 3; i >= 0; i--) {
      uint64_t current = (remainder << 32) | limbs[i];
      limbs[i] = (uint32_t)(current / 10);
      remainder = current % 10;
    }
    digits[count++] = (char)('0' + remainder);
  } while (0 != limbs[0] || 0 != limbs[1] || 0 != limbs[2] || 0 != limbs[3]);

  // at least one integer digit before the 24 decimals
  while (count < 24 + 1) {
    digits[count++] = '0';
  }

  // keep at least 6 decimals, and every non-zero one after them
  size_t lowest_kept = 0;
  while (lowest_kept < 24 - 6 && '0' == digits[lowest_kept]) {
    lowest_kept++;
  }

  size_t needed = (count - 24) + 1 + (24 - lowest_kept) + 1;
  if (NULL == out || needed > out_size) {
    return false;
  }
  size_t offset = 0;
  for (size_t i = count; i > 24; i--) {
    out[offset++] = digits[i - 1];
  }
  out[offset++] = '.';
  for (size_t i = 24; i > lowest_kept; i--) {
    out[offset++] = digits[i - 1];
  }
  out[offset] = '\0';
  return true;
}

bool sia_parse_transaction(const uint8_t *blob,
                           size_t blob_len,
                           sia_transaction_t *txn) {
  size_t offset = 0;

  // Parse input count
  if (blob_len < offset + 8)
    return false;
  txn->input_count = U64_READ_LE_ARRAY(blob + offset);
  offset += 8;

  if (txn->input_count > SIA_MAX_INPUTS) {
    return false;
  }

  for (int i = 0; i < txn->input_count; i++) {
    if (blob_len < offset + 32)
      return false;
    memcpy(txn->parent_ids[i], blob + offset, 32);
    offset += 32;
  }

  // Parse output count
  if (blob_len < offset + 8)
    return false;
  txn->output_count = U64_READ_LE_ARRAY(blob + offset);
  offset += 8;

  // the first output is the one shown to the user, so it must exist
  if (0 == txn->output_count || txn->output_count > SIA_MAX_OUTPUTS) {
    return false;
  }

  // Parse outputs
  for (int i = 0; i < txn->output_count; i++) {
    if (blob_len < offset + 48)
      return false;    // 32 + 8 + 8

    memcpy(txn->outputs[i].address_hash, blob + offset, 32);
    offset += 32;

    txn->outputs[i].value_lo = U64_READ_LE_ARRAY(blob + offset);
    offset += 8;

    txn->outputs[i].value_hi = U64_READ_LE_ARRAY(blob + offset);
    offset += 8;
  }

  // Parse fee
  if (blob_len < offset + 16)
    return false;
  txn->fee_lo = U64_READ_LE_ARRAY(blob + offset);
  offset += 8;
  txn->fee_hi = U64_READ_LE_ARRAY(blob + offset);
  offset += 8;

  return true;
}