/**
 * @file    canton_txn_encoding.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to Canton chains
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

#include "canton_txn_encoding.h"

#include <errno.h>
#include <sha2.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "canton/canton_prepared_transaction.pb.h"
#include "canton/canton_txn_nodes.pb.h"
#include "canton/canton_value.pb.h"
#include "canton/sign_txn.pb.h"
#include "canton_api.h"
#include "canton_context.h"
#include "canton_helpers.h"
#include "canton_priv.h"
#include "pb_decode.h"
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

/**
 * @brief Finds node seed with the given node id
 * @param node_id[in] Node id of the seed to find
 *
 * @returns Returns node seed if found otherwise null
 */
static inline const canton_node_seed_t *get_node_seed_from_node_id(
    int32_t node_id);

/**
 * @brief Finds hashed encoded node with the given node id
 * @param node_id[in] Node id of the node to find
 *
 * @returns Returns @ref canton_txn_node_hash_t instance if found otherwise null
 */
static inline const canton_txn_node_hash_t *get_hashed_encoded_node_from_id(
    int32_t node_id);

/**
 * @brief Encodes empty protobuf type
 * @param buf[out] Out buffer
 */
static inline void encode_empty(uint8_t *buf);

/**
 * @brief Encodes bool protobuf type
 * @param[in] value Boolean value
 * @param[out] buf Out buffer
 */
static inline void encode_bool(bool value, uint8_t *buf);

/**
 * @brief Encodes int32 protobuf type
 * @param value[in] int32 value
 * @param buf[out] Out buffer
 */
static inline void encode_int32(int32_t value, uint8_t *buf);

/**
 * @brief Encodes int64 protobuf type
 * @param value[in] int64 value
 * @param buf[out] Out buffer
 */
static inline void encode_int64(int64_t value, uint8_t *buf);

/**
 * @brief Encodes string to out buffer
 *
 * @param str[in] String to encode
 * @param out_buf[out] Out buffer to write to
 * @param out_len[out] Count of bytes written
 *
 * @return Bool indicating whether successfully written or not
 */
static bool encode_string(const char *str, uint8_t *out_buf, size_t *out_len);

/**
 * @brief Encodes raw bytes
 * @details length (as int32) + data is encoded
 *
 * @param data[in] Encoding bytes
 * @param len[in] Data length
 * @param out_buf[out] Out buffer
 * @param out_len[out] Total encoded length
 * @returns Bool indicating whether successfully encoded or not
 */
static bool encode_bytes(const uint8_t *data,
                         size_t len,
                         uint8_t *out_buf,
                         size_t *out_len);

/**
 * @brief Encodes hex string
 *
 * @param str[in] Hex string
 * @param out_buf[out] Out buffer
 * @param out_len[out] Total encoded length
 * @returns Bool indicating whether successfully encoded or not
 */
static bool encode_hex_string(const char *str,
                              uint8_t *out_buf,
                              size_t *out_len);

/**
 * @brief Encodes hash data (uint8_t buffer)
 *
 * @param hash[in] uint8_t hash buffer
 * @param hash_size[in] size of hash buffer
 * @param out_buf[out] Out buffer
 *
 */
static bool encode_hash(const uint8_t *hash,
                        size_t hash_size,
                        uint8_t *out_buf);

/**
 * @brief Computes canton identifier size
 *
 * @param value[in] Reference to @ref canton_value_t instance
 * @returns Computed size
 */
static size_t get_encoded_value_size(const canton_value_t *value);

/**
 * @brief Computes canton identifier size
 *
 * @param ident[in] Reference to @ref canton_identifier_t instance
 * @returns Computed size of identifier
 */
static size_t get_encoded_identifier_size(const canton_identifier_t *ident);

/**
 * @brief Computes canton fetch node size
 *
 * @param node[in] Reference to @ref canton_fetch_t instance
 * @param node_id[in] Node id of the given node
 *
 * @returns Computed size of fetch node
 */
static size_t get_encoded_fetch_node_size(const canton_fetch_t *node,
                                          int32_t node_id);

/**
 * @brief Computes canton exercise node size
 *
 * @param node[in] Reference to @ref canton_exercise_t instance
 * @param node_id[in] Node id of the given node
 *
 * @returns Computed size of exercise node
 */
static size_t get_encoded_exercise_node_size(const canton_exercise_t *node,
                                             int32_t node_id);

/**
 * @brief Computes canton create node size
 *
 * @param node[in] Reference to @ref canton_create_t instance
 * @param node_id[in] Node id of the given node
 *
 * @returns Computed size of create node
 */
static size_t get_encoded_create_node_size(const canton_create_t *node,
                                           int32_t node_id);

/**
 * @brief Computes canton rollback node size
 *
 * @param node[in] Reference to @ref canton_rollback_t instance
 * @param node_id[in] Node id of the given node
 *
 * @returns Computed size of rollback node
 */
static size_t get_encoded_rollback_node_size(const canton_rollback_t *node,
                                             int32_t node_id);

/**
 * @brief Computes canton metadata size
 *
 * @param metadata[in] Reference to @ref canton_metadata_t instance
 *
 * @returns Computed size of metadata
 */
static size_t get_encoded_metadata_size(
    const canton_sign_txn_canton_metadata_t *metadata);

/**
 * @brief Encodes value @ref canton_value_t to the given buffer
 *
 * @param value[in] Reference to @ref canton_value_t instance
 * @param out_buf[out] Buffer where encoded value to be written
 */
static void encode_value(const canton_value_t *value, uint8_t *out_buf);

/**
 * @brief Encodes identifier @ref canton_identifier_t in the given buffer
 *
 * @param ident[in] Reference to @ref canton_identifier_t instance
 * @param out_buf[out] Buffer where encoded value to be written
 * @param out_len[out] Count of bytes written
 */
static void encode_identifier(const canton_identifier_t *ident,
                              uint8_t *out_buf,
                              size_t *out_len);

/**
 * @brief Encodes @ref canton_fetch_t to a new buffer
 *
 * @param node[in] Reference to @ref canton_fetch_t instance
 * @param node_id[in] Id of the given node
 * @param out_len[out] Encoded buffer size
 *
 * @returns Heap allocated buffer with node encoded
 */
static uint8_t *encode_fetch_node(const canton_fetch_t *node,
                                  int32_t node_id,
                                  size_t *out_len);

/**
 * @brief Encodes @ref canton_exercise_t to a new buffer
 *
 * @param node[in] Reference to @ref canton_exercise_t instance
 * @param node_id[in] Id of the given node
 * @param out_len[out] Encoded buffer size
 *
 * @returns Heap allocated buffer with node encoded
 */
static uint8_t *encode_exercise_node(const canton_exercise_t *node,
                                     int32_t node_id,
                                     size_t *out_len);

/**
 * @brief Encodes @ref canton_create_t to a new buffer
 *
 * @param node[in] Reference to @ref canton_create_t instance
 * @param node_id[in] Id of the given node
 * @param out_len[out] Encoded buffer size
 *
 * @returns Heap allocated buffer with node encoded
 */
static uint8_t *encode_create_node(const canton_create_t *node,
                                   int32_t node_id,
                                   size_t *out_len);

/**
 * @brief Encodes @ref canton_rollback_t to a new buffer
 *
 * @param node[in] Reference to @ref canton_rollback_t instance
 * @param node_id[in] Id of the given node
 * @param out_len[out] Encoded buffer size
 *
 * @returns Heap allocated buffer with node encoded
 */
static uint8_t *encode_rollback_node(const canton_rollback_t *node,
                                     int32_t node_id,
                                     size_t *out_len);

/**
 * @brief Encodes generic node v1 @ref canton_node_t to a new buffer
 *
 * @param node[in] Reference to @ref canton_node_t instance
 * @param node_id[in] Id of the given node in char[10]
 * @param out_len[out] Encoded buffer size
 *
 * @returns Heap allocated buffer with node encoded
 */
static uint8_t *encode_node_v1(const canton_node_t *node,
                               char *node_id,
                               size_t *out_len);

/**
 * @brief Encodes generic daml transcation d node v1 @ref
 * canton_daml_transaction_d_node_t to a new buffer
 *
 * @param node[in] Reference to @ref canton_daml_transaction_d_node_t instance
 * @param out_len[out] Encoded buffer size
 *
 * @returns Heap allocated buffer with node encoded
 */
static uint8_t *encode_node(const canton_daml_transaction_d_node_t *node,
                            size_t *out_len);

