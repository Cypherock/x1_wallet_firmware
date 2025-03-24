/**
 * @file    icp_txn_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Helper functions for the ICP app for txn signing flow
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

#include "icp_txn_helpers.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "icp_context.h"
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

/**
 * Hash a string using SHA-256.
 */
void hash_string(const char *value, uint8_t *hash) {
  sha256_Raw((const uint8_t *)value, strlen(value), hash);
}

/**
 * Hash a given LEB128-encoded integer.
 */
void hash_leb128(uint64_t value, uint8_t *hash) {
  uint8_t buffer[10] = {0};
  size_t offset = 0;

  do {
    buffer[offset] = (value & 0x7F) | (value >= 0x80 ? 0x80 : 0);
    value >>= 7;
    offset++;
  } while (value > 0);

  sha256_Raw(buffer, offset, hash);
}

/**
 * Comparison function for sorting hash pairs.
 */
int compare_hashes(const void *a, const void *b) {
  return memcmp(((hash_pair_t *)a)->key_hash,
                ((hash_pair_t *)b)->key_hash,
                SHA256_DIGEST_LENGTH);
}

uint64_t decode_vec(int64_t type,
                    const uint8_t *data,
                    size_t *offset,
                    uint8_t *res) {
  switch (type) {
    case Nat8: {
      uint64_t len = leb_decode(data, offset);
      memcpy(res, data + *offset, len);
      (*offset) += len;
      return len;
    }
    default:
      break;
  }
  return 0;
}

bool read_recipient_account_id(const uint8_t *data,
                               size_t *offset,
                               icp_txn_context_t *icp_txn_context) {
  if (icp_txn_context->is_token_transfer_txn) {
    bool has_owner = *(data + *offset);
    (*offset)++;

    if (!has_owner) {
      return false;
    }

    icp_token_transfer_t *txn = icp_txn_context->raw_icp_token_transfer_txn;

    if (!decode_vec(Nat8, data, offset, txn->to.owner)) {
      return false;
    }

    txn->to.has_subaccount = *(data + *offset);
    (*offset)++;

    if (txn->to.has_subaccount) {
      return decode_vec(Nat8, data, offset, txn->to.subaccount);
    }
    return true;
  }

  return decode_vec(
      Nat8, data, offset, icp_txn_context->raw_icp_coin_transfer_txn->to);
}

bool read_amount_value(const uint8_t *data,
                       size_t *offset,
                       icp_txn_context_t *icp_txn_context) {
  if (icp_txn_context->is_token_transfer_txn) {
    icp_txn_context->raw_icp_token_transfer_txn->amount =
        leb_decode(data, offset);
  } else {
    icp_txn_context->raw_icp_coin_transfer_txn->amount.e8s =
        U64_READ_LE_ARRAY(data + *offset);
    (*offset) += 8;
  }
  return true;
}

bool read_fee_value(const uint8_t *data,
                    size_t *offset,
                    icp_txn_context_t *icp_txn_context) {
  if (icp_txn_context->is_token_transfer_txn) {
    icp_token_transfer_t *txn = icp_txn_context->raw_icp_token_transfer_txn;

    txn->has_fee = *(data + *offset);
    (*offset)++;

    if (txn->has_fee) {
      icp_txn_context->raw_icp_token_transfer_txn->fee =
          leb_decode(data, offset);
    }
  } else {
    icp_txn_context->raw_icp_coin_transfer_txn->fee.e8s =
        U64_READ_LE_ARRAY(data + *offset);
    (*offset) += 8;
  }
  return true;
}

bool read_memo_value(const uint8_t *data,
                     size_t *offset,
                     icp_txn_context_t *icp_txn_context) {
  if (icp_txn_context->is_token_transfer_txn) {
    bool has_memo = *(data + *offset);
    (*offset)++;

    if (has_memo) {
      icp_txn_context->raw_icp_token_transfer_txn->has_memo = has_memo;
      uint64_t decoded_len =
          decode_vec(Nat8,
                     data,
                     offset,
                     icp_txn_context->raw_icp_token_transfer_txn->memo.bytes);
      if (!decoded_len) {
        return false;
      }
      icp_txn_context->raw_icp_token_transfer_txn->memo.size = decoded_len;
    }
  } else {
    icp_txn_context->raw_icp_coin_transfer_txn->memo =
        U64_READ_LE_ARRAY(data + *offset);
    (*offset) += 8;
  }
  return true;
}

