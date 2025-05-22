/**
 * @file    tron_contracts.c
 * @author  Cypherock X1 Team
 * @brief   Tron chain contract parsing logic
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

#include <pb_decode.h>
#include <tron/contract.pb.h>
#include <tron/google/protobuf/any.pb.h>
#include <tron/sign_txn.pb.h>
#include <tron/tron.pb.h>
#include <tron_txn_helpers.h>

#include "abi_extract.h"
#include "base58.h"
#include "coin_utils.h"
#include "curves.h"
#include "ecdsa.h"
#include "exchange_main.h"
#include "hasher.h"
#include "secp256k1.h"
#include "sha2.h"
#include "status_api.h"
#include "tron_api.h"
#include "tron_contracts.h"
#include "tron_helpers.h"
#include "tron_priv.h"
#include "tron_txn_helpers.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "wallet_list.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/**
 * @brief Whitelisted contracts with respective token symbol
 * @details A map of Tron contract addresses with their token symbols. These
 * will enable the device to verify the TRC20 token transaction in a
 * user-friendly manner.
 *
 * @see trc20_contracts_t
 */
extern const trc20_contracts_t trc20_contracts[];

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
 * @brief Checks if the provided token address is whitelisted and return the
 * matching contract instance.
 *
 * @param address Reference to the buffer containing the token address
 * @param contract Pointer to store the matched contract address instance
 *
 * @return bool Indicating if the provided token address is whitelisted
 * @return true If the address matches to an entry in the whitelist
 * @return false If the address does not match to an entry in the whitelist
 */
static bool is_token_whitelisted(const uint8_t *address,
                                 const trc20_contracts_t **contract);

/**
 * @brief User verification case when token is whitelisted
 *
 * @param data ABI encoded data field of tron txn
 * @param contract_address contract address of current txn
 * @param status Indicates the result of is_token_whitelisted
 *
 * @return bool Indicating parsing and verification status
 * @return true If parsing and user verification successful
 * @return false If parsing error or user denial
 */
static bool parse_whitelist(uint8_t *data,
                            const uint8_t *contract_address,
                            bool *status);

/**
 * @brief User verification case when token is not whitelisted
 *
 * @param data ABI encoded data field of tron txn
 * @param contract_address contract address of current txn
 *
 * @return bool Indicating parsing and verification status
 * @return true If parsing and user verification successful
 * @return false If parsing error or user denial
 */
static bool parse_unverified(uint8_t *data, uint8_t *contract_address);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool is_token_whitelisted(const uint8_t *address,
                                 const trc20_contracts_t **contract) {
  const trc20_contracts_t *match = NULL;
  bool status = false;
  for (int16_t i = 0; i < TRC20_WHITELISTED_CONTRACTS_COUNT; i++) {
    if (memcmp(address,
               trc20_contracts[i].address,
               TRON_INITIAL_ADDRESS_LENGTH) == 0) {
      match = &trc20_contracts[i];
      status = true;
      break;
    }
  }

  if (NULL != contract) {
    *contract = match;
  }
  return status;
}