/**
 * @brief Encodes metadata @ref canton_metadata_t to a new buffer
 *
 * @param metadata[in] Reference to @ref canton_metadata_t instance
 * @param out_len[out] Encoded buffer size
 *
 * @returns Heap allocated buffer with metadata encoded
 */
static uint8_t *encode_metadata(
    const canton_sign_txn_canton_metadata_t *metadata,
    size_t *out_len);

/**
 * @brief Parses and stores transaction fields which are to be displayed for
 * user verification For now it parses & stores Amount, Receiver address.
 *
 * @param value[in] Reference to @ref canton_value_t instance
 */
static void parse_display_info(const char *choice_id,
                               const canton_value_t *chosen_value);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

const static uint32_t PREPARED_TRANSACTION_HASH_PURPOSE = 0x30000000;
const static uint8_t HASHING_SCHEME_VERSION_V2 = 0x02;
const static uint8_t NODE_ENCODING_VERSION = 0x01;
const static uint8_t CREATE_NODE_TAG = 0x00;
const static uint8_t EXERCISE_NODE_TAG = 0x01;
const static uint8_t FETCH_NODE_TAG = 0x02;
const static uint8_t ROLLBACK_NODE_TAG = 0x03;

const static char *TRANSFER_CHOICE_ID = "TransferFactory_Transfer";
const static char *TAP_CHOICE_ID = "AmuletRules_DevNet_Tap";
const static char *WITHDRAW_CHOICE_ID = "TransferInstruction_Withdraw";
const static char *ACCEPT_CHOICE_ID = "TransferInstruction_Accept";
const static char *REJECT_CHOICE_ID = "TransferInstruction_Reject";
const static char *PREAPPROVAL_CHOICE_ID = "TransferPreapprovalProposal";
const static char *MERGE_DELEGATION_PROPOSAL_CHOICE_ID =
    "MergeDelegationProposal";
const static char *MERGE_DELEGATION_CHOICE_ID = "MergeDelegation";
const static char *CANTON_TRANSFER_INSTRUCTION = "AmuletTransferInstruction";
const static char *CANTON_TRANSFER_OFFER =
    "TransferOffer";    // Tokens use TransferOffer; atleast CBTC uses this;
                        // Verify when supporting other tokens
const static char *TRANSFER_LABEL = "transfer";
const static char *SENDER_LABEL = "sender";
const static char *RECEIVER_LABEL = "receiver";
const static char *AMOUNT_LABEL = "amount";
const static char *START_TIME_LABEL = "requestedAt";
const static char *EXPIRY_TIME_LABEL = "executeBefore";
const static char *INSTRUMENT_ID_LABEL = "instrumentId";
const static char *ADMIN_LABEL = "admin";
const static char *ID_LABEL = "id";
const static char *DELEGATION_LABEL = "delegation";
const static char *OWNER_LABEL = "owner";

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static inline const canton_node_seed_t *get_node_seed_from_node_id(
    int32_t node_id) {
  const canton_node_seed_t *node_seeds =
      canton_txn_context->unsigned_txn.txn_node_seeds;

  for (size_t i = 0;
       i < canton_txn_context->unsigned_txn.txn_meta.node_seeds_count;
       i++) {
    if (node_id == node_seeds[i].node_id) {
      return &node_seeds[i];
    }
  }
  return NULL;
}

static inline const canton_txn_node_hash_t *get_hashed_encoded_node_from_id(
    int32_t node_id) {
  const canton_txn_node_hash_t *node_hashes =
      canton_txn_context->unsigned_txn.txn_node_hashes;

  // TODO: use number of current encoded nodes instead of total nodes
  // count
  // WARN: could lead to reading uninit memory
  for (size_t i = 0; i < canton_txn_context->unsigned_txn.txn_meta.nodes_count;
       i++) {
    if (node_id == node_hashes[i].node_id) {
      return &node_hashes[i];
    }
  }
  return NULL;
}

static inline void encode_empty(uint8_t *buf) {
  buf[0] = 0x00;
}

static inline void encode_bool(bool value, uint8_t *buf) {
  buf[0] = value ? 0x01 : 0x00;
}

static inline void encode_int32(int32_t value, uint8_t *buf) {
  buf[0] = (value >> 24) & 0xFF;
  buf[1] = (value >> 16) & 0xFF;
  buf[2] = (value >> 8) & 0xFF;
  buf[3] = value & 0xFF;
}

static inline void encode_int64(int64_t value, uint8_t *buf) {
  for (int i = 0; i < 8; ++i) {
    buf[7 - i] = (value >> (i * 8)) & 0xFF;
  }
}

static bool encode_string(const char *str, uint8_t *out_buf, size_t *out_len) {
  // TODO:
  //  if (!is_valid_utf8(str)) {
  //    *out_len = 0;
  //    return false;
  //  }

  int32_t len = (int32_t)strlen(str);
  encode_int32(len, out_buf);
  memcpy(out_buf + 4, str, len);
  *out_len = 4 + len;
  return true;
}

static bool encode_bytes(const uint8_t *data,
                         size_t len,
                         uint8_t *out_buf,
                         size_t *out_len) {
  encode_int32((int32_t)len, out_buf);
  memcpy(out_buf + 4, data, len);
  *out_len = 4 + len;
  return true;
}

static bool encode_hex_string(const char *str,
                              uint8_t *out_buf,
                              size_t *out_len) {
  int32_t tmp_out_len = (int32_t)strlen(str) / 2;
  uint8_t result[tmp_out_len];
  hex_string_to_byte_array(str, strlen(str), result);
  encode_bytes(result, tmp_out_len, out_buf, out_len);
  return true;
}

static bool encode_hash(const uint8_t *hash,
                        size_t hash_size,
                        uint8_t *out_buf) {
  memcpy(out_buf, hash, hash_size);
  return true;
}

size_t get_encoded_value_size(const canton_value_t *value) {
  if (NULL == value) {
    return 0;
  }

  switch (value->which_sum) {
    case CANTON_VALUE_UNIT_TAG: {
      return 1;
      break;
    }
    case CANTON_VALUE_BOOLEAN_TAG: {
      return 1 + 1;
      break;
    }
    case CANTON_VALUE_INT64_TAG: {
      return 1 + 8;
      break;
    }
    case CANTON_VALUE_NUMERIC_TAG: {
      return 1 + 4 + strlen(value->numeric);
      break;
    }
    case CANTON_VALUE_TIMESTAMP_TAG: {
      return 1 + 8;
      break;
    }
    case CANTON_VALUE_DATE_TAG: {
      return 1 + 4;
      break;
    }
    case CANTON_VALUE_PARTY_TAG: {
      return 1 + 4 + strlen(value->party);
      break;
    }
    case CANTON_VALUE_TEXT_TAG: {
      return 1 + 4 + strlen(value->text);
      break;
    }
    case CANTON_VALUE_CONTRACT_ID_TAG: {
      return 1 + 4 + (int32_t)(strlen(value->contract_id) / 2);
      break;
    }
    case CANTON_VALUE_OPTIONAL_TAG: {
      return 1 + 1 + get_encoded_value_size(value->optional->value);
      break;
    }
    case CANTON_VALUE_LIST_TAG: {
      size_t total_size = 1 + 4;
      for (size_t i = 0; i < value->list->elements_count; i++) {
        total_size += get_encoded_value_size(&value->list->elements[i]);
      }
      return total_size;
      break;
    }

    case CANTON_VALUE_TEXT_MAP_TAG: {
      size_t total_size = 1 + 4;
      for (size_t i = 0; i < value->text_map->entries_count; i++) {
        total_size += 4 + strlen(value->text_map->entries[i].key);
        total_size += get_encoded_value_size(value->text_map->entries[i].value);
      }
      return total_size;
      break;
    }

    case CANTON_VALUE_RECORD_TAG: {
      size_t total_size = 1;
      // optional record_id
      total_size += 1;
      if (value->record->has_record_id) {
        total_size += get_encoded_identifier_size(&value->record->record_id);
      }

      // repeated records
      total_size += 4;
      for (size_t i = 0; i < value->record->fields_count; i++) {
        // optional label field
        total_size += 1;    // flag
        size_t label_size = strlen(value->record->fields[i].label);
        if (0 < label_size) {
          total_size += 4 + label_size;
        }

        // value
        total_size += get_encoded_value_size(value->record->fields[i].value);
      }

      return total_size;
    }
    case CANTON_VALUE_VARIANT_TAG: {
      size_t total_size = 1;
      // optional
      total_size += 1;    // flag
      if (value->variant->has_variant_id) {
        // ident
        total_size += get_encoded_identifier_size(&value->variant->variant_id);
      }

      // constructor string
      total_size += 4 + strlen(value->variant->constructor);

      // sub value
      total_size += get_encoded_value_size(value->variant->value);

      return total_size;
    }
    case CANTON_VALUE_ENUM_T_TAG: {
      size_t total_size = 1;

      // optional ident
      total_size += 1;    // flag
      if (value->enum_t.has_enum_id) {
        total_size += get_encoded_identifier_size(&value->enum_t.enum_id);
      }

      // constructor
      total_size += 4 + strlen(value->enum_t.constructor);

      return total_size;
    }

    case CANTON_VALUE_GEN_MAP_TAG: {
      size_t total_size = 1;

      // count
      total_size += 4;
      for (size_t i = 0; i < value->gen_map->entries_count; i++) {
        total_size += get_encoded_value_size(value->gen_map->entries[i].key);
        total_size += get_encoded_value_size(value->gen_map->entries[i].value);
      }

      return total_size;
    }
    default:
      return 0;
  }

  return 0;
}

