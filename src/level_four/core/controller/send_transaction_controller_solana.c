/**
 * @file    send_transaction_controller_solana.c
 * @author  Cypherock X1 Team
 * @brief   Send transaction next controller for SOLANA.
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

solana_unsigned_txn solana_unsigned_txn_ptr;
uint8_t *solana_unsigned_txn_byte_array = NULL;
uint16_t solana_unsigned_txn_len;

void send_transaction_controller_solana() {
  switch (flow_level.level_three) {
    case SEND_TXN_VERIFY_COIN_SOLANA: {
      uint8_t arr[3] = {0x01, 0x1f, 0x40};
      transmit_data_to_app(SEND_TXN_REQ_UNSIGNED_TXN, arr, sizeof(arr));
      flow_level.level_three = SEND_TXN_UNSIGNED_TXN_WAIT_SCREEN_SOLANA;
    } break;

    case SEND_TXN_UNSIGNED_TXN_WAIT_SCREEN_SOLANA: {
      uint8_t *data_array = NULL;
      uint16_t msg_size = 0;
      if (get_usb_msg_by_cmd_type(
              SEND_TXN_UNSIGNED_TXN, &data_array, &msg_size)) {
        solana_unsigned_txn_byte_array = (uint8_t *)cy_malloc(msg_size);
        solana_unsigned_txn_len = msg_size;
        memcpy(solana_unsigned_txn_byte_array, data_array, msg_size);

        int status =
            solana_byte_array_to_unsigned_txn(solana_unsigned_txn_byte_array,
                                              solana_unsigned_txn_len,
                                              &solana_unsigned_txn_ptr);

        clear_message_received_data();
        flow_level.level_three = SEND_TXN_UNSIGNED_TXN_RECEIVED_SOLANA;

        if (status == SOL_OK)
          status = solana_validate_unsigned_txn(&solana_unsigned_txn_ptr);

        if (status != SOL_OK) {
          LOG_ERROR("Solana error code: %d", status);
          instruction_scr_destructor();
          mark_error_screen(ui_text_worng_eth_transaction);
          comm_reject_request(SEND_TXN_USER_VERIFIES_ADDRESS, 0);
          reset_flow_level();
        }
      }
    } break;

    case SEND_TXN_UNSIGNED_TXN_RECEIVED_SOLANA: {
      flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS_SOLANA;
    } break;

    case SEND_TXN_VERIFY_CONTRACT_ADDRESS: {
      flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS_SOLANA;
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_SOLANA: {
      flow_level.level_three = SEND_TXN_CALCULATE_AMOUNT_SOLANA;
    } break;

    case SEND_TXN_CALCULATE_AMOUNT_SOLANA: {
      flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_AMOUNT_SOLANA;
    } break;

    case SEND_TXN_VERIFY_RECEIPT_AMOUNT_SOLANA: {
      flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_FEES_SOLANA;
    } break;

    case SEND_TXN_VERIFY_RECEIPT_FEES_SOLANA: {
      flow_level.level_three = SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD_SOLANA;
    } break;

    case SEND_TXN_VERIFY_RECEIPT_ADDRESS_SEND_CMD_SOLANA: {
      memzero(wallet_credential_data.passphrase,
              sizeof(wallet_credential_data.passphrase));
      if (WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
        flow_level.level_three = SEND_TXN_ENTER_PASSPHRASE_SOLANA;
      } else {
        flow_level.level_three = SEND_TXN_CHECK_PIN_SOLANA;
      }
    } break;

    case SEND_TXN_ENTER_PASSPHRASE_SOLANA: {
      flow_level.level_three = SEND_TXN_CONFIRM_PASSPHRASE_SOLANA;
    } break;

    case SEND_TXN_CONFIRM_PASSPHRASE_SOLANA: {
      snprintf(wallet_credential_data.passphrase,
               sizeof(wallet_credential_data.passphrase),
               "%s",
               flow_level.screen_input.input_text);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = SEND_TXN_CHECK_PIN_SOLANA;
    } break;

    case SEND_TXN_CHECK_PIN_SOLANA: {
      if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
        flow_level.level_three = SEND_TXN_ENTER_PIN_SOLANA;
      } else {
        flow_level.level_three = SEND_TXN_TAP_CARD_SOLANA;
      }

    } break;

    case SEND_TXN_ENTER_PIN_SOLANA: {
      sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                 strnlen(flow_level.screen_input.input_text,
                         sizeof(flow_level.screen_input.input_text)),
                 wallet_credential_data.password_single_hash);
      sha256_Raw(wallet_credential_data.password_single_hash,
                 SHA256_DIGEST_LENGTH,
                 wallet.password_double_hash);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = SEND_TXN_TAP_CARD_SOLANA;
    } break;

    case SEND_TXN_TAP_CARD_SOLANA: {
      tap_card_data.desktop_control = true;
      tap_threshold_cards_for_reconstruction_flow_controller(1);
    } break;

    case SEND_TXN_TAP_CARD_SEND_CMD_SOLANA: {
      flow_level.level_three = SEND_TXN_UPDATE_BLOCKHASH_SOLANA;
      uint8_t arr[1] = {1};
      transmit_data_to_app(SEND_TXN_UNSIGNED_TXN, arr, sizeof(arr));
    } break;

    case SEND_TXN_UPDATE_BLOCKHASH_SOLANA: {
      uint8_t *blockhash_received = NULL;
      uint16_t blockhash_msg_size = 0;
      uint8_t solana_latest_blockhash[SOLANA_BLOCKHASH_LENGTH] = {0};

      if (get_usb_msg_by_cmd_type(SEND_TXN_PRE_SIGNING_DATA,
                                  &blockhash_received,
                                  &blockhash_msg_size)) {
        if (blockhash_msg_size != SOLANA_BLOCKHASH_LENGTH) {
          comm_reject_invalid_cmd();
          reset_flow_level();
        }
        memcpy(solana_latest_blockhash,
               blockhash_received,
               SOLANA_BLOCKHASH_LENGTH);
        clear_message_received_data();

        int status = solana_update_blockhash_in_byte_array(
            solana_unsigned_txn_byte_array, solana_latest_blockhash);
        if (status != SOL_OK) {
          LOG_ERROR("SOL: %d", status);
          comm_reject_request(SEND_TXN_PRE_SIGNING_DATA, 0);
          reset_flow_level();
        }
        flow_level.level_three = SEND_TXN_READ_DEVICE_SHARE_SOLANA;
      }

    } break;

    case SEND_TXN_READ_DEVICE_SHARE_SOLANA:
      wallet_shamir_data.share_x_coords[1] = 5;
      get_flash_wallet_share_by_name((const char *)wallet.wallet_name,
                                     wallet_shamir_data.mnemonic_shares[1]);
      memcpy(wallet_shamir_data.share_encryption_data[1],
             wallet_shamir_data.share_encryption_data[0],
             PADDED_NONCE_SIZE + WALLET_MAC_SIZE);
      flow_level.level_three = SEND_TXN_SIGN_TXN_SOLANA;
      break;

    case SEND_TXN_SIGN_TXN_SOLANA: {
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

      uint8_t sig[64];
      solana_sig_unsigned_byte_array(
          solana_unsigned_txn_byte_array,
          solana_unsigned_txn_len,
          (const txn_metadata *)&var_send_transaction_data.transaction_metadata,
          mnemo,
          wallet_credential_data.passphrase,
          sig);
      transmit_data_to_app(SEND_TXN_SENDING_SIGNED_TXN, sig, 64);
      mnemonic_clear();
      memzero(secret, sizeof(secret));
      memzero(wallet_shamir_data.mnemonic_shares,
              sizeof(wallet_shamir_data.mnemonic_shares));
      memzero(wallet_credential_data.passphrase,
              sizeof(wallet_credential_data.passphrase));

      flow_level.level_three = SEND_TXN_WAITING_SCREEN_SOLANA;
    } break;

    case SEND_TXN_WAITING_SCREEN_SOLANA:
      instruction_scr_destructor();
      flow_level.level_three = SEND_TXN_FINAL_SCREEN_SOLANA;
      break;

    case SEND_TXN_FINAL_SCREEN_SOLANA:
      reset_flow_level();
      break;

    default:
      break;
  }
}