bool read_from_subaccount_value(const uint8_t *data,
                                size_t *offset,
                                icp_txn_context_t *icp_txn_context) {
  bool has_from_subaccount = *(data + *offset);
  (*offset)++;

  if (has_from_subaccount) {
    uint8_t *from_subaccount = NULL;
    if (icp_txn_context->is_token_transfer_txn) {
      icp_txn_context->raw_icp_token_transfer_txn->has_from_subaccount =
          has_from_subaccount;
      from_subaccount =
          icp_txn_context->raw_icp_token_transfer_txn->from_subaccount;
    } else {
      icp_txn_context->raw_icp_coin_transfer_txn->has_from_subaccount =
          has_from_subaccount;
      from_subaccount =
          icp_txn_context->raw_icp_coin_transfer_txn->from_subaccount;
    }

    return decode_vec(Nat8, data, offset, from_subaccount);
  }
  return true;
}

bool read_created_at_time_value(const uint8_t *data,
                                size_t *offset,
                                icp_txn_context_t *icp_txn_context) {
  bool has_created_at_time = *(data + *offset);
  (*offset)++;

  if (has_created_at_time) {
    uint64_t timestamp_nanos = U64_READ_LE_ARRAY(data + *offset);
    (*offset) += 8;

    if (icp_txn_context->is_token_transfer_txn) {
      icp_txn_context->raw_icp_token_transfer_txn->has_created_at_time =
          has_created_at_time;
      icp_txn_context->raw_icp_token_transfer_txn->created_at_time =
          timestamp_nanos;
    } else {
      icp_txn_context->raw_icp_coin_transfer_txn->has_created_at_time =
          has_created_at_time;
      icp_txn_context->raw_icp_coin_transfer_txn->created_at_time
          .timestamp_nanos = timestamp_nanos;
    }
  }

  return true;
}

/// Reference:
// https://github.com/dfinity/agent-js/blob/main/packages/candid/src/utils/leb128.ts#L74
uint64_t leb_decode(const uint8_t *buffer, size_t *offset) {
  uint64_t result = 0;
  int shift = 0;
  uint8_t byte;
  do {
    byte = buffer[*offset];
    (*offset)++;
    result |= ((uint64_t)(byte & 0x7F) << shift);
    shift += 7;
  } while ((byte & 0x80) != 0);
  return result;
}

/// Reference:
// https://github.com/dfinity/agent-js/blob/main/packages/candid/src/utils/leb128.ts#L135
int64_t sleb_decode(const uint8_t *buffer, size_t *offset) {
  int64_t result = 0;
  int shift = 0;
  uint8_t byte;

  while (1) {
    byte = buffer[*offset];
    (*offset)++;
    result |= (int64_t)(byte & 0x7F) << shift;
    shift += 7;

    // If MSB is not set, break (last byte)
    if ((byte & 0x80) == 0) {
      // Check if the sign bit (0x40) is set for negative numbers
      if (shift < 64 && (byte & 0x40)) {
        result |= -((int64_t)1 << shift);
      }
      break;
    }
  }
  return result;
}