size_t get_encoded_identifier_size(const canton_identifier_t *ident) {
  size_t total_size = 0;
  total_size += 4 + strlen(ident->package_id);

  total_size += 4;    // count
  char *module_copy = strdup(ident->module_name);
  char *part = strtok(module_copy, ".");
  while (part != NULL) {
    total_size += 4 + strlen(part);
    part = strtok(NULL, ".");
  }
  free(module_copy);

  total_size += 4;    // count
  char *entity_copy = strdup(ident->entity_name);
  part = strtok(entity_copy, ".");
  while (part != NULL) {
    total_size += 4 + strlen(part);
    part = strtok(NULL, ".");
  }
  free(entity_copy);
  return total_size;
}

static size_t get_encoded_fetch_node_size(const canton_fetch_t *node,
                                          int32_t node_id) {
  size_t total_buf_size = 0;

  // NODE_ENCODING_VERSION: 1 byte
  total_buf_size += 1;

  // lf_version: 4 + strlen() : string
  total_buf_size += 4 + strlen(node->lf_version);

  // fetch node tag ; 1 byte: 0x02
  total_buf_size += 1;

  // contract_id: 4 + strlen()/2 : hex string
  total_buf_size += 4 + (int)strlen(node->contract_id) / 2;

  // package_name: 4 + strlen() : string
  total_buf_size += 4 + strlen(node->package_name);

  // template_id: get_encoded_identifier_size
  total_buf_size += get_encoded_identifier_size(&node->template_id);

  // signatories_count: 4 bytes : int32
  total_buf_size += 4;

  // signatories: strings
  for (size_t i = 0; i < node->signatories_count; i++) {
    // 4 + strlen() : strings
    total_buf_size += 4 + strlen(node->signatories[i]);
  }

  // stakeholders: 4 bytes : int32
  total_buf_size += 4;

  // stakeholders: strings
  for (size_t i = 0; i < node->stakeholders_count; i++) {
    // 4 + strlen() : strings
    total_buf_size += 4 + strlen(node->stakeholders[i]);
  }

  // interface_id: `optional` flag
  total_buf_size += 1;
  if (node->has_interface_id) {
    // interface_id: identifier
    total_buf_size += get_encoded_identifier_size(&node->interface_id);
  }

  // acting_parties: 4 bytes: int32
  total_buf_size += 4;

  // acting_parties: strings
  for (size_t i = 0; i < node->acting_parties_count; i++) {
    // 4 + strlen() : strings
    total_buf_size += 4 + strlen(node->acting_parties[i]);
  }

  return total_buf_size;
}

static size_t get_encoded_exercise_node_size(const canton_exercise_t *node,
                                             int32_t node_id) {
  size_t total_buf_size = 0;

  // NODE_ENCODING_VERSION: 1 byte
  total_buf_size += 1;

  // lf_version: 4 + strlen() : string
  total_buf_size += 4 + strlen(node->lf_version);

  // create node tag ; 1 byte: 0x01
  total_buf_size += 1;

  // hash of seed
  const canton_node_seed_t *seed = get_node_seed_from_node_id(node_id);
  if (seed == NULL) {
    return -1;
  }
  total_buf_size += seed->seed.size;

  // contract_id: 4 + strlen()/2 : hex string
  total_buf_size += 4 + (int32_t)strlen(node->contract_id) / 2;

  // package_name: 4 + strlen() : string
  total_buf_size += 4 + strlen(node->package_name);

  // template_id: get_encoded_identifier_size
  total_buf_size += get_encoded_identifier_size(&node->template_id);

  // signatories_count: 4 bytes : int32
  total_buf_size += 4;

  // signatories: strings
  for (size_t i = 0; i < node->signatories_count; i++) {
    // 4 + strlen() : strings
    total_buf_size += 4 + strlen(node->signatories[i]);
  }

  // stakeholders: 4 bytes : int32
  total_buf_size += 4;

  // stakeholders: strings
  for (size_t i = 0; i < node->stakeholders_count; i++) {
    // 4 + strlen() : strings
    total_buf_size += 4 + strlen(node->stakeholders[i]);
  }

  // acting_parties: 4 bytes: int32
  total_buf_size += 4;

  // acting_parties: strings
  for (size_t i = 0; i < node->acting_parties_count; i++) {
    // 4 + strlen() : strings
    total_buf_size += 4 + strlen(node->acting_parties[i]);
  }

  // interface_id: `optional` flag
  total_buf_size += 1;
  if (node->has_interface_id) {
    // interface_id: identifier
    total_buf_size += get_encoded_identifier_size(&node->interface_id);
  }

  // 4 + strlen() : strings
  total_buf_size += 4 + strlen(node->choice_id);

  //  chosen_value: Value type
  total_buf_size += get_encoded_value_size(node->chosen_value);

  // consuming: 1 : bool
  total_buf_size += 1;

  // exercise_result : `optional` flag
  total_buf_size += 1;
  //  if (node->has_exercise_result) {
  if (NULL != node->exercise_result) {
    // exercise_result: Value
    total_buf_size += get_encoded_value_size(node->exercise_result);
  }

  // choice_observers_count: int32
  total_buf_size += 4;

  // choice_observers: strings
  for (size_t i = 0; i < node->choice_observers_count; i++) {
    // 4 + strlen() : strings
    total_buf_size += 4 + strlen(node->choice_observers[i]);
  }

  // children_count: int32
  total_buf_size += 4;

  // chidren: sha256 hash of encoded nodes with children ids
  for (size_t i = 0; i < node->children_count; i++) {
    // hash: 32 bytes
    total_buf_size += SHA256_DIGEST_LENGTH;
  }
  return total_buf_size;
}

static size_t get_encoded_create_node_size(const canton_create_t *node,
                                           int32_t node_id) {
  size_t total_buf_size = 0;

  // NODE_ENCODING_VERSION: 1 byte
  total_buf_size += 1;

  // lf_version: 4 + strlen() : string
  total_buf_size += 4 + strlen(node->lf_version);

  // create node tag ; 1 byte: 0x00
  total_buf_size += 1;

  // encode seeds
  total_buf_size += 1;    // `optional` flag
  const canton_node_seed_t *seed = get_node_seed_from_node_id(node_id);
  if (NULL != seed) {
    total_buf_size += seed->seed.size;
  }

  // contract_id: 4 + strlen()/2 : hex string
  total_buf_size += 4 + (int)strlen(node->contract_id) / 2;

  // package_name: 4 + strlen() : string
  total_buf_size += 4 + strlen(node->package_name);

  // template_id: get_encoded_identifier_size
  total_buf_size += get_encoded_identifier_size(&node->template_id);

  // argument: value
  total_buf_size += get_encoded_value_size(node->argument);

  // signatories_count: 4 bytes : int32
  total_buf_size += 4;

  // signatories: strings
  for (size_t i = 0; i < node->signatories_count; i++) {
    // 4 + strlen() : strings
    total_buf_size += 4 + strlen(node->signatories[i]);
  }

  // stakeholders: 4 bytes : int32
  total_buf_size += 4;

  // stakeholders: strings
  for (size_t i = 0; i < node->stakeholders_count; i++) {
    // 4 + strlen() : strings
    total_buf_size += 4 + strlen(node->stakeholders[i]);
  }

  return total_buf_size;
}

static size_t get_encoded_rollback_node_size(const canton_rollback_t *node,
                                             int32_t node_id) {
  size_t total_buf_size = 0;

  // NODE_ENCODING_VERSION: 1 byte
  total_buf_size += 1;

  // create node tag ; 1 byte: 0x03
  total_buf_size += 1;

  // children_count: int32
  total_buf_size += 4;

  // chidren: sha256 hash of encoded nodes with children ids
  for (size_t i = 0; i < node->children_count; i++) {
    // hash: 32 bytes
    total_buf_size += SHA256_DIGEST_LENGTH;
  }
  return total_buf_size;
}

