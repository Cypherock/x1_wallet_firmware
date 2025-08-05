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

static void read_stellar_account(const uint8_t *data,
                                 uint32_t *offset,
                                 uint8_t *account) {
  memcpy(account, data + *offset, STELLAR_PUBKEY_RAW_SIZE);
  *offset += STELLAR_PUBKEY_RAW_SIZE;
}

static uint32_t read_xdr_string(const uint8_t *data,
                                uint32_t *offset,
                                char *str,
                                uint32_t max_len,
                                uint32_t data_len) {
  if (*offset + 4 > data_len) {
    return 0;
  }

  uint32_t len = U32_READ_BE_ARRAY(data + *offset);
  *offset += 4;

  if (len >= max_len || *offset + len > data_len) {
    return 0;
  }

  // Calculate padded length for 4-byte XDR alignment
  // See
  // https://github.com/stellar/js-xdr/blob/master/src/serialization/xdr-writer.js#L110
  uint32_t padded_len = ((len + 3) / 4) * 4;
  if (*offset + padded_len > data_len) {
    return 0;
  }

  memcpy(str, data + *offset, len);
  str[len] = '\0';
  *offset += padded_len;

  return len;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

static int parse_memo_data(const uint8_t *xdr,
                           uint32_t *offset,
                           uint32_t xdr_len,
                           stellar_transaction_t *txn) {
  txn->memo_type = (stellar_memo_type_t)U32_READ_BE_ARRAY(xdr + *offset);
  *offset += 4;

  switch (txn->memo_type) {
    case STELLAR_MEMO_NONE:
      break;

    case STELLAR_MEMO_TEXT: {
      char temp_memo[64] = "";
      uint32_t memo_len =
          read_xdr_string(xdr, offset, temp_memo, sizeof(temp_memo), xdr_len);
      if (memo_len == 0 || memo_len >= sizeof(txn->memo.text)) {
        return -1;
      }
      strncpy(txn->memo.text, temp_memo, memo_len);
      txn->memo.text[memo_len] = '\0';
    } break;

    case STELLAR_MEMO_ID:
      txn->memo.id = U64_READ_BE_ARRAY(xdr + *offset);
      *offset += 8;
      break;

    case STELLAR_MEMO_HASH:
    case STELLAR_MEMO_RETURN:
      if (*offset + 32 > xdr_len) {
        return -1;
      }
      memcpy(txn->memo.hash, xdr + *offset, 32);
      *offset += 32;
      break;

    default:
      return -1;
  }

  return 0;
}

static int parse_operation_data(const uint8_t *xdr,
                                uint32_t *offset,
                                stellar_transaction_t *txn) {
  // Parse Operations count
  txn->operation_count = U32_READ_BE_ARRAY(xdr + *offset);
  *offset += 4;

  if (txn->operation_count != 1) {
    return -1;
  }

  // Parse First Operation
  uint32_t has_source_account = U32_READ_BE_ARRAY(xdr + *offset);
  *offset += 4;

  if (has_source_account == 1) {
    uint32_t op_source_type = U32_READ_BE_ARRAY(xdr + *offset);
    *offset += 4;

    if (op_source_type != STELLAR_KEY_TYPE_ED25519) {
      return -1;
    }

    if (memcmp(xdr + *offset, txn->source_account, STELLAR_PUBKEY_RAW_SIZE) !=
        0) {
      return -1;
    }

    *offset += STELLAR_PUBKEY_RAW_SIZE;
  } else if (has_source_account != 0) {
    return -1;
  }

  // Parse operation type
  uint32_t operation_type = U32_READ_BE_ARRAY(xdr + *offset);
  *offset += 4;

  if (operation_type != STELLAR_OPERATION_PAYMENT &&
      operation_type != STELLAR_OPERATION_CREATE_ACCOUNT) {
    return -1;
  }
  txn->operations[0].type = (stellar_operation_type_t)operation_type;

  // Parse destination account (common for both operations)
  uint32_t dest_account_type = U32_READ_BE_ARRAY(xdr + *offset);
  *offset += 4;
  if (dest_account_type != STELLAR_KEY_TYPE_ED25519) {
    return -1;
  }
  read_stellar_account(xdr, offset, txn->operations[0].destination);

  // For PAYMENT operations, we need to parse the asset type
  if (operation_type == STELLAR_OPERATION_PAYMENT) {
    uint32_t asset_type = U32_READ_BE_ARRAY(xdr + *offset);
    *offset += 4;
    if (asset_type != STELLAR_ASSET_TYPE_NATIVE) {
      return -1;
    }
  }

  // Parse amount (common for both operations)
  txn->operations[0].amount = U64_READ_BE_ARRAY(xdr + *offset);
  *offset += 8;

  return 0;
}

int stellar_parse_transaction(const uint8_t *xdr,
                              uint32_t xdr_len,
                              stellar_transaction_t *txn,
                              uint32_t *txn_signature_data_len) {
  if (!xdr || !txn || xdr_len < 60) {
    return -1;
  }

  uint32_t offset = 0;

  memset(txn, 0, sizeof(stellar_transaction_t));

  // Parse Envelope Type (4 bytes)
  uint32_t envelope_type = U32_READ_BE_ARRAY(xdr + offset);
  offset += 4;
  if (envelope_type != STELLAR_ENVELOPE_TYPE_TX) {
    return -1;
  }

  // Parse Source Account
  uint32_t source_account_type = U32_READ_BE_ARRAY(xdr + offset);
  offset += 4;
  if (source_account_type != STELLAR_KEY_TYPE_ED25519) {
    return -1;
  }

  read_stellar_account(xdr, &offset, txn->source_account);

  // Parse Fee (4 bytes)
  txn->fee = U32_READ_BE_ARRAY(xdr + offset);
  offset += 4;

  // Parse Sequence Number (8 bytes)
  txn->sequence_number = U64_READ_BE_ARRAY(xdr + offset);
  offset += 8;

  // Parse Preconditions
  uint32_t preconditions_type = U32_READ_BE_ARRAY(xdr + offset);
  offset += 4;

  if (preconditions_type == 1) {
    offset += 16;    // Skip time bounds (8 + 8 bytes)
  } else if (preconditions_type != 0) {
    return -1;
  }

  // Parse Memo
  if (parse_memo_data(xdr, &offset, xdr_len, txn) != 0) {
    return -1;
  }

  // Parse Operations
  int result = parse_operation_data(xdr, &offset, txn);
  if (result != 0) {
    return result;
  }

  // Skip transaction extension
  if (offset + 4 <= xdr_len) {
    offset += 4;    // Just skip the extension, don't need to read the value
  }

  *txn_signature_data_len = offset;
  return 0;
}