/// Reference:
// https://github.com/dfinity/agent-js/blob/main/packages/candid/src/idl.ts#L1588
// @TODO: add unit tests for parser
bool icp_parse_transfer_txn(const uint8_t *byte_array,
                            uint16_t byte_array_size,
                            icp_txn_context_t *icp_txn_context) {
  size_t offset = 0;

  // Verify the "DIDL" Magic Number
  if (memcmp(byte_array, MAGIC_NUMBER, 4) != 0) {
    return false;
  }
  offset += 4;

  // Decode Type Table
  size_t num_types = leb_decode(byte_array, &offset);
  IDL_complex_type_t type_table[num_types];

  for (size_t i = 0; i < num_types; i++) {
    int64_t type = sleb_decode(byte_array, &offset);

    switch (type) {
      case Opt:
      case Vector: {
        int64_t child_type = sleb_decode(byte_array, &offset);
        IDL_complex_type_t c_ty;
        c_ty.type_id = type;
        c_ty.child_type = child_type;
        type_table[i] = c_ty;
        break;
      }
      case Record: {
        IDL_complex_type_t c_ty;
        c_ty.type_id = type;
        c_ty.num_fields = leb_decode(byte_array, &offset);
        c_ty.fields =
            (record_field_t *)malloc(sizeof(record_field_t) * c_ty.num_fields);
        for (int j = 0; j < c_ty.num_fields; j++) {
          uint64_t hash = leb_decode(byte_array, &offset);
          int64_t field_type = sleb_decode(byte_array, &offset);
          record_field_t field;
          field.key_hash = hash;
          field.type = field_type;
          c_ty.fields[j] = field;
        }

        type_table[i] = c_ty;
        break;
      }
      default:
        return false;
    }
  }

  uint64_t arg_count = leb_decode(byte_array, &offset);
  // only 1 argument supported
  if (arg_count != 1) {
    return false;
  }

  uint64_t arg_type_index = leb_decode(byte_array, &offset);

  if (arg_type_index < 0 && arg_type_index >= num_types) {
    return false;
  }

  IDL_complex_type_t arg_type = type_table[arg_type_index];

  // we expect a single record of 4-6 fields (to, amount, fee, memo,
  // from_subaccount, created_at_time)
  if (arg_type.type_id != Record || arg_type.num_fields < 4 ||
      arg_type.num_fields > 6) {
    return false;
  }

  // Decode Transfer Args
  for (int i = 0; i < arg_type.num_fields; i++) {
    record_field_t field = arg_type.fields[i];
    switch (field.key_hash) {
      case transfer_hash_to:
        // we can also verify the type
        // not doing right now
        if (!read_recipient_account_id(byte_array, &offset, icp_txn_context)) {
          return false;
        }
        break;
      case transfer_hash_amount:
        if (!read_amount_value(byte_array, &offset, icp_txn_context)) {
          return false;
        }
        break;
      case transfer_hash_fee:
        if (!read_fee_value(byte_array, &offset, icp_txn_context)) {
          return false;
        }
        break;
      case transfer_hash_memo:
        if (!read_memo_value(byte_array, &offset, icp_txn_context)) {
          return false;
        }
        break;
      case transfer_hash_from_subaccount:
        if (!read_from_subaccount_value(byte_array, &offset, icp_txn_context)) {
          return false;
        }
        break;
      case transfer_hash_created_at_time:
        if (!read_created_at_time_value(byte_array, &offset, icp_txn_context)) {
          return false;
        }
        break;
      default:
        return false;
    }
  }

  if (offset != byte_array_size) {
    return false;
  }

  // free memory
  for (size_t i = 0; i < num_types; i++) {
    if (type_table[i].fields) {
      free(type_table[i].fields);
      type_table[i].fields = NULL;
    }
  }

  return true;
}

/// Reference:
// https://github.com/dfinity/agent-js/blob/main/packages/agent/src/request_id.ts#L87
void hash_icp_transfer_request(const icp_transfer_request_t *request,
                               uint8_t *hash) {
  hash_pair_t pairs[NUM_FIELDS_IN_ICP_TRANSFER_REQUEST] = {0};
  size_t pair_count = 0;

  hash_string("request_type", pairs[pair_count].key_hash);
  sha256_Raw(request->request_type,
             sizeof(request->request_type),
             pairs[pair_count].value_hash);
  pair_count++;

  hash_string("canister_id", pairs[pair_count].key_hash);
  sha256_Raw(request->canister_id.bytes,
             request->canister_id.size,
             pairs[pair_count].value_hash);
  pair_count++;

  hash_string("method_name", pairs[pair_count].key_hash);
  sha256_Raw(request->method_name.bytes,
             request->method_name.size,
             pairs[pair_count].value_hash);
  pair_count++;

  hash_string("sender", pairs[pair_count].key_hash);
  sha256_Raw(
      request->sender, sizeof(request->sender), pairs[pair_count].value_hash);
  pair_count++;

  hash_string("ingress_expiry", pairs[pair_count].key_hash);
  sha256_Raw(request->ingress_expiry.bytes,
             request->ingress_expiry.size,
             pairs[pair_count].value_hash);
  pair_count++;

  hash_string("nonce", pairs[pair_count].key_hash);
  sha256_Raw(
      request->nonce, sizeof(request->nonce), pairs[pair_count].value_hash);
  pair_count++;

  hash_string("arg", pairs[pair_count].key_hash);
  sha256_Raw(
      request->arg.bytes, request->arg.size, pairs[pair_count].value_hash);
  pair_count++;

  // Sort key-value pairs by key hash
  qsort(pairs, pair_count, sizeof(hash_pair_t), compare_hashes);

  uint8_t concatenated[MAX_CONCATENATED_ICP_REQUEST_HASHES_SIZE] = {0};
  size_t offset = 0;
  for (size_t i = 0; i < pair_count; i++) {
    memcpy(&concatenated[offset], pairs[i].key_hash, SHA256_DIGEST_LENGTH);
    offset += SHA256_DIGEST_LENGTH;
    memcpy(&concatenated[offset], pairs[i].value_hash, SHA256_DIGEST_LENGTH);
    offset += SHA256_DIGEST_LENGTH;
  }

  sha256_Raw(concatenated, offset, hash);
}