static size_t get_encoded_metadata_size(
    const canton_sign_txn_canton_metadata_t *metadata) {
  size_t total_buf_size = 0;

  // calc: constant : 1 byte
  total_buf_size += 1;

  // calc: submitter_info.act_as_count : 4 bytes : int32
  total_buf_size += 4;

  // calc: submitter_info.act_as : strings
  for (size_t i = 0; i < metadata->submitter_info.act_as_count; i++) {
    // 4 + strlen() : strings
    total_buf_size += 4 + strlen(metadata->submitter_info.act_as[i]);
  }

  // calc: submitter_info.command_id : 4 + strlen() : string
  total_buf_size += 4 + strlen(metadata->submitter_info.command_id);

  // calc: transaction_uuid : 1 + strlen() : string
  total_buf_size += 4 + strlen(metadata->transaction_uuid);

  // calc: mediator_group : 4 : int32
  total_buf_size += 4;

  // calc: synchronizer_id : 4 + strlen() : string
  total_buf_size += 4 + strlen(metadata->synchronizer_id);

  // calc: min_ledger_effective_time
  total_buf_size += 1;    // optional flag
  if (metadata->has_min_ledger_effective_time) {
    // 8 : int64
    total_buf_size += 8;
  }

  // calc: max_ledger_effective_time
  total_buf_size += 1;    // optional flag
  if (metadata->has_max_ledger_effective_time) {
    // 8 : int64
    total_buf_size += 8;
  }

  // calc: preparation_time : 8 : int64
  total_buf_size += 8;

  return total_buf_size;
}

static void encode_value(const canton_value_t *value, uint8_t *buf) {
  if (!value) {
    return NULL;
  }

  size_t out_len = 0;
  switch (value->which_sum) {
    case CANTON_VALUE_UNIT_TAG: {
      buf[0] = 0x00;
      break;
    }
    case CANTON_VALUE_BOOLEAN_TAG: {
      buf[0] = 0x01;
      encode_bool(value->boolean, buf + 1);
      break;
    }
    case CANTON_VALUE_INT64_TAG: {
      buf[0] = 0x02;
      encode_int64(value->int64, buf + 1);
      break;
    }
    case CANTON_VALUE_NUMERIC_TAG: {
      buf[0] = 0x03;
      encode_string(value->numeric, buf + 1, &out_len);
      break;
    }
    case CANTON_VALUE_TIMESTAMP_TAG: {
      buf[0] = 0x04;
      encode_int64(value->timestamp, buf + 1);
      break;
    }
    case CANTON_VALUE_DATE_TAG: {
      buf[0] = 0x05;
      encode_int32(value->date, buf + 1);
      break;
    }
    case CANTON_VALUE_PARTY_TAG: {
      buf[0] = 0x06;
      encode_string(value->party, buf + 1, &out_len);
      break;
    }
    case CANTON_VALUE_TEXT_TAG: {
      buf[0] = 0x07;
      encode_string(value->text, buf + 1, &out_len);
      break;
    }
    case CANTON_VALUE_CONTRACT_ID_TAG: {
      buf[0] = 0x08;
      encode_hex_string(value->contract_id, buf + 1, &out_len);
      break;
    }
    case CANTON_VALUE_OPTIONAL_TAG: {
      buf[0] = 0x09;
      if (NULL != value->optional->value) {
        buf[1] = 0x01;
        encode_value(value->optional->value, buf + 2);
      } else {
        buf[1] = 0x00;
      }
      break;
    }
    case CANTON_VALUE_LIST_TAG: {
      buf[0] = 0x0a;
      size_t offset = 1;
      encode_int32(value->list->elements_count, buf + offset);
      offset += 4;
      for (size_t i = 0; i < value->list->elements_count; i++) {
        canton_value_t *this_value = &value->list->elements[i];
        encode_value(this_value, buf + offset);
        offset += get_encoded_value_size(this_value);
      }
      break;
    }

    case CANTON_VALUE_TEXT_MAP_TAG: {
      buf[0] = 0x0b;
      size_t offset = 1;
      encode_int32(value->text_map->entries_count, buf + offset);
      offset += 4;
      for (size_t i = 0; i < value->text_map->entries_count; i++) {
        canton_text_map_entry_t *entry = &value->text_map->entries[i];

        size_t tmp = 0;
        encode_string(entry->key, buf + offset, &tmp);
        offset += tmp;

        encode_value(entry->value, buf + offset);
        offset += get_encoded_value_size(entry->value);
      }
      break;
    }

    case CANTON_VALUE_RECORD_TAG: {
      buf[0] = 0x0c;
      size_t offset = 1;

      // optional record_id
      *(buf + offset) = 0x00;
      offset += 1;
      if (value->record->has_record_id) {
        *(buf + offset - 1) = 0x01;
        size_t tmp = 0;
        encode_identifier(&value->record->record_id, buf + offset, &tmp);
        offset += tmp;
      }

      // records count
      encode_int32(value->record->fields_count, buf + offset);
      offset += 4;

      // repeated record
      for (size_t i = 0; i < value->record->fields_count; i++) {
        canton_record_field_t *field = &value->record->fields[i];

        // optional label
        *(buf + offset) = 0x00;
        offset += 1;
        if (0 < strlen(field->label)) {
          *(buf + offset - 1) = 0x01;
          size_t tmp = 0;
          encode_string(field->label, buf + offset, &tmp);
          offset += tmp;
        }

        encode_value(field->value, buf + offset);
        offset += get_encoded_value_size(field->value);
      }
      break;
    }
    case CANTON_VALUE_VARIANT_TAG: {
      buf[0] = 0x0d;

      size_t offset = 1;

      *(buf + offset) = 0x00;
      offset += 1;

      size_t tmp = 0;
      if (value->variant->has_variant_id) {
        *(buf + offset - 1) = 0x01;
        encode_identifier(&value->variant->variant_id, buf + offset, &tmp);
        offset += tmp;
      }
      tmp = 0;
      encode_string(value->variant->constructor, buf + offset, &tmp);
      offset += tmp;

      encode_value(value->variant->value, buf + offset);
      offset += get_encoded_value_size(value->variant->value);
      break;
    }
    case CANTON_VALUE_ENUM_T_TAG: {
      buf[0] = 0x0e;
      size_t offset = 1;

      // optional ident
      *(buf + offset) = 0x00;
      offset += 1;

      size_t tmp = 0;
      if (value->enum_t.has_enum_id) {
        *(buf + offset - 1) = 0x01;
        encode_identifier(&value->enum_t.enum_id, buf + offset, &tmp);
        offset += tmp;
      }

      tmp = 0;
      // constructor
      encode_string(value->enum_t.constructor, buf + offset, &tmp);
      break;
    }

    case CANTON_VALUE_GEN_MAP_TAG: {
      buf[0] = 0x0f;
      size_t offset = 1;

      // count
      encode_int32(value->gen_map->entries_count, buf + offset);
      offset += 4;

      for (size_t i = 0; i < value->gen_map->entries_count; i++) {
        canton_gen_map_entry_t *entry = &value->gen_map->entries[i];
        encode_value(entry->key, buf + offset);
        offset += get_encoded_value_size(entry->key);

        encode_value(entry->value, buf + offset);
        offset += get_encoded_value_size(entry->value);
      }
      break;
    }
    default: {
      break;
    }
  }
}

static void encode_identifier(const canton_identifier_t *ident,
                              uint8_t *out_buf,
                              size_t *out_len) {
  // encode package id
  size_t tmp_offset = 0;
  *out_len = 0;
  encode_string(ident->package_id, out_buf, &tmp_offset);
  *out_len += tmp_offset;

  // count number of divided parts
  size_t count = 0;
  char *module_copy = strdup(ident->module_name);
  char *part = strtok(module_copy, ".");
  while (part != NULL) {
    count++;
    part = strtok(NULL, ".");
  }
  free(module_copy);

  // encode count
  encode_int32((int32_t)count, out_buf + tmp_offset);
  tmp_offset += 4;
  *out_len += 4;

  // encode individual strings
  module_copy = strdup(ident->module_name);
  part = strtok(module_copy, ".");
  while (part != NULL) {
    size_t loop_offset = 0;
    encode_string(part, out_buf + tmp_offset, &loop_offset);
    tmp_offset += loop_offset;
    *out_len += loop_offset;
    part = strtok(NULL, ".");
  }
  free(module_copy);

  // count number of divided parts
  count = 0;
  char *entity_copy = strdup(ident->entity_name);
  part = strtok(entity_copy, ".");
  while (part != NULL) {
    count++;
    part = strtok(NULL, ".");
  }
  free(entity_copy);

  // encode count
  encode_int32((int32_t)count, out_buf + tmp_offset);
  tmp_offset += 4;
  *out_len += 4;

  entity_copy = strdup(ident->entity_name);
  part = strtok(entity_copy, ".");
  while (part != NULL) {
    size_t loop_offset = 0;
    encode_string(part, out_buf + tmp_offset, &loop_offset);
    tmp_offset += loop_offset;
    *out_len += loop_offset;
    part = strtok(NULL, ".");
  }

  free(entity_copy);
}

