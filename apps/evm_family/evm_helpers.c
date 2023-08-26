/**
 * @file    evm_helpers.c
 * @author  Cypherock X1 Team
 * @brief   Utilities specific to EVM chains
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
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

#include "evm_helpers.h"

#include "coin_utils.h"
#include "eip712_utils.h"
#include "eth.h"
#include "evm_priv.h"
#include "evm_txn_helpers.h"

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
 * @brief Verifies the derivation path for legacy accounts.
 *
 * @param[in] path      The derivation path
 * @param[in] depth     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided path is valid for legacy accounts
 */
static inline bool is_legacy_hd_path(const uint32_t *path, uint32_t depth);

/**
 * @brief Verifies the derivation path for default BIP44 path. Metamask uses
 * this.
 *
 * @param[in] path      The derivation path
 * @param[in] depth     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided path is a default bip44 path
 */
static inline bool is_bip44_hd_path(const uint32_t *path, uint32_t depth);

/**
 * @brief Verifies the derivation path for account model. Ledger live uses this.
 *
 * @param[in] path      The derivation path
 * @param[in] depth     The number of levels in the derivation path
 *
 * @return bool Indicates if the provided path uses account as the key
 */
static inline bool is_account_hd_path(const uint32_t *path, uint32_t depth);

/**
 * @brief This function calculates the digest of ETH sign or Personal sign data
 * using the Keccak-256 algorithm.
 *
 * @param[in] msg_data A pointer to the data that needs to be hashed.
 * @param[in] msg_data_size The size of the message data in bytes.
 * @param[out] digest_out A pointer to a buffer where the computed digest will
 * be stored.
 *
 * @return bool value indicating hash calculation status.
 */
static bool evm_get_personal_data_digest(uint8_t *msg_data,
                                         uint32_t msg_data_size,
                                         uint8_t *digest_out);
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static inline bool is_legacy_hd_path(const uint32_t *path, uint32_t depth) {
  return EVM_DRV_LEGACY_DEPTH == depth && ETHEREUM_PURPOSE_INDEX == path[0] &&
         ETHEREUM_COIN_INDEX == path[1] && EVM_DRV_ACCOUNT == path[2] &&
         is_non_hardened(path[3]);
}

static inline bool is_bip44_hd_path(const uint32_t *path, uint32_t depth) {
  return EVM_DRV_BIP44_DEPTH == depth && ETHEREUM_PURPOSE_INDEX == path[0] &&
         ETHEREUM_COIN_INDEX == path[1] && is_hardened(path[2]) &&
         0 == path[3] && 0 == path[4];
}

static inline bool is_account_hd_path(const uint32_t *path, uint32_t depth) {
  return EVM_DRV_ACCOUNT_DEPTH == depth && ETHEREUM_PURPOSE_INDEX == path[0] &&
         ETHEREUM_COIN_INDEX == path[1] && EVM_DRV_ACCOUNT == path[2] &&
         0 == path[3] && is_non_hardened(path[4]);
}

static bool evm_get_personal_data_digest(uint8_t *msg_data,
                                         uint32_t msg_data_size,
                                         uint8_t *digest_out) {
  if (NULL == msg_data || NULL == digest_out || 0 == msg_data_size) {
    return false;
  }

  char size_string[256] = {0};
  uint8_t size_string_size = 0;
  SHA3_CTX ctx = {0};

  keccak_256_Init(&ctx);

  size_string_size =
      snprintf(size_string, sizeof(size_string), "%ld", msg_data_size);

  keccak_Update(&ctx,
                (const uint8_t *)ETH_PERSONAL_SIGN_IDENTIFIER,
                sizeof(ETH_PERSONAL_SIGN_IDENTIFIER) - 1);
  keccak_Update(&ctx, (const uint8_t *)size_string, size_string_size);
  keccak_Update(&ctx, (const uint8_t *)msg_data, msg_data_size);

  keccak_Final(&ctx, digest_out);
  return true;
}

