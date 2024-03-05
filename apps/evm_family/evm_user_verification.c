/**
 * @file    evm_user_verification.c
 * @author  Cypherock X1 Team
 * @brief   Apis for user verification of all different transaction type info
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

#include "evm_user_verification.h"

#include "address.h"
#include "constant_texts.h"
#include "evm_api.h"
#include "evm_priv.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"

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

bool evm_verify_transfer(const evm_txn_context_t *txn_context) {
  bool status = false;
  char address[43] = "0x";
  const uint8_t *to_address = NULL;
  const char *unit = evm_get_asset_symbol(txn_context);
  char value[34] = {'\0'};
  char hex_str[30] = {'\0'};
  char display[80] = "";

  // TODO: verify transaction nonce; this is pending on settings option
#if 0
  // TODO: convert byte to value
  if (!core_scroll_page("Verify nonce", address, evm_send_error)) {
    return status;
  }
#endif

  // verify recipient address; TODO: handle harmony address encoding
  eth_get_to_address(txn_context, &to_address);
  ethereum_address_checksum(
      to_address, &address[2], false, g_evm_app->chain_id);
  snprintf(
      display, sizeof(display), UI_TEXT_SEND_PROMPT, unit, g_evm_app->name);
  if (!core_scroll_page(NULL, display, evm_send_error) ||
      !core_scroll_page(ui_text_verify_address, address, evm_send_error)) {
    return status;
  }

  // verify recipient amount
  uint8_t len = eth_get_value(txn_context, hex_str);
  if (!convert_byte_array_to_decimal_string(
          len, evm_get_decimal(txn_context), hex_str, value, sizeof(value))) {
    evm_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 1);
    return status;
  }

  snprintf(display, sizeof(display), UI_TEXT_VERIFY_AMOUNT, value, unit);
  if (!core_confirmation(display, evm_send_error)) {
    return status;
  }

  // verify transaction fee
  unit = g_evm_app->lunit_name;
  eth_get_fee_string(
      &txn_context->transaction_info, value, sizeof(value), ETH_DECIMAL);
  snprintf(display, sizeof(display), UI_TEXT_SEND_TXN_FEE, value, unit);
  if (!core_scroll_page(UI_TEXT_TXN_FEE, display, evm_send_error)) {
    return status;
  }

  return true;
}

bool evm_verify_clear_signing(const evm_txn_context_t *txn_context) {
  char address[43] = "0x";
  const uint8_t *to_address = NULL;
  const char *unit = g_evm_app->lunit_name;
  char fee[34] = "";
  char display[40] = "";

  // show warning for not-whitelisted contracts; take user consent
  to_address = txn_context->transaction_info.to_address;
  ethereum_address_checksum(
      to_address, &address[2], false, g_evm_app->chain_id);
  delay_scr_init(ui_text_unverified_contract, DELAY_TIME);

  if (!core_scroll_page(ui_text_verify_contract, address, evm_send_error)) {
    return false;
  }

  // verify transaction fee
  eth_get_fee_string(
      &txn_context->transaction_info, fee, sizeof(fee), ETH_DECIMAL);
  snprintf(display, sizeof(display), UI_TEXT_SEND_TXN_FEE, fee, unit);
  if (!core_scroll_page(UI_TEXT_TXN_FEE, display, evm_send_error)) {
    return false;
  }

  ui_display_node *ui_node = txn_context->display_node;
  while (NULL != ui_node) {
    if (!core_scroll_page(ui_node->title, ui_node->value, evm_send_error)) {
      return false;
    }
    ui_node = ui_node->next;
  }
  /**
   * The function ETH_ExtractArguments prepares the list of display nodes
   * containing presentable strings. The function reserves heap memory for the
   * list elements. This is the earliest safe point to free the memory.
   */
  cy_free();
  return true;
}

bool evm_verify_blind_signing(const evm_txn_context_t *txn_context) {
  bool status = false;
  const uint8_t *to_address = NULL;
  char address[43] = "0x";
  char path_str[64] = "";
  char fee[34] = "";
  char display[40] = "";
  const char *unit = g_evm_app->lunit_name;
  const uint32_t *hd_path = txn_context->init_info.derivation_path;
  size_t depth = txn_context->init_info.derivation_path_count;

  // TODO: decide on handling blind signing via wallet setting
  to_address = txn_context->transaction_info.to_address;
  ethereum_address_checksum(
      to_address, &address[2], false, g_evm_app->chain_id);
  hd_path_array_to_string(hd_path, depth, false, path_str, sizeof(path_str));
  eth_get_fee_string(
      &txn_context->transaction_info, fee, sizeof(fee), ETH_DECIMAL);
  snprintf(display, sizeof(display), UI_TEXT_SEND_TXN_FEE, fee, unit);
  // show warning for unknown EVM function; take user consent
  if (!core_confirmation(UI_TEXT_BLIND_SIGNING_WARNING, evm_send_error) ||
      !core_scroll_page(UI_TEXT_VERIFY_HD_PATH, path_str, evm_send_error) ||
      !core_scroll_page(ui_text_verify_contract, address, evm_send_error) ||
      !core_scroll_page(UI_TEXT_TXN_FEE, display, evm_send_error)) {
    return status;
  }

  return true;
}