static uint8_t *encode_fetch_node(const canton_fetch_t *node,
                                  int32_t node_id,
                                  size_t *out_len) {
  *out_len = get_encoded_fetch_node_size(node, node_id);
  uint8_t *buf = (uint8_t *)malloc(*out_len);
  size_t offset = 0;
  size_t tmp_len = 0;

  // NODE_ENCODING_VERSION: 1 byte
  *(buf + offset) = NODE_ENCODING_VERSION;
  offset += 1;

  // lf_version: 4 + strlen() : string
  encode_string(node->lf_version, buf + offset, &tmp_len);
  offset += tmp_len;

  // fetch node tag ; 1 byte: 0x02
  *(buf + offset) = FETCH_NODE_TAG;
  offset += 1;

  // contract_id: 4 + strlen()/2 : hex string
  encode_hex_string(node->contract_id, (buf + offset), &tmp_len);
  offset += tmp_len;

  // package_name: 4 + strlen() : string
  encode_string(node->package_name, (buf + offset), &tmp_len);
  offset += tmp_len;

  // template_id: identifier
  encode_identifier(&node->template_id, (buf + offset), &tmp_len);
  offset += tmp_len;

  // signatories_count: 4 bytes : int32
  encode_int32(node->signatories_count, (buf + offset));
  offset += 4;

  // signatories: strings
  for (size_t i = 0; i < node->signatories_count; i++) {
    encode_string(node->signatories[i], (buf + offset), &tmp_len);
    offset += tmp_len;
  }

  // stakeholders_count: 4 bytes : int32
  encode_int32(node->stakeholders_count, (buf + offset));
  offset += 4;

  // stakeholders: strings
  for (size_t i = 0; i < node->stakeholders_count; i++) {
    encode_string(node->stakeholders[i], (buf + offset), &tmp_len);
    offset += tmp_len;
  }

  // interface_id: `optional` flag
  *(buf + offset) = 0x00;
  offset += 1;
  if (node->has_interface_id) {
    *(buf + offset - 1) = 0x01;
    encode_identifier(&node->interface_id, (buf + offset), &tmp_len);
    offset += tmp_len;
  }

  // acting_parties_count: 4 bytes : int32
  encode_int32(node->acting_parties_count, (buf + offset));
  offset += 4;

  // acting_parties : strings
  for (size_t i = 0; i < node->acting_parties_count; i++) {
    encode_string(node->acting_parties[i], (buf + offset), &tmp_len);
    offset += tmp_len;
  }

  return buf;
}

static uint8_t *encode_exercise_node(const canton_exercise_t *node,
                                     int32_t node_id,
                                     size_t *out_len) {
  *out_len = get_encoded_exercise_node_size(node, node_id);
  uint8_t *buf = (uint8_t *)malloc(*out_len);
  size_t offset = 0;
  size_t tmp_len = 0;

  // NODE_ENCODING_VERSION: 1 byte
  *(buf + offset) = NODE_ENCODING_VERSION;
  offset += 1;

  // lf_version: 4 + strlen() : string
  encode_string(node->lf_version, buf + offset, &tmp_len);
  offset += tmp_len;

  // exercise node tag ; 1 byte: 0x01
  *(buf + offset) = EXERCISE_NODE_TAG;
  offset += 1;

  // hash of seed with this node id
  const canton_node_seed_t *seed = get_node_seed_from_node_id(node_id);
  if (NULL == seed) {
    free(buf);
    return NULL;
  }
  encode_hash(seed->seed.bytes, seed->seed.size, buf + offset);
  offset += seed->seed.size;

  // contract_id: 4 + strlen()/2 : hex string
  encode_hex_string(node->contract_id, (buf + offset), &tmp_len);
  offset += tmp_len;

  // package_name: 4 + strlen() : string
  encode_string(node->package_name, (buf + offset), &tmp_len);
  offset += tmp_len;

  // encode_identifier
  encode_identifier(&node->template_id, (buf + offset), &tmp_len);
  offset += tmp_len;

  // signatories_count: 4 bytes : int32
  encode_int32(node->signatories_count, (buf + offset));
  offset += 4;

  // signatories: strings
  for (size_t i = 0; i < node->signatories_count; i++) {
    encode_string(node->signatories[i], (buf + offset), &tmp_len);
    offset += tmp_len;
  }

  // stakeholders_count: 4 bytes : int32
  encode_int32(node->stakeholders_count, (buf + offset));
  offset += 4;

  // stakeholders: strings
  for (size_t i = 0; i < node->stakeholders_count; i++) {
    encode_string(node->stakeholders[i], (buf + offset), &tmp_len);
    offset += tmp_len;
  }

  // acting_parties_count: 4 bytes : int32
  encode_int32(node->acting_parties_count, (buf + offset));
  offset += 4;

  // acting_parties : strings
  for (size_t i = 0; i < node->acting_parties_count; i++) {
    encode_string(node->acting_parties[i], (buf + offset), &tmp_len);
    offset += tmp_len;
  }

  // interface_id: `optional` flag
  *(buf + offset) = 0x00;
  offset += 1;
  if (node->has_interface_id) {
    *(buf + offset - 1) = 0x01;
    encode_identifier(&node->interface_id, (buf + offset), &tmp_len);
    offset += tmp_len;
  }

  // choice_id: 4 + strlen() : strings
  encode_string(node->choice_id, (buf + offset), &tmp_len);
  offset += tmp_len;

  // chosen_value: Value type
  //
  // we would want to extract amount, receiver address right before encoding
  // them. They are stored in exersie node's chosen_value field.

  parse_display_info(node->choice_id, node->chosen_value);

  tmp_len = get_encoded_value_size(node->chosen_value);
  encode_value(node->chosen_value, (buf + offset));
  offset += tmp_len;

  // consuming: 1 : bool
  encode_bool(node->consuming, buf + offset);
  offset += 1;

  // exercise_result : `optional` flag
  *(buf + offset) = 0x00;
  offset += 1;
  if (NULL != node->exercise_result) {
    *(buf + offset - 1) = 0x01;
    // exercise_result: Value
    tmp_len = get_encoded_value_size(node->exercise_result);
    encode_value(node->exercise_result, (buf + offset));
    offset += tmp_len;
  }

  // choice_observers_count: int32
  encode_int32(node->choice_observers_count, buf + offset);
  offset += 4;

  // choice_observers: strings
  for (size_t i = 0; i < node->choice_observers_count; i++) {
    // 4 + strlen() : strings
    encode_string(node->choice_observers[i], buf + offset, &tmp_len);
    offset += tmp_len;
  }

  // children_count: int32
  encode_int32(node->children_count, buf + offset);
  offset += 4;

  // chidren: sha256 hash of encoded nodes with children ids
  for (size_t i = 0; i < node->children_count; i++) {
    // hash: 32 bytes
    int32_t node_id = strtol(node->children[i], NULL, 10);
    const canton_txn_node_hash_t *node =
        get_hashed_encoded_node_from_id(node_id);

    if (NULL == node) {
      free(buf);
      return NULL;
    }
    encode_hash(node->hash, SHA256_DIGEST_LENGTH, buf + offset);
    offset += SHA256_DIGEST_LENGTH;
  }
  return buf;
}