static bool evm_get_typed_struct_data_digest(
    const evm_sign_typed_data_struct_t *typed_data,
    uint8_t *digest_out) {
  if (NULL == typed_data || NULL == digest_out) {
    return false;
  }

  eip712_status_codes_e eip712_status = EIP712_ERROR;
  bool status = false;
  uint8_t *data = NULL;
  uint16_t data_size = 0, offset = 0;

  data_size = sizeof(ETH_SIGN_TYPED_DATA_IDENTIFIER) - 1 + HASH_SIZE * 2;
  data = malloc(data_size);
  ASSERT(NULL != data);
  memzero(data, data_size);
  memcpy(data,
         ETH_SIGN_TYPED_DATA_IDENTIFIER,
         sizeof(ETH_SIGN_TYPED_DATA_IDENTIFIER) - 1);

  offset += sizeof(ETH_SIGN_TYPED_DATA_IDENTIFIER) - 1;
  eip712_status = hash_struct(&typed_data->domain, data + offset);
  offset += HASH_SIZE;
  eip712_status |= hash_struct(&typed_data->message, data + offset);

  if (EIP712_OK == eip712_status) {
    keccak_256(data, data_size, digest_out);
    status = true;
  }

  memzero(data, data_size);
  free(data);

  return status;
}

static ui_display_node *evm_ui_create_display_node(const char *title,
                                                   const size_t title_size,
                                                   const char *value,
                                                   const size_t value_size) {
  ui_display_node *result = cy_malloc(sizeof(ui_display_node));
  memzero(result, sizeof(ui_display_node));

  size_t title_length = strnlen(title, title_size) + 1;
  result->title = cy_malloc(title_length);
  memzero(result->title, title_length);
  strncpy(result->title, title, title_length - 1);

  size_t value_length = strnlen(value, value_size) + 1;
  result->value = cy_malloc(value_length);
  memzero(result->value, value_length);
  strncpy(result->value, value, value_length - 1);

  result->next = NULL;
  return result;
}

static ui_display_node *evm_create_typed_data_display_nodes(
    evm_sign_typed_data_node_t *root,
    ui_display_node **display_node) {
  queue *q = create_queue();
  enqueue(q, root, "");
  ui_display_node *temp = *display_node;
  while (!is_empty(q)) {
    int node_count = 0;
    node_count = q->count;
    while (node_count > 0) {
      queue_node *node = dequeue(q);
      evm_sign_typed_data_node_t *curr_node = node->tree_node;
      char title[BUFFER_SIZE] = {0};
      snprintf(title, BUFFER_SIZE, "%s%s", node->prefix, curr_node->name);

      char data[BUFFER_SIZE] = {0};
      fill_string_with_data(curr_node, data, sizeof(data));

      temp->next =
          evm_ui_create_display_node(title, BUFFER_SIZE, data, sizeof(data));
      temp = temp->next;

      for (int i = 0; i < curr_node->children_count; i++) {
        char prefix[1024] = {0};
        strcat(prefix, node->prefix);
        strcat(prefix, node->tree_node->name);
        strcat(prefix, ".");
        enqueue(q, curr_node->children + i, prefix);
      }
      // free(node);
      node_count--;
    }
  }

  return temp;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool evm_derivation_path_guard(const uint32_t *path, uint32_t depth) {
  if (is_legacy_hd_path(path, depth)) {
    return true;
  }

  if (is_bip44_hd_path(path, depth)) {
    return true;
  }

  if (is_account_hd_path(path, depth)) {
    return true;
  }

  return false;
}

bool evm_get_msg_data_digest(const evm_sign_msg_context_t *ctx,
                             uint8_t *digest) {
  bool result = false;

  switch (ctx->init.message_type) {
    case EVM_SIGN_MSG_TYPE_ETH_SIGN:
    case EVM_SIGN_MSG_TYPE_PERSONAL_SIGN: {
      result = evm_get_personal_data_digest(
          ctx->msg_data, ctx->init.total_msg_size, digest);
    } break;

    case EVM_SIGN_MSG_TYPE_SIGN_TYPED_DATA: {
      result = evm_get_typed_struct_data_digest(&(ctx->typed_data), digest);
    } break;
    default:
      break;
  }

  return result;
}

void evm_init_typed_data_display_node(
    ui_display_node **node,
    evm_sign_typed_data_struct_t *typed_data) {
  *node = evm_ui_create_display_node(UI_TEXT_VERIFY_DOMAIN,
                                     sizeof(UI_TEXT_VERIFY_DOMAIN),
                                     UI_TEXT_EIP712_DOMAIN_TYPE,
                                     sizeof(UI_TEXT_EIP712_DOMAIN_TYPE));
  ui_display_node *temp = *node;
  temp = evm_create_typed_data_display_nodes(&typed_data->domain, &temp);
  temp->next = evm_ui_create_display_node(UI_TEXT_VERIFY_MESSAGE,
                                          sizeof(UI_TEXT_VERIFY_MESSAGE),
                                          typed_data->message.struct_name,
                                          256);
  temp =
      evm_create_typed_data_display_nodes(&typed_data->message, &(temp->next));

  return;
}