/// Reference:
// https://github.com/dfinity/agent-js/blob/main/packages/agent/src/request_id.ts#L87
void hash_icp_read_state_request(const icp_read_state_request_t *request,
                                 uint8_t *hash) {
  hash_pair_t pairs[NUM_FIELDS_IN_ICP_READ_STATE_REQUEST] = {0};
  size_t pair_count = 0;

  hash_string("request_type", pairs[pair_count].key_hash);
  sha256_Raw((const uint8_t *)request->request_type,
             strlen(request->request_type),
             pairs[pair_count].value_hash);
  pair_count++;

  hash_string("sender", pairs[pair_count].key_hash);
  sha256_Raw(
      request->sender, sizeof(request->sender), pairs[pair_count].value_hash);
  pair_count++;

  hash_string("ingress_expiry", pairs[pair_count].key_hash);
  sha256_Raw(request->ingress_expiry.bytes,
             request->ingress_expiry.size,
             pairs[pair_count].value_hash);
  pair_count++;

  hash_string("paths", pairs[pair_count].key_hash);
  uint8_t path_hashes[SHA256_DIGEST_LENGTH * request->path_count];

  for (size_t path_index = 0; path_index < request->path_count; path_index++) {
    size_t segment_count = request->paths[path_index].segment_count;
    uint8_t segment_hashes[SHA256_DIGEST_LENGTH * segment_count];

    for (size_t segment_index = 0; segment_index < segment_count;
         segment_index++) {
      sha256_Raw(request->paths[path_index].segments[segment_index].bytes,
                 request->paths[path_index].segments[segment_index].size,
                 segment_hashes + SHA256_DIGEST_LENGTH * segment_index);
    }

    sha256_Raw(segment_hashes,
               sizeof(segment_hashes),
               path_hashes + SHA256_DIGEST_LENGTH * path_index);
  }

  sha256_Raw(path_hashes, sizeof(path_hashes), pairs[pair_count].value_hash);
  pair_count++;

  // Sort key-value pairs by key hash
  qsort(pairs, pair_count, sizeof(hash_pair_t), compare_hashes);

  uint8_t concatenated[MAX_CONCATENATED_ICP_REQUEST_HASHES_SIZE] = {0};
  size_t offset = 0;
  for (size_t i = 0; i < pair_count; i++) {
    memcpy(&concatenated[offset], pairs[i].key_hash, SHA256_DIGEST_LENGTH);
    offset += SHA256_DIGEST_LENGTH;
    memcpy(&concatenated[offset], pairs[i].value_hash, SHA256_DIGEST_LENGTH);
    offset += SHA256_DIGEST_LENGTH;
  }

  sha256_Raw(concatenated, offset, hash);
}

/// Reference:
// https://github.com/dfinity/agent-js/blob/main/packages/agent/src/agent/http/index.ts#L1030
void get_icp_read_state_request_id(
    uint8_t *read_state_request_id,
    const uint8_t *transfer_request_id,
    size_t transfer_request_id_len,
    const icp_transfer_request_t *transfer_request) {
  icp_read_state_request_t read_state_req;

  // request_type
  read_state_req.request_type = "read_state";

  // path = ["request_status", requestId]
  // paths = [path]
  // see
  // https://github.com/dfinity/agent-js/blob/main/packages/agent/src/polling/index.ts#L86
  read_state_req.path_count = 1;

  read_state_req.paths[0].segment_count = 2;

  const char *status_str = "request_status";
  size_t status_len = strlen(status_str);
  memcpy(read_state_req.paths[0].segments[0].bytes, status_str, status_len);
  read_state_req.paths[0].segments[0].size = status_len;

  memcpy(read_state_req.paths[0].segments[1].bytes,
         transfer_request_id,
         transfer_request_id_len);
  read_state_req.paths[0].segments[1].size = transfer_request_id_len;

  // sender
  memcpy(read_state_req.sender,
         transfer_request->sender,
         sizeof(transfer_request->sender));

  // ingress_expiry
  memcpy(read_state_req.ingress_expiry.bytes,
         transfer_request->ingress_expiry.bytes,
         transfer_request->ingress_expiry.size);
  read_state_req.ingress_expiry.size = transfer_request->ingress_expiry.size;

  hash_icp_read_state_request(&read_state_req, read_state_request_id);
}