static uint8_t *encode_create_node(const canton_create_t *node,
                                   int32_t node_id,
                                   size_t *out_len) {
  *out_len = get_encoded_create_node_size(node, node_id);
  uint8_t *buf = (uint8_t *)malloc(*out_len);
  if (!buf) {
    return NULL;
  }
  size_t offset = 0;
  size_t tmp_len = 0;

  // NODE_ENCODING_VERSION: 1 byte
  *(buf + offset) = NODE_ENCODING_VERSION;
  offset += 1;

  // lf_version: 4 + strlen() : string
  encode_string(node->lf_version, buf + offset, &tmp_len);
  offset += tmp_len;

  // create node tag ; 1 byte
  *(buf + offset) = CREATE_NODE_TAG;
  offset += 1;

  // node seed: optional
  *(buf + offset) = 0x00;
  offset += 1;
  const canton_node_seed_t *seed = get_node_seed_from_node_id(node_id);
  if (NULL != seed) {
    *(buf + offset - 1) = 0x01;
    encode_hash(seed->seed.bytes, seed->seed.size, (buf + offset));
    offset += seed->seed.size;
  }

  // contract_id: 4 + strlen()/2 : hex string
  encode_hex_string(node->contract_id, (buf + offset), &tmp_len);
  offset += tmp_len;

  // package_name: 4 + strlen() : string
  encode_string(node->package_name, (buf + offset), &tmp_len);
  offset += tmp_len;

  if (node->has_template_id) {
    // encode_identifier
    encode_identifier(&node->template_id, (buf + offset), &tmp_len);
    offset += tmp_len;

    parse_display_info(node->template_id.entity_name, node->argument);
  }

  // argument: value
  encode_value(node->argument, buf + offset);
  offset += get_encoded_value_size(node->argument);

  // signatories_count: 4 bytes : int32
  encode_int32(node->signatories_count, (buf + offset));
  offset += 4;

  // signatories: strings
  for (size_t i = 0; i < node->signatories_count; i++) {
    encode_string(node->signatories[i], (buf + offset), &tmp_len);
    offset += tmp_len;
  }

  // stakeholders_count: 4 bytes : int32
  encode_int32(node->stakeholders_count, (buf + offset));
  offset += 4;

  // stakeholders: strings
  for (size_t i = 0; i < node->stakeholders_count; i++) {
    encode_string(node->stakeholders[i], (buf + offset), &tmp_len);
    offset += tmp_len;
  }

  return buf;
}

static uint8_t *encode_rollback_node(const canton_rollback_t *node,
                                     int32_t node_id,
                                     size_t *out_len) {
  *out_len = get_encoded_rollback_node_size(node, node_id);

  uint8_t *buf = (uint8_t *)malloc(*out_len);
  size_t offset = 0;

  // NODE_ENCODING_VERSION: 1 byte
  *(buf + offset) = NODE_ENCODING_VERSION;
  offset += 1;

  // rollback node tag ; 1 byte: 0x03
  *(buf + offset) = ROLLBACK_NODE_TAG;
  offset += 1;

  // children_count: int32
  encode_int32(node->children_count, buf + offset);
  offset += 4;

  // chidren: sha256 hash of encoded nodes with children ids
  for (size_t i = 0; i < node->children_count; i++) {
    // hash: 32 bytes
    int32_t node_id = strtol(node->children[i], NULL, 10);
    const canton_txn_node_hash_t *node =
        get_hashed_encoded_node_from_id(node_id);

    if (NULL == node) {
      free(buf);
      return NULL;
    }

    encode_hash(node->hash, SHA256_DIGEST_LENGTH, buf + offset);
    offset += SHA256_DIGEST_LENGTH;
  }

  return buf;
}

static uint8_t *encode_node_v1(const canton_node_t *node,
                               char *node_id,
                               size_t *out_len) {
  int32_t node_id_l = strtol(node_id, NULL, 10);
  *out_len = 0;
  switch (node->which_node_type) {
    case CANTON_NODE_CREATE_TAG: {
      return encode_create_node(&node->create, node_id_l, out_len);
    }
    case CANTON_NODE_FETCH_TAG: {
      return encode_fetch_node(&node->fetch, node_id_l, out_len);
    }
    case CANTON_NODE_EXERCISE_TAG: {
      return encode_exercise_node(&node->exercise, node_id_l, out_len);
    }
    case CANTON_NODE_ROLLBACK_TAG: {
      return encode_rollback_node(&node->rollback, node_id_l, out_len);
    }
    default:
      return NULL;
  }
  return NULL;
}

static uint8_t *encode_node(const canton_daml_transaction_d_node_t *node,
                            size_t *out_len) {
  *out_len = 0;
  switch (node->which_versioned_node) {
    case CANTON_DAML_TRANSACTION_D_NODE_V1_TAG: {
      return encode_node_v1(&node->v1, (char *)node->node_id, out_len);
    }
    default: {
      return NULL;
    }
  }
  return NULL;
}

static uint8_t *encode_metadata(
    const canton_sign_txn_canton_metadata_t *metadata,
    size_t *out_len) {
  *out_len = get_encoded_metadata_size(metadata);
  uint8_t *buf = (uint8_t *)malloc(*out_len);
  size_t offset = 0;
  size_t tmp_len = 0;

  *(buf + offset) = 0x01;
  offset += 1;

  encode_int32(metadata->submitter_info.act_as_count, buf + offset);
  offset += 4;

  for (size_t i = 0; i < metadata->submitter_info.act_as_count; i++) {
    encode_string(metadata->submitter_info.act_as[i], buf + offset, &tmp_len);
    offset += tmp_len;
  }

  // command id
  encode_string(metadata->submitter_info.command_id, buf + offset, &tmp_len);
  offset += tmp_len;

  // transaction uuid
  encode_string(metadata->transaction_uuid, buf + offset, &tmp_len);
  offset += tmp_len;

  encode_int32(metadata->mediator_group, buf + offset);
  offset += 4;

  // sync id
  encode_string(metadata->synchronizer_id, buf + offset, &tmp_len);
  offset += tmp_len;

  *(buf + offset) = 0x00;
  offset += 1;
  if (metadata->has_min_ledger_effective_time) {
    *(buf + offset - 1) = 0x01;
    encode_int64(metadata->min_ledger_effective_time, buf + offset);
    offset += 8;
  }

  *(buf + offset) = 0x00;
  offset += 1;
  if (metadata->has_max_ledger_effective_time) {
    *(buf + offset - 1) = 0x01;
    encode_int64(metadata->max_ledger_effective_time, buf + offset);
    offset += 8;
  }

  encode_int64(metadata->preparation_time, buf + offset);
  return buf;
}

static void parse_display_info_from_transfer_record(
    const canton_record_t *record,
    canton_txn_display_info_t *display_info) {
  if (!record || !display_info) {
    return;
  }
  for (size_t i = 0; i < record->fields_count; i++) {
    canton_record_field_t *transfer_field = &record->fields[i];
    canton_value_t *transfer_value = transfer_field->value;

    if (!transfer_value) {
      continue;
    }

    if (strcmp(transfer_field->label, TRANSFER_LABEL) == 0) {
      if (CANTON_VALUE_RECORD_TAG != transfer_value->which_sum) {
        continue;
      }

      display_info->start_time = display_info->expiry_time = 0;

      canton_record_t *transfer_record = transfer_value->record;
      for (size_t j = 0; j < transfer_record->fields_count; j++) {
        canton_record_field_t *display_field = &transfer_record->fields[j];
        canton_value_t *display_value = display_field->value;

        if (!display_value) {
          continue;
        }

        if (strcmp(display_field->label, SENDER_LABEL) == 0) {
          if (CANTON_VALUE_PARTY_TAG != display_value->which_sum) {
            continue;
          }
          strcpy(display_info->sender_party_id, display_value->party);

        } else if (strcmp(display_field->label, RECEIVER_LABEL) == 0) {
          if (CANTON_VALUE_PARTY_TAG != display_value->which_sum) {
            continue;
          }
          strcpy(display_info->receiver_party_id, display_value->party);

        } else if (strcmp(display_field->label, AMOUNT_LABEL) == 0) {
          if (CANTON_VALUE_NUMERIC_TAG != display_value->which_sum) {
            continue;
          }
          strcpy(display_info->amount, display_value->numeric);
        } else if (strcmp(display_field->label, START_TIME_LABEL) == 0) {
          if (CANTON_VALUE_TIMESTAMP_TAG != display_value->which_sum) {
            continue;
          }
          display_info->start_time = display_value->timestamp;
        } else if (strcmp(display_field->label, EXPIRY_TIME_LABEL) == 0) {
          if (CANTON_VALUE_TIMESTAMP_TAG != display_value->which_sum) {
            continue;
          }
          display_info->expiry_time = display_value->timestamp;
        } else if (strcmp(display_field->label, INSTRUMENT_ID_LABEL) == 0) {
          if (CANTON_VALUE_RECORD_TAG != display_value->which_sum) {
            continue;
          }
          canton_record_t *instrument_record = display_value->record;
          for (size_t k = 0; k < instrument_record->fields_count; k++) {
            canton_record_field_t *instrument_field =
                &instrument_record->fields[k];
            canton_value_t *instrument_value = instrument_field->value;
            if (!instrument_value) {
              continue;
            }

            if (strcmp(instrument_field->label, ID_LABEL) == 0) {
              if (CANTON_VALUE_TEXT_TAG != instrument_value->which_sum) {
                continue;
              }
              strcpy(display_info->instrument.id, instrument_value->text);
            } else if (strcmp(instrument_field->label, ADMIN_LABEL) == 0) {
              if (CANTON_VALUE_PARTY_TAG != instrument_value->which_sum) {
                continue;
              }
              strcpy(display_info->instrument.admin, instrument_value->party);
            }
          }
        }

        // TODO: Parse other fields as well like meta->memo
      }
    }
  }
}

