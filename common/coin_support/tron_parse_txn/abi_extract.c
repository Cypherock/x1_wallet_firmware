/**
 * @file    abi_extract.c
 * @author  Cypherock X1 Team
 * @brief   Tron chain abi extract logic
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

#include "abi_extract.h"

#include "tron_contracts.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

extern const uint8_t selector[][4];
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

ui_display_node *extract_data(uint8_t *data) {
  uint8_t to_address[TRON_INITIAL_ADDRESS_LENGTH] = {0};
  to_address[0] = 0x41;
  uint8_t amount[32];
  uint8_t function_selector[4] = {0};

  memcpy(function_selector, data, 4);
  // address is initial_address without '0x41' (20 Bytes)
  memcpy(to_address + 1, data + 4 + (32 - 20), 20);
  memcpy(amount, data + 4 + 32, 32);

  // check function selector
  for (int i = 0; i < TRC20_FUNCTION_SELECTOR_COUNT; i++) {
    bool is_valid = 0;
    if (memcmp(function_selector, selector[i], 4) == 0) {
      is_valid = 1;
    }
    if (!is_valid) {
      return NULL;
    }
  }
  char address[TRON_ACCOUNT_ADDRESS_LENGTH + 1] = {0};
  // receipent address
  if (!base58_encode_check(to_address,
                           1 + 20,
                           HASHER_SHA2D,
                           address,
                           TRON_ACCOUNT_ADDRESS_LENGTH + 1)) {
    tron_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return NULL;
  }
  ui_display_node *ui_node = ui_create_display_node(
      "Datatype:address\0", 25, address, sizeof(address));
  ui_display_node *head = ui_node;

  // receipent amount
  char staticBufferInUTF8[200];
  memzero(staticBufferInUTF8, sizeof(staticBufferInUTF8));
  byte_array_to_hex_string(amount, 32, &(staticBufferInUTF8[0]), 65);
  convert_byte_array_to_decimal_string(
      64, 0, &(staticBufferInUTF8[0]), &(staticBufferInUTF8[100]), 100);

  (ui_node)->next = ui_create_display_node(
      "Datatype:uint256\0", 25, &(staticBufferInUTF8[100]), 100);

  ui_node = (ui_node)->next;
  (ui_node)->next = NULL;

  return head;
}