static bool parse_whitelist(uint8_t *data,
                            const uint8_t *contract_address,
                            bool *status) {
  const trc20_contracts_t *contract;
  if (!is_token_whitelisted(contract_address, &contract)) {
    *status = 0;
    return true;
  }

  *status = 1;
  char msg[100] = "";
  snprintf(msg, sizeof(msg), "Send \n%s on \n%s", contract->symbol, TRON_NAME);
  if (!core_confirmation(msg, tron_send_error)) {
    return false;
  }

  uint8_t to_address[TRON_INITIAL_ADDRESS_LENGTH] = {0};
  to_address[0] = 0x41;
  uint8_t amount[32];

  // address is initial_address without '0x41' (20 Bytes)
  memcpy(to_address + 1, data + 4 + (32 - 20), 20);
  memcpy(amount, data + 4 + 32, 32);

  // receipent address
  char address[TRON_ACCOUNT_ADDRESS_LENGTH + 1] = {0};
  if (!base58_encode_check(to_address,
                           1 + 20,
                           HASHER_SHA2D,
                           address,
                           TRON_ACCOUNT_ADDRESS_LENGTH + 1)) {
    tron_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  if (!core_scroll_page(ui_text_verify_address, address, tron_send_error)) {
    return false;
  }

  // receipent amount
  char staticBufferInUTF8[200];
  memzero(staticBufferInUTF8, sizeof(staticBufferInUTF8));
  byte_array_to_hex_string(amount, 32, &(staticBufferInUTF8[0]), 65);
  convert_byte_array_to_decimal_string(64,
                                       contract->decimal,
                                       &(staticBufferInUTF8[0]),
                                       &(staticBufferInUTF8[100]),
                                       100);

  char display_amount[200] = "";
  snprintf(display_amount,
           sizeof(msg),
           UI_TEXT_VERIFY_AMOUNT,
           &(staticBufferInUTF8[100]),
           contract->symbol);
  if (!core_confirmation(display_amount, tron_send_error)) {
    return false;
  }

  return true;
}

static bool parse_unverified(uint8_t *data, uint8_t *contract_address) {
  char address[TRON_ACCOUNT_ADDRESS_LENGTH + 1] = {0};
  if (!base58_encode_check(contract_address,
                           1 + 20,
                           HASHER_SHA2D,
                           address,
                           TRON_ACCOUNT_ADDRESS_LENGTH + 1)) {
    tron_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  // store contract address
  ui_display_node *ui_node = ui_create_display_node(
      ui_text_verify_address, 15, address, sizeof(address));
  // store head node
  ui_display_node *curr_head = ui_node;

  // stringify data for user verification
  // create display nodes
  ui_node->next = extract_data(data);
  if (NULL == ui_node->next) {
    return false;
  }

  // Contract Unverifed, Display warning
  delay_scr_init(ui_text_unverified_contract, DELAY_TIME);

  while (NULL != curr_head) {
    if (!core_scroll_page(
            curr_head->title, curr_head->value, tron_send_error)) {
      return false;
    }
    curr_head = curr_head->next;
  }

  cy_free();
  return true;
}

static bool transfer_contract_txn(tron_transaction_contract_t *contract,
                                  bool use_signature_verification) {
  uint8_t to_address[TRON_INITIAL_ADDRESS_LENGTH] = {0};
  // uint8_t owner_address[TRON_INITIAL_ADDRESS_LENGTH] = {0};
  int64_t amount = 0;
  char address[TRON_ACCOUNT_ADDRESS_LENGTH + 1] = {0};

  tron_transfer_contract_t transfer_contract =
      TRON_TRANSFER_CONTRACT_INIT_DEFAULT;
  google_protobuf_any_t any = contract->parameter;
  pb_istream_t stream = pb_istream_from_buffer(any.value.bytes, any.value.size);

  if (!pb_decode(&stream, TRON_TRANSFER_CONTRACT_FIELDS, &transfer_contract)) {
    tron_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                    ERROR_DATA_FLOW_DECODING_FAILED);
    return false;
  }
  memcpy(&amount, &transfer_contract.amount, sizeof(int64_t));
  memcpy(to_address,
         (uint8_t *)transfer_contract.to_address,
         TRON_INITIAL_ADDRESS_LENGTH);

  // verify recipient address;
  if (!base58_encode_check(to_address,
                           1 + 20,
                           HASHER_SHA2D,
                           address,
                           TRON_ACCOUNT_ADDRESS_LENGTH + 1)) {
    tron_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  if (use_signature_verification) {
    if (!exchange_validate_stored_signature(address, sizeof(address))) {
      return false;
    }
  }

  if (!core_scroll_page(ui_text_verify_address, address, tron_send_error)) {
    return false;
  }

  // verify recipient amount
  char amount_string[30] = {'\0'};
  double decimal_amount = (double)amount;
  decimal_amount *= 1e-6;
  snprintf(amount_string, sizeof(amount_string), "%.6f", decimal_amount);

  char display[100] = {'\0'};
  snprintf(display,
           sizeof(display),
           UI_TEXT_VERIFY_AMOUNT,
           amount_string,
           TRON_LUNIT);

  if (!core_confirmation(display, tron_send_error)) {
    return false;
  }

  set_app_flow_status(TRON_SIGN_TXN_STATUS_VERIFY);
  return true;
}

static bool trigger_smart_contract_txn(tron_transaction_contract_t *contract) {
  tron_trigger_smart_contract_t trigger_smc_contract =
      TRON_TRIGGER_SMART_CONTRACT_INIT_DEFAULT;
  google_protobuf_any_t any = contract->parameter;
  pb_istream_t stream = pb_istream_from_buffer(any.value.bytes, any.value.size);

  if (!pb_decode(
          &stream, TRON_TRIGGER_SMART_CONTRACT_FIELDS, &trigger_smc_contract)) {
    tron_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                    ERROR_DATA_FLOW_DECODING_FAILED);
    return false;
  }

  // cpy details
  uint8_t contract_address[TRON_INITIAL_ADDRESS_LENGTH];
  uint8_t data[68];
  memcpy(contract_address,
         (uint8_t *)trigger_smc_contract.contract_address,
         TRON_INITIAL_ADDRESS_LENGTH);
  memcpy(data, (uint8_t *)trigger_smc_contract.data, 68);

  // user verification
  bool is_verified = 0;
  if (!parse_whitelist(data, contract_address, &is_verified)) {
    return false;
  }
  if (0 == is_verified && (!parse_unverified(data, contract_address))) {
    return false;
  }

  set_app_flow_status(TRON_SIGN_TXN_STATUS_VERIFY);
  return true;
}

static bool transfer_asset_contract_txn(tron_transaction_contract_t *contract) {
  tron_transfer_asset_contract_t tac_contract =
      TRON_TRANSFER_ASSET_CONTRACT_INIT_DEFAULT;
  google_protobuf_any_t any = contract->parameter;
  pb_istream_t stream = pb_istream_from_buffer(any.value.bytes, any.value.size);

  if (!pb_decode(&stream, TRON_TRANSFER_ASSET_CONTRACT_FIELDS, &tac_contract)) {
    tron_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                    ERROR_DATA_FLOW_DECODING_FAILED);
    return false;
  }

  uint8_t to_address[TRON_INITIAL_ADDRESS_LENGTH] = {0};
  int64_t amount = 0;
  char address[TRON_ACCOUNT_ADDRESS_LENGTH + 1] = {0};

  memcpy(&amount, &tac_contract.amount, sizeof(int64_t));
  memcpy(to_address,
         (uint8_t *)tac_contract.to_address,
         TRON_INITIAL_ADDRESS_LENGTH);

  // verify recipient address;
  if (!base58_encode_check(to_address,
                           1 + 20,
                           HASHER_SHA2D,
                           address,
                           TRON_ACCOUNT_ADDRESS_LENGTH + 1)) {
    tron_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG, 2);
    return false;
  }

  delay_scr_init(ui_text_unverified_contract, DELAY_TIME);

  if (!core_scroll_page(ui_text_verify_address, address, tron_send_error)) {
    return false;
  }

  // verify recipient amount
  char amount_string[30] = {'\0'};
  char display[100] = {'\0'};

  snprintf(amount_string, sizeof(amount_string), "%lli", amount);
  snprintf(display, sizeof(display), "Verify amount\n%s", amount_string);

  if (!core_confirmation(display, tron_send_error)) {
    return false;
  }
  // TODO: DISPLAY ASSET NAME/TOKEN ID

  set_app_flow_status(TRON_SIGN_TXN_STATUS_VERIFY);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

bool extract_contract_info(tron_transaction_raw_t *raw_txn,
                           bool use_signature_verification) {
  if (!(raw_txn->contract_count > 0)) {
    tron_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                    ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  }

  tron_transaction_contract_t contract = raw_txn->contract[0];

  switch (contract.type) {
    case TRON_TRANSACTION_CONTRACT_TRANSFER_CONTRACT: {
      // Transfer TRX type
      if (!transfer_contract_txn(&contract, use_signature_verification)) {
        return false;
      }
      break;
    }
    case TRON_TRANSACTION_CONTRACT_TRIGGER_SMART_CONTRACT: {
      // TRC20 type
      if (!trigger_smart_contract_txn(&contract)) {
        return false;
      }
      break;
    }
    case TRON_TRANSACTION_CONTRACT_TRANSFER_ASSET_CONTRACT: {
      // TRC10 type
      if (!transfer_asset_contract_txn(&contract)) {
        return false;
      }
      break;
    }
    default: {
      tron_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }
  }
  return true;
}
