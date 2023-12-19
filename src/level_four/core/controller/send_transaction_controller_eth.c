/**
 * @file    send_transaction_controller_eth.c
 * @author  Cypherock X1 Team
 * @brief   Send transaction next controller for ETH.
 *          Handles post event (only next events) operations for send
 *transaction flow initiated by desktop app.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
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
#include "communication.h"
#include "controller_level_four.h"
#include "controller_main.h"
#include "controller_tap_cards.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "ui_confirmation.h"
#include "ui_instruction.h"
#include "ui_message.h"

extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;

extern lv_task_t *timeout_task;

evm_unsigned_txn eth_unsigned_txn_ptr = {
    .nonce_size = {0},
    .nonce = {0},
    .gas_price_size = {0},
    .gas_price = {0},
    .gas_limit_size = {0},
    .gas_limit = {0},
    .to_address = {0},
    .value_size = {0},
    .value = {0},
    .data_size = 0,
    .data = NULL,
    .chain_id_size = {0},
    .chain_id = {0},
    .dummy_r = {0},
    .dummy_s = {0},
};
uint8_t *eth_unsigned_txn_byte_array = NULL;
uint16_t eth_unsigned_txn_len;

extern ui_display_node *current_display_node;
extern bool evm_is_token_whitelisted;

void send_transaction_controller_eth() {
  switch (flow_level.level_three) {
    case SEND_TXN_VERIFY_COIN_ETH: {
      uint8_t arr[3] = {0x01, 0x1f, 0x40};
      transmit_data_to_app(SEND_TXN_REQ_UNSIGNED_TXN, arr, sizeof(arr));
      flow_level.level_three = SEND_TXN_UNSIGNED_TXN_WAIT_SCREEN_ETH;
    } break;

    case SEND_TXN_UNSIGNED_TXN_WAIT_SCREEN_ETH: {
      uint8_t *data_array = NULL;
      uint16_t msg_size = 0;
      if (get_usb_msg_by_cmd_type(
              SEND_TXN_UNSIGNED_TXN, &data_array, &msg_size)) {
        eth_unsigned_txn_byte_array = (uint8_t *)cy_malloc(msg_size);
        eth_unsigned_txn_len = msg_size;
        memcpy(eth_unsigned_txn_byte_array, data_array, msg_size);

        clear_message_received_data();
        flow_level.level_three = SEND_TXN_UNSIGNED_TXN_RECEIVED_ETH;
      }
    } break;

    case SEND_TXN_VERIFY_BLIND_SIGNING_ETH: {
      flow_level.level_three = SEND_TXN_VERIFY_DERIVATION_PATH;
    } break;

    case SEND_TXN_VERIFY_DERIVATION_PATH: {
      flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS_ETH;
    } break;

    case SEND_TXN_VERIFY_CONTRACT_ADDRESS: {
      flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS_ETH;
    } break;

    case SEND_TXN_VERIFY_TXN_NONCE_ETH: {
      flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS_ETH;
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_ETH: {
      flow_level.level_three = SEND_TXN_CALCULATE_AMOUNT_ETH;
    } break;

    case SEND_TXN_VERIFY_RECEIPT_AMOUNT_ETH: {
      flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_FEES_ETH;
    } break;

    case SEND_TXN_VERIFY_RECEIPT_FEES_ETH: {
      flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD_ETH;
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD_ETH: {
      flow_level.level_three = SEND_TXN_DISPLAY_INFO_ETH;
    } break;

    case SEND_TXN_DISPLAY_INFO_ETH: {
      if (current_display_node == NULL) {
        memzero(wallet_credential_data.passphrase,
                sizeof(wallet_credential_data.passphrase));
        if (WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
          flow_level.level_three = SEND_TXN_ENTER_PASSPHRASE_ETH;
        } else {
          flow_level.level_three = SEND_TXN_CHECK_PIN_ETH;
        }
      } else {
        current_display_node = current_display_node->next;
      }
    } break;

    case SEND_TXN_ENTER_PASSPHRASE_ETH: {
      flow_level.level_three = SEND_TXN_CONFIRM_PASSPHRASE_ETH;
    } break;

    case SEND_TXN_CONFIRM_PASSPHRASE_ETH: {
      snprintf(wallet_credential_data.passphrase,
               sizeof(wallet_credential_data.passphrase),
               "%s",
               flow_level.screen_input.input_text);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = SEND_TXN_CHECK_PIN_ETH;
    } break;

    case SEND_TXN_CHECK_PIN_ETH: {
      if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
        flow_level.level_three = SEND_TXN_ENTER_PIN_ETH;
      } else {
        flow_level.level_three = SEND_TXN_TAP_CARD_ETH;
      }

    } break;

    case SEND_TXN_ENTER_PIN_ETH: {
      sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                 strnlen(flow_level.screen_input.input_text,
                         sizeof(flow_level.screen_input.input_text)),
                 wallet_credential_data.password_single_hash);
      sha256_Raw(wallet_credential_data.password_single_hash,
                 SHA256_DIGEST_LENGTH,
                 wallet.password_double_hash);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = SEND_TXN_TAP_CARD_ETH;
    } break;

    case SEND_TXN_TAP_CARD_ETH: {
      tap_card_data.desktop_control = true;
      tap_threshold_cards_for_reconstruction_flow_controller(1);
    } break;

    case SEND_TXN_TAP_CARD_SEND_CMD_ETH: {
      flow_level.level_three = SEND_TXN_READ_DEVICE_SHARE_ETH;
    } break;

    case SEND_TXN_READ_DEVICE_SHARE_ETH:
      wallet_shamir_data.share_x_coords[1] = 5;
      get_flash_wallet_share_by_name((const char *)wallet.wallet_name,
                                     wallet_shamir_data.mnemonic_shares[1]);
      memcpy(wallet_shamir_data.share_encryption_data[1],
             wallet_shamir_data.share_encryption_data[0],
             PADDED_NONCE_SIZE + WALLET_MAC_SIZE);
      flow_level.level_three = SEND_TXN_SIGN_TXN_ETH;
      break;

    case SEND_TXN_SIGN_TXN_ETH: {
      uint8_t secret[BLOCK_SIZE];
      if (WALLET_IS_PIN_SET(wallet.wallet_info))
        decrypt_shares();
      recover_secret_from_shares(BLOCK_SIZE,
                                 MINIMUM_NO_OF_SHARES,
                                 wallet_shamir_data.mnemonic_shares,
                                 wallet_shamir_data.share_x_coords,
                                 secret);
      mnemonic_clear();
      const char *mnemo =
          mnemonic_from_data(secret, wallet.number_of_mnemonics * 4 / 3);
      ASSERT(mnemo != NULL);

      uint8_t sig[65];
      sig_unsigned_byte_array(
          eth_unsigned_txn_byte_array,
          eth_unsigned_txn_len,
          (const txn_metadata *)&var_send_transaction_data.transaction_metadata,
          mnemo,
          wallet_credential_data.passphrase,
          sig);
      transmit_data_to_app(SEND_TXN_SENDING_SIGNED_TXN, sig, 65);
      mnemonic_clear();
      memzero(secret, sizeof(secret));
      memzero(wallet_shamir_data.mnemonic_shares,
              sizeof(wallet_shamir_data.mnemonic_shares));
      memzero(wallet_credential_data.passphrase,
              sizeof(wallet_credential_data.passphrase));

      flow_level.level_three = SEND_TXN_WAITING_SCREEN_ETH;
    } break;

    case SEND_TXN_WAITING_SCREEN_ETH:
      instruction_scr_destructor();
      lv_obj_clean(lv_scr_act());
      flow_level.level_three = SEND_TXN_FINAL_SCREEN_ETH;
      break;

    case SEND_TXN_FINAL_SCREEN_ETH:
      reset_flow_level();
      break;

    default:
      break;
  }
}