static void parse_display_info(const char *choice_id,
                               const canton_value_t *chosen_value) {
  if (!choice_id || !chosen_value) {
    return;
  }

  if (CANTON_VALUE_RECORD_TAG != chosen_value->which_sum) {
    return;
  }
  canton_txn_display_info_t *display_info =
      &canton_txn_context->unsigned_txn.txn_display_info;

  canton_record_t *record = chosen_value->record;
  // for send, compare choice_id with TransferFactory_Transfer
  if (strcmp(choice_id, TRANSFER_CHOICE_ID) == 0) {
    display_info->txn_type = CANTON_TXN_TYPE_TRANSFER;
    parse_display_info_from_transfer_record(record, display_info);
  } else if (strcmp(choice_id, TAP_CHOICE_ID) == 0) {
    display_info->txn_type = CANTON_TXN_TYPE_TAP;

    for (size_t i = 0; i < record->fields_count; i++) {
      canton_record_field_t *tap_field = &record->fields[i];
      canton_value_t *tap_value = tap_field->value;

      if (!tap_value) {
        continue;
      }

      if (strcmp(tap_field->label, RECEIVER_LABEL) == 0) {
        if (CANTON_VALUE_PARTY_TAG != tap_value->which_sum) {
          continue;
        }
        strcpy(display_info->receiver_party_id, tap_value->party);

      } else if (0 == strcmp(tap_field->label, AMOUNT_LABEL)) {
        if (CANTON_VALUE_NUMERIC_TAG != tap_value->which_sum) {
          continue;
        }
        strcpy(display_info->amount, tap_value->numeric);
      }
    }
  } else if (strcmp(choice_id, CANTON_TRANSFER_INSTRUCTION) == 0 ||
             strcmp(choice_id, CANTON_TRANSFER_OFFER) == 0) {
    // txn is either accept, reject or withdraw
    if (!record->has_record_id ||
        (strcmp(record->record_id.entity_name, CANTON_TRANSFER_INSTRUCTION) !=
             0 &&
         strcmp(record->record_id.entity_name, CANTON_TRANSFER_OFFER) != 0)) {
      return;
    }
    parse_display_info_from_transfer_record(record, display_info);
  } else if (strcmp(choice_id, WITHDRAW_CHOICE_ID) == 0) {
    display_info->txn_type = CANTON_TXN_TYPE_WITHDRAW;
  } else if (strcmp(choice_id, ACCEPT_CHOICE_ID) == 0) {
    display_info->txn_type = CANTON_TXN_TYPE_ACCEPT;
  } else if (strcmp(choice_id, REJECT_CHOICE_ID) == 0) {
    display_info->txn_type = CANTON_TXN_TYPE_REJECT;
  } else if (strcmp(choice_id, PREAPPROVAL_CHOICE_ID) == 0) {
    if (!record->has_record_id ||
        strcmp(record->record_id.entity_name, PREAPPROVAL_CHOICE_ID) != 0) {
      return;
    }
    display_info->txn_type = CANTON_TXN_TYPE_PREAPPROVAL;

    for (size_t i = 0; i < record->fields_count; i++) {
      canton_record_field_t *preapproval_field = &record->fields[i];
      canton_value_t *preapproval_value = preapproval_field->value;

      if (!preapproval_value) {
        continue;
      }

      if (strcmp(preapproval_field->label, RECEIVER_LABEL) == 0) {
        if (CANTON_VALUE_PARTY_TAG != preapproval_value->which_sum) {
          continue;
        }
        strcpy(display_info->receiver_party_id, preapproval_value->party);
      }

      // TODO: parse other fields if required. ex 'provider'
    }
  } else if (strcmp(choice_id, MERGE_DELEGATION_PROPOSAL_CHOICE_ID) == 0) {
    if (!record->has_record_id ||
        strcmp(record->record_id.entity_name,
               MERGE_DELEGATION_PROPOSAL_CHOICE_ID) != 0) {
      return;
    }
    display_info->txn_type = CANTON_TXN_TYPE_MERGE_DELEGATION_PROPOSAL;

    for (size_t i = 0; i < record->fields_count; i++) {
      canton_record_field_t *merge_delegation_proposal_field =
          &record->fields[i];
      canton_value_t *merge_delegation_proposal_value =
          merge_delegation_proposal_field->value;

      if (!merge_delegation_proposal_value) {
        continue;
      }

      if (strcmp(merge_delegation_proposal_field->label, DELEGATION_LABEL) ==
          0) {
        if (CANTON_VALUE_RECORD_TAG !=
            merge_delegation_proposal_value->which_sum) {
          continue;
        }

        canton_record_t *merge_delegation_proposal_record =
            merge_delegation_proposal_value->record;
        if (!merge_delegation_proposal_record->has_record_id ||
            strcmp(merge_delegation_proposal_record->record_id.entity_name,
                   MERGE_DELEGATION_CHOICE_ID) != 0) {
          continue;
        }

        for (size_t j = 0; j < merge_delegation_proposal_record->fields_count;
             j++) {
          canton_record_field_t *merge_delegation_field =
              &merge_delegation_proposal_record->fields[j];
          canton_value_t *merge_delegation_value =
              merge_delegation_field->value;

          if (!merge_delegation_value) {
            continue;
          }

          if (strcmp(merge_delegation_field->label, OWNER_LABEL) == 0) {
            if (CANTON_VALUE_PARTY_TAG != merge_delegation_value->which_sum) {
              continue;
            }
            strcpy(display_info->owner_party_id, merge_delegation_value->party);
          }
        }
      }

      // TODO: parse other fields if required. ex 'operator'
    }
  }
}

