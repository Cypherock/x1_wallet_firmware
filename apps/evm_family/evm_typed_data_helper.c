/**
 * @file    evm_typed_data_helper.c
 * @author  Cypherock X1 Team
 * @brief   Implements the typed data helper functions.
 *          Short description of the file
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
#include "evm_typed_data_helper.h"

#include "coin_utils.h"
#include "eip712_utils.h"
#include "evm_priv.h"

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
 * @brief The function creates display nodes for a tree of typed data, using a
 * breadth-first search approach.
 *
 * @param root A pointer to the root node of the evm_sign_typed_data_node_t tree
 * structure.
 * @param display_node A pointer to a pointer of a `ui_display_node` object.
 * This is used to keep track of the current display node while traversing the
 * tree.
 *
 * @return a pointer to the last created `ui_display_node` in the linked list.
 */
static ui_display_node *evm_create_typed_data_display_nodes(
    evm_sign_typed_data_node_t *root,
    ui_display_node **display_node);
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
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
          ui_create_display_node(title, BUFFER_SIZE, data, sizeof(data));
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
void evm_init_typed_data_display_node(
    ui_display_node **node,
    evm_sign_typed_data_struct_t *typed_data) {
  *node = ui_create_display_node(UI_TEXT_VERIFY_DOMAIN,
                                 sizeof(UI_TEXT_VERIFY_DOMAIN),
                                 UI_TEXT_EIP712_DOMAIN_TYPE,
                                 sizeof(UI_TEXT_EIP712_DOMAIN_TYPE));
  ui_display_node *temp = *node;
  temp = evm_create_typed_data_display_nodes(&typed_data->domain, &temp);
  temp->next = ui_create_display_node(UI_TEXT_VERIFY_MESSAGE,
                                      sizeof(UI_TEXT_VERIFY_MESSAGE),
                                      typed_data->message.struct_name,
                                      256);
  temp =
      evm_create_typed_data_display_nodes(&typed_data->message, &(temp->next));

  return;
}

bool evm_get_typed_struct_data_digest(
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
  eip712_status = hash_struct(&(typed_data->domain), data + offset);
  offset += HASH_SIZE;
  eip712_status |= hash_struct(&(typed_data->message), data + offset);

  if (EIP712_OK == eip712_status) {
    keccak_256(data, data_size, digest_out);
    status = true;
  }

  memzero(data, data_size);
  free(data);

  return status;
}
