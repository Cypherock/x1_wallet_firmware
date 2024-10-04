/**
 * @file    xrp_txn_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the XRP app for txn signing flow
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

#include "xrp_txn_helpers.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "xrp_context.h"

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

uint16_t decode_length(const uint8_t *byte_data, uint32_t *length) {
  // See
  // https://xrpl.org/docs/references/protocol/binary-format#length-prefixing

  uint32_t len = 0;
  uint16_t bytes_read = 0;

  uint8_t byte1 = byte_data[0];
  if (byte1 <= 192) {
    len = byte1;
    bytes_read = 1;
  } else if (byte1 <= 240) {
    uint8_t byte2 = byte_data[1];
    len = 193 + ((byte1 - 193) * 256) + byte2;
    bytes_read = 2;
  } else if (byte1 <= 254) {
    uint8_t byte2 = byte_data[1];
    uint8_t byte3 = byte_data[2];
    len = 12481 + ((byte1 - 241) * 65536) + (byte2 * 256) + byte3;
    bytes_read = 3;
  }
  *length = len;

  return bytes_read;
}

uint16_t decode_field_type(const uint8_t *byte_data,
                           uint8_t *type_code,
                           uint8_t *field_code) {
  // See https://xrpl.org/docs/references/protocol/binary-format#field-ids

  uint8_t high_4_bits = byte_data[0] >> 4;
  uint8_t low_4_bits = byte_data[0] & 0x0f;

  uint8_t type = 0;
  uint8_t field = 0;
  uint16_t bytes_read = 0;

  if (high_4_bits != 0 && low_4_bits != 0) {
    type = high_4_bits;
    field = low_4_bits;
    bytes_read = 1;
  } else if (high_4_bits == 0 && low_4_bits != 0) {
    field = low_4_bits;
    type = byte_data[1];
    bytes_read = 2;
  } else if (high_4_bits != 0 && low_4_bits == 0) {
    type = high_4_bits;
    field = byte_data[1];
    bytes_read = 2;
  } else if (high_4_bits == 0 && low_4_bits == 0) {
    type = byte_data[1];
    field = byte_data[2];
    bytes_read = 3;
  }

  *type_code = type;
  *field_code = field;

  return bytes_read;
}

uint16_t fill_INT16_type(const uint8_t *byte_data,
                         uint8_t field_code,
                         xrp_unsigned_txn *txn) {
  uint16_t decoded = U16_READ_BE_ARRAY(byte_data);
  switch (field_code) {
    case TransactionType: {
      txn->TransactionType = decoded;
      break;
    }
    default: {
      return 0;
    }
  }
  return 2;    // 16 bits = 2 bytes
}

uint16_t fill_INT32_type(const uint8_t *byte_data,
                         uint8_t field_code,
                         xrp_unsigned_txn *txn) {
  uint32_t decoded = U32_READ_BE_ARRAY(byte_data);
  switch (field_code) {
    case Flags: {
      txn->Flags = decoded;
      break;
    }
    case Sequence: {
      txn->Sequence = decoded;
      break;
    }
    case DestinationTag: {
      txn->hasDestinationTag = true;
      txn->DestinationTag = decoded;
      break;
    }
    case LastLedgerSequence: {
      txn->LastLedgerSequence = decoded;
      break;
    }
    default: {
      return 0;
    }
  }

  return 4;    // 32 bits = 4 bytes
}

uint16_t fill_AMOUNT_type(const uint8_t *byte_data,
                          uint8_t field_code,
                          xrp_unsigned_txn *txn) {
  // See https://xrpl.org/docs/references/protocol/binary-format#amount-fields

  uint8_t not_xrp = byte_data[0] >> 7;    // get the first bit
  if (not_xrp) {
    // means amount is not plain xrp. It is token currency amount which is not
    // supported yet.
    return 0;
  }

  uint64_t decoded = U64_READ_BE_ARRAY(byte_data);
  decoded &= 0xafffffffffffffff;    // for xrp

  switch (field_code) {
    case Amount: {
      txn->Amount = decoded;
      break;
    }
    case Fee: {
      txn->Fee = decoded;
      break;
    }
    default: {
      return 0;
    }
  }

  return 8;    // xrp amount is of fixed 64 bits = 8 bytes
}

uint16_t fill_BLOB_type(const uint8_t *byte_data,
                        uint8_t field_code,
                        xrp_unsigned_txn *txn) {
  // See https://xrpl.org/docs/references/protocol/binary-format#blob-fields

  uint32_t length = 0;
  uint16_t bytes_read = decode_length(byte_data, &length);
  if (!bytes_read || !length) {
    return 0;
  }

  switch (field_code) {
    case SigningPubKey: {
      memcpy(txn->SigningPubKey, byte_data, length);
      break;
    }
    default: {
      return 0;
    }
  }
  bytes_read += length;

  return bytes_read;
}

uint16_t fill_ACCOUNT_type(const uint8_t *byte_data,
                           uint8_t field_code,
                           xrp_unsigned_txn *txn) {
  // See
  // https://xrpl.org/docs/references/protocol/binary-format#accountid-fields

  uint32_t length = 0;
  uint16_t bytes_read = decode_length(byte_data, &length);
  if (!bytes_read || !length) {
    return 0;
  }

  switch (field_code) {
    case Account: {
      memcpy(txn->Account, byte_data, length);
      break;
    }
    case Destination: {
      memcpy(txn->Destination, byte_data, length);
      break;
    }
    default: {
      return 0;
    }
  }
  bytes_read += length;

  return bytes_read;
}

bool xrp_parse_transaction(const uint8_t *byte_array,
                           uint16_t byte_array_size,
                           xrp_unsigned_txn *txn) {
  if (byte_array == NULL || txn == NULL) {
    return false;
  }

  // See https://xrpl.org/docs/references/protocol/binary-format

  txn->hasDestinationTag = false;

  uint16_t offset = 0;
  offset += 4;    // network prefix

  while (offset < byte_array_size) {
    uint8_t type_code;
    uint8_t field_code;
    uint16_t bytes_read =
        decode_field_type(byte_array + offset, &type_code, &field_code);
    if (!bytes_read) {
      return false;
    }

    offset += bytes_read;

    switch (type_code) {
      case INT16: {
        bytes_read = fill_INT16_type(byte_array + offset, field_code, txn);
        if (!bytes_read) {
          return false;
        }
        offset += bytes_read;
        break;
      }
      case INT32: {
        bytes_read = fill_INT32_type(byte_array + offset, field_code, txn);
        if (!bytes_read) {
          return false;
        }
        offset += bytes_read;
        break;
      }
      case AMOUNT: {
        bytes_read = fill_AMOUNT_type(byte_array + offset, field_code, txn);
        if (!bytes_read) {
          return false;
        }
        offset += bytes_read;
        break;
      }
      case BLOB: {
        bytes_read = fill_BLOB_type(byte_array + offset, field_code, txn);
        if (!bytes_read) {
          return false;
        }
        offset += bytes_read;
        break;
      }
      case ACCOUNT: {
        bytes_read = fill_ACCOUNT_type(byte_array + offset, field_code, txn);
        if (!bytes_read) {
          return false;
        }
        offset += bytes_read;
        break;
      }
      default:
        return false;
    }
  }

  if (offset > byte_array_size) {
    return false;
  }

  return true;
}