static int compare_hashes(const void *a, const void *b) {
  const canton_party_txn *party_txn_a = (const canton_party_txn *)a;
  const canton_party_txn *party_txn_b = (const canton_party_txn *)b;
  return memcmp(party_txn_a->hash, party_txn_b->hash, CANTON_HASH_SIZE);
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

uint8_t *encode_canton_txn_node(const canton_daml_transaction_d_node_t *d_node,
                                size_t *out_len) {
  if (!d_node || !out_len) {
    return NULL;
  }
  return encode_node(d_node, out_len);
}

uint8_t *encode_canton_metadata_input_contract(
    const canton_metadata_input_contract_t *input_contract,
    size_t *const out_len) {
  if (!input_contract || !out_len) {
    return NULL;
  }

  /*
   * encode_input_contract:
   *    creation time + sha256(encode_create_node(input_contract))
   */

  *out_len = 8 + SHA256_DIGEST_LENGTH;

  uint8_t *final_buf = (uint8_t *)malloc(*out_len);

  if (!final_buf) {
    return NULL;
  }

  encode_int64(input_contract->created_at, final_buf);

  size_t tmp_out_len = 0;
  uint8_t *buf = encode_create_node(&input_contract->v1, -10, &tmp_out_len);
  if (NULL == buf) {
    return NULL;
  }

  uint8_t create_node_digest[SHA256_DIGEST_LENGTH];
  sha256_Raw(buf, tmp_out_len, create_node_digest);

  memcpy(final_buf + 8, create_node_digest, SHA256_DIGEST_LENGTH);

  free(buf);

  return final_buf;
}

bool parse_and_hash_canton_metadata_input_contract(
    const uint8_t *txn_serialized_input_contract,
    uint32_t txn_input_contract_size,
    canton_txn_input_contract_hash_t *input_contract_hash) {
  if (!txn_serialized_input_contract || !txn_input_contract_size ||
      !input_contract_hash) {
    return false;
  }

  // decode protobuf serialized input contract
  canton_metadata_input_contract_t decoded_input_contract =
      CANTON_METADATA_INPUT_CONTRACT_INIT_ZERO;
  if (!decode_canton_serialized_data(txn_serialized_input_contract,
                                     txn_input_contract_size,
                                     CANTON_METADATA_INPUT_CONTRACT_FIELDS,
                                     &decoded_input_contract)) {
    return false;
  }

  // validate and encode input contract
  size_t input_contract_out_len = 0;
  uint8_t *encoded_input_contract = encode_canton_metadata_input_contract(
      &decoded_input_contract, &input_contract_out_len);
  if (!encoded_input_contract) {
    return false;
  }

  /* copy encoded input contract */
  memcpy(input_contract_hash->hash,
         encoded_input_contract,
         input_contract_out_len);

  // release input contract data
  pb_release(CANTON_METADATA_INPUT_CONTRACT_FIELDS, &decoded_input_contract);

  // free encoded input contract
  free(encoded_input_contract);

  return true;
}

bool parse_and_hash_canton_txn_node(const uint8_t *txn_serialized_node,
                                    uint32_t txn_node_size,
                                    canton_txn_node_hash_t *node_hash) {
  if (!txn_serialized_node || !txn_node_size || !node_hash) {
    return false;
  }

  // decode protobuf serialized node
  canton_daml_transaction_d_node_t decoded_d_node =
      CANTON_DAML_TRANSACTION_D_NODE_INIT_ZERO;
  if (!decode_canton_serialized_data(txn_serialized_node,
                                     txn_node_size,
                                     CANTON_DAML_TRANSACTION_D_NODE_FIELDS,
                                     &decoded_d_node)) {
    return false;
  }

  // validate and encode transaction node
  size_t node_out_len = 0;
  uint8_t *encoded_node =
      encode_canton_txn_node(&decoded_d_node, &node_out_len);

  if (!encoded_node) {
    return false;
  }

  // hash encoded node
  sha256_Raw(encoded_node, node_out_len, node_hash->hash);

  // copy node id
  node_hash->node_id = strtol(decoded_d_node.node_id, NULL, 10);

  // release node data
  pb_release(CANTON_DAML_TRANSACTION_D_NODE_FIELDS, &decoded_d_node);

  // free encoded node
  free(encoded_node);
  return true;
}

bool validate_and_encode_canton_unsigned_txn() {
  if (!canton_txn_context) {
    return false;
  }

  /* Initialize transaction hash context */
  SHA256_CTX transaction_hash_ctx = {0};
  sha256_Init(&transaction_hash_ctx);

  /* update prepared transaction hash purpose to transaction hash context */
  sha256_Update(&transaction_hash_ctx,
                (const uint8_t *)&PREPARED_TRANSACTION_HASH_PURPOSE,
                sizeof(PREPARED_TRANSACTION_HASH_PURPOSE));

  /* update transaction_hash with transaction version encoded as string */
  uint8_t version_out_buf[10];
  size_t out_len = 0;
  encode_string(canton_txn_context->unsigned_txn.txn_meta.version,
                version_out_buf,
                &out_len);
  sha256_Update(&transaction_hash_ctx, version_out_buf, out_len);

  /* update transaction_hash with: */
  /* root nodes count */
  uint8_t root_node_count_buf[4];
  encode_int32(canton_txn_context->unsigned_txn.txn_meta.roots_count,
               root_node_count_buf);
  sha256_Update(
      &transaction_hash_ctx, root_node_count_buf, sizeof(root_node_count_buf));

  /* root nodes hashes */
  for (size_t i = 0; i < canton_txn_context->unsigned_txn.txn_meta.roots_count;
       i++) {
    char *root_node_id = canton_txn_context->unsigned_txn.txn_meta.roots[i];
    int32_t root_node_id_l = strtol(root_node_id, NULL, 10);
    sha256_Update(&transaction_hash_ctx,
                  get_hashed_encoded_node_from_id(root_node_id_l)->hash,
                  SHA256_DIGEST_LENGTH);
  }

  /* finalize transaction hash digest */
  uint8_t transaction_hash_digest[SHA256_DIGEST_LENGTH];
  sha256_Final(&transaction_hash_ctx, transaction_hash_digest);

  /* create metadata_hash_ctx */
  SHA256_CTX metadata_hash_ctx = {0};
  sha256_Init(&metadata_hash_ctx);

  /* update metadata hash with PREPARED_TRANSACTION_HASH_PURPOSE */
  sha256_Update(&metadata_hash_ctx,
                (const uint8_t *)&PREPARED_TRANSACTION_HASH_PURPOSE,
                sizeof(PREPARED_TRANSACTION_HASH_PURPOSE));

  /* encoded canton metadata and update metadata hash with it */
  size_t encoded_metadata_size = 0;
  uint8_t *encoded_metadata = encode_metadata(
      &canton_txn_context->unsigned_txn.canton_meta, &encoded_metadata_size);

  sha256_Update(&metadata_hash_ctx, encoded_metadata, encoded_metadata_size);
  free(encoded_metadata);

  /* input contract length */
  uint8_t input_contract_count_buf[4];
  encode_int32(
      canton_txn_context->unsigned_txn.canton_meta.input_contracts_count,
      input_contract_count_buf);
  sha256_Update(&metadata_hash_ctx,
                input_contract_count_buf,
                sizeof(input_contract_count_buf));

  /* update metadata hash with input contract hashes */
  for (size_t i = 0;
       i < canton_txn_context->unsigned_txn.canton_meta.input_contracts_count;
       i++) {
    sha256_Update(
        &metadata_hash_ctx,
        canton_txn_context->unsigned_txn.input_contract_hashes[i].hash,
        CANTON_INPUT_CONTRACT_HASH_SIZE);
  }

  /* finalize metadata hash */
  uint8_t metadata_hash_digest[SHA256_DIGEST_LENGTH];
  sha256_Final(&metadata_hash_ctx, metadata_hash_digest);

  /* final encoded txn hash */
  size_t offset = 0;

  /* prepared transastion hash purpose */
  memcpy(canton_txn_context->encoded_txn + offset,
         &PREPARED_TRANSACTION_HASH_PURPOSE,
         sizeof(PREPARED_TRANSACTION_HASH_PURPOSE));
  offset += sizeof(PREPARED_TRANSACTION_HASH_PURPOSE);

  /* hashing scheme version */
  memcpy(canton_txn_context->encoded_txn + offset,
         &HASHING_SCHEME_VERSION_V2,
         sizeof(HASHING_SCHEME_VERSION_V2));
  offset += sizeof(HASHING_SCHEME_VERSION_V2);

  /* transaction hash */
  memcpy(canton_txn_context->encoded_txn + offset,
         transaction_hash_digest,
         SHA256_DIGEST_LENGTH);
  offset += SHA256_DIGEST_LENGTH;

  /* metadata hash */
  memcpy(canton_txn_context->encoded_txn + offset,
         metadata_hash_digest,
         SHA256_DIGEST_LENGTH);

  return true;
}

void sha256_canton(int32_t purpose,
                   const uint8_t *data,
                   size_t data_size,
                   uint8_t *hash) {
  uint8_t buf[4 + data_size];
  memzero(buf, sizeof(buf));

  encode_int32(purpose, buf);

  memcpy(buf + 4, data, data_size);
  sha256_with_prefix(buf, sizeof(buf), hash);
}

void hash_party_txns(canton_party_txn *party_txns,
                     uint32_t party_txns_count,
                     uint8_t *hash) {
  if (!party_txns || !hash || party_txns_count == 0) {
    return;
  }

  // sort the party txns
  qsort(party_txns, party_txns_count, sizeof(canton_party_txn), compare_hashes);

  // combine the hashes of the txns
  // combined_hashes = [encoded_party_txns_count, (encoded_individual_hash_size,
  // encoded_individual_hash), ...]
  size_t encoded_int32_size = 4;
  size_t individual_hash_size = CANTON_HASH_PREFIX_SIZE + SHA256_DIGEST_LENGTH;
  size_t combined_hashes_size =
      encoded_int32_size +
      (party_txns_count * (encoded_int32_size + individual_hash_size));

  uint8_t combined_hashes[combined_hashes_size];
  memzero(combined_hashes, combined_hashes_size);

  size_t offset = 0;
  encode_int32(party_txns_count, combined_hashes + offset);
  offset += encoded_int32_size;

  for (size_t i = 0; i < party_txns_count; i++) {
    encode_int32(individual_hash_size, combined_hashes + offset);
    offset += encoded_int32_size;

    memcpy(combined_hashes + offset, party_txns[i].hash, individual_hash_size);
    offset += individual_hash_size;
  }

  sha256_canton(CANTON_MULTI_TOPOLOGY_TXNS_COMBINED_HASH_PURPOSE,
                combined_hashes,
                sizeof(combined_hashes),
                hash);
}
