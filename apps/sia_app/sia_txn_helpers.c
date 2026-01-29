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

double sia_convert_to_sc(uint64_t lo, uint64_t hi) {
  // Converting SC to hastings and is approximate for display purposes
  if (hi == 0) {
    return (double)lo / 1e24;
  } else {
    double hi_part = (double)hi * (double)(1ULL << 32) * (double)(1ULL << 32);
    return (hi_part + (double)lo) / 1e24;
  }
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

  if (txn->output_count > SIA_MAX_OUTPUTS) {
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