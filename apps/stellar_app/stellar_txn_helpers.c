/**
 * @file    stellar_txn_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the STELLAR app for txn signing flow
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

#include "stellar_txn_helpers.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stellar_context.h"
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

static uint32_t read_uint32_be_offset(const uint8_t *data, int *offset) {
  uint32_t value = U32_READ_BE_ARRAY(data + *offset);
  *offset += 4;
  return value;
}

static uint64_t read_uint64_be_offset(const uint8_t *data, int *offset) {
  uint64_t high = read_uint32_be_offset(data, offset);
  uint64_t low = read_uint32_be_offset(data, offset);
  return (high << 32) | low;
}

static void read_account_offset(const uint8_t *data,
                                int *offset,
                                uint8_t *account) {
  memcpy(account, data + *offset, STELLAR_PUBKEY_RAW_SIZE);
  *offset += STELLAR_PUBKEY_RAW_SIZE;
}

static int read_string_offset(const uint8_t *data,
                              int *offset,
                              char *str,
                              int max_len,
                              int data_len) {
  if (*offset + 4 > data_len)
    return -1;

  uint32_t len = read_uint32_be_offset(data, offset);
  if (len >= max_len || *offset + len > data_len)
    return -1;

  // Calculate padded length (round up to 4-byte boundary)
  int padded_len = ((len + 3) / 4) * 4;
  if (*offset + padded_len > data_len)
    return -1;

  memcpy(str, data + *offset, len);
  str[len] = '\0';
  *offset += padded_len;    // Skip data + padding in one go

  return len;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

static int parse_memo_data(const uint8_t *xdr,
                           int *offset,
                           int xdr_len,
                           stellar_transaction_t *tx) {
  tx->memo_type = (stellar_memo_type_t)read_uint32_be_offset(xdr, offset);

  switch (tx->memo_type) {
    case STELLAR_MEMO_NONE:
      break;

    case STELLAR_MEMO_TEXT: {
      char temp_memo[64];
      int memo_len = read_string_offset(
          xdr, offset, temp_memo, sizeof(temp_memo), xdr_len);
      if (memo_len < 0) {
        return -1;
      }
      strncpy(tx->memo.text, temp_memo, sizeof(tx->memo.text) - 1);
      tx->memo.text[sizeof(tx->memo.text) - 1] = '\0';
    } break;

    case STELLAR_MEMO_ID:
      tx->memo.id = read_uint64_be_offset(xdr, offset);
      break;

    case STELLAR_MEMO_HASH:
    case STELLAR_MEMO_RETURN:
      if (*offset + 32 > xdr_len) {
        return -1;
      }
      memcpy(tx->memo.hash, xdr + *offset, 32);
      *offset += 32;
      break;

    default:
      return -1;
  }

  return 0;
}

static int parse_operation_data(const uint8_t *xdr,
                                int *offset,
                                int xdr_len,
                                stellar_transaction_t *tx,
                                stellar_payment_t *payment) {
  // Parse Operations count
  tx->operation_count = read_uint32_be_offset(xdr, offset);

  if (tx->operation_count == 0) {
    return -1;
  }

  // Parse First Operation
  uint32_t has_source_account = read_uint32_be_offset(xdr, offset);

  if (has_source_account == 1) {
    *offset += 36;    // Skip source account (4 bytes type + 32 bytes key)
  } else if (has_source_account != 0) {
    return -1;
  }

  // Parse operation type
  uint32_t operation_type = read_uint32_be_offset(xdr, offset);

  if (operation_type != STELLAR_OPERATION_PAYMENT &&
      operation_type != STELLAR_OPERATION_CREATE_ACCOUNT) {
    return -1;
  }
  tx->operation_type = (stellar_operation_type_t)operation_type;

  // Parse destination account (common for both operations)
  uint32_t dest_account_type = read_uint32_be_offset(xdr, offset);
  if (dest_account_type != STELLAR_KEY_TYPE_ED25519) {
    return -1;
  }
  read_account_offset(xdr, offset, payment->destination);

  // For PAYMENT operations, we need to parse the asset type
  if (operation_type == STELLAR_OPERATION_PAYMENT) {
    uint32_t asset_type = read_uint32_be_offset(xdr, offset);
    if (asset_type != STELLAR_ASSET_TYPE_NATIVE) {
      return -1;
    }
  }

  // Parse amount (common for both operations)
  payment->amount = read_uint64_be_offset(xdr, offset);

  return 0;
}

int stellar_parse_transaction(const uint8_t *xdr,
                              int xdr_len,
                              stellar_transaction_t *tx,
                              stellar_payment_t *payment) {
  if (!xdr || !tx || !payment || xdr_len < 60) {
    return -1;
  }

  int offset = 0;

  memset(tx, 0, sizeof(stellar_transaction_t));
  memset(payment, 0, sizeof(stellar_payment_t));

  // Parse Envelope Type (4 bytes)
  uint32_t envelope_type = read_uint32_be_offset(xdr, &offset);
  if (envelope_type != STELLAR_ENVELOPE_TYPE_TX) {
    return -1;
  }

  // Parse Source Account
  uint32_t source_account_type = read_uint32_be_offset(xdr, &offset);
  if (source_account_type != STELLAR_KEY_TYPE_ED25519) {
    return -1;
  }

  read_account_offset(xdr, &offset, tx->source_account);

  // Parse Fee (4 bytes)
  tx->fee = read_uint32_be_offset(xdr, &offset);

  // Parse Sequence Number (8 bytes)
  tx->sequence_number = read_uint64_be_offset(xdr, &offset);

  // Parse Preconditions
  uint32_t preconditions_type = read_uint32_be_offset(xdr, &offset);

  if (preconditions_type == 1) {
    offset += 16;    // Skip time bounds (8 + 8 bytes)
  } else if (preconditions_type != 0) {
    return -1;
  }

  // Parse Memo
  if (parse_memo_data(xdr, &offset, xdr_len, tx) != 0) {
    return -1;
  }

  // Parse Operations
  int result = parse_operation_data(xdr, &offset, xdr_len, tx, payment);
  if (result != 0) {
    return result;
  }

  // Skip transaction extension
  if (offset + 4 <= xdr_len) {
    read_uint32_be_offset(xdr, &offset);
  }

  return 0;
}