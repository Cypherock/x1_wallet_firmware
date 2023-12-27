/**
 * @file    receive_transaction_controller.c
 * @author  Cypherock X1 Team
 * @brief   Receive transaction next controller (for BTC).
 *          Handles post event (only next events) operations for receive
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
#include "bip32.h"
#include "communication.h"
#include "constant_texts.h"
#include "controller_level_four.h"
#include "controller_tap_cards.h"
#include "near_context.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "ui_confirmation.h"
#include "ui_instruction.h"

extern Wallet_credential_data wallet_credential_data;

extern Receive_Transaction_Data receive_transaction_data;
Coin_Specific_Data_Struct coin_specific_data;

void receive_transaction_controller_near() {
  switch (flow_level.level_three) {
    case RECV_TXN_FIND_XPUB_NEAR: {
      receive_transaction_data.near_acc_found = false;

      if (false && receive_transaction_data.near_account_type == 1) {
        coin_specific_data.coin_type = COIN_TYPE_NEAR;
        memcpy(coin_specific_data.wallet_id,
               receive_transaction_data.wallet_id,
               WALLET_ID_SIZE);
        coin_specific_data.coin_data =
            (uint8_t *)cy_malloc(NEAR_COIN_DATA_MAX_LEN);
        uint16_t len = 0;

        get_coin_data(&coin_specific_data, NEAR_COIN_DATA_MAX_LEN, &len);
        size_t acc_count =
            near_get_account_ids_count(coin_specific_data.coin_data, len);
        receive_transaction_data.near_acc_count = acc_count;

        char *acc_id[NEAR_REGISTERED_ACCOUNT_COUNT] = {0};

        near_deserialize_account_ids(
            coin_specific_data.coin_data, len, acc_id, acc_count);

        for (size_t i = 0; i < NEAR_REGISTERED_ACCOUNT_COUNT; i++) {
          if (strcmp(acc_id[i],
                     receive_transaction_data.near_registered_account) == 0) {
            receive_transaction_data.near_acc_found = true;
            break;
          }
        }
      }

      memzero(wallet_credential_data.passphrase,
              sizeof(wallet_credential_data.passphrase));
      if (WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
        flow_level.level_three = RECV_TXN_ENTER_PASSPHRASE_NEAR;
      } else {
        flow_level.level_three = RECV_TXN_CHECK_PIN_NEAR;
      }
    } break;

    case RECV_TXN_ENTER_PASSPHRASE_NEAR: {
      flow_level.level_three = RECV_TXN_CONFIRM_PASSPHRASE_NEAR;
    } break;

    case RECV_TXN_CONFIRM_PASSPHRASE_NEAR: {
      snprintf(wallet_credential_data.passphrase,
               sizeof(wallet_credential_data.passphrase),
               "%s",
               flow_level.screen_input.input_text);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = RECV_TXN_CHECK_PIN_NEAR;
      flow_level.level_one = 1;
    } break;

    case RECV_TXN_CHECK_PIN_NEAR: {
      if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
        flow_level.level_three = RECV_TXN_ENTER_PIN_NEAR;
      } else {
        flow_level.level_three = RECV_TXN_TAP_CARD_NEAR;
      }
    } break;

    case RECV_TXN_ENTER_PIN_NEAR: {
      sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                 strnlen(flow_level.screen_input.input_text,
                         sizeof(flow_level.screen_input.input_text)),
                 wallet_credential_data.password_single_hash);
      sha256_Raw(wallet_credential_data.password_single_hash,
                 SHA256_DIGEST_LENGTH,
                 wallet.password_double_hash);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));

      flow_level.level_three = RECV_TXN_TAP_CARD_NEAR;
    } break;

    case RECV_TXN_TAP_CARD_NEAR: {
      tap_card_data.desktop_control = true;
      // TODO: Shorten func name
      tap_threshold_cards_for_reconstruction_flow_controller(1);
    } break;

    case RECV_TXN_TAP_CARD_SEND_CMD_NEAR: {
      flow_level.level_three = RECV_TXN_READ_DEVICE_SHARE_NEAR;
      if (!receive_transaction_data.near_acc_found &&
          receive_transaction_data.near_account_type == 1)
        transmit_one_byte(RECV_TXN_XPUBS_EXISTS_ON_DEVICE, 1);
    } break;

    case RECV_TXN_READ_DEVICE_SHARE_NEAR:
      wallet_shamir_data.share_x_coords[1] = 5;
      get_flash_wallet_share_by_name((const char *)wallet.wallet_name,
                                     wallet_shamir_data.mnemonic_shares[1]);
      memcpy(wallet_shamir_data.share_encryption_data[1],
             wallet_shamir_data.share_encryption_data[0],
             PADDED_NONCE_SIZE + WALLET_MAC_SIZE);
      flow_level.level_three = RECV_TXN_DERIVE_ADD_SCREEN_NEAR;
      break;

    case RECV_TXN_DERIVE_ADD_SCREEN_NEAR: {
      flow_level.level_three = RECV_TXN_DERIVE_ADD_NEAR;
    } break;

    case RECV_TXN_DERIVE_ADD_NEAR: {
      // TODO: Extract common part i.e. secret recreation and public key
      // derication
      uint8_t secret[BLOCK_SIZE] = {0};
      if (WALLET_IS_PIN_SET(wallet.wallet_info))
        decrypt_shares();
      recover_secret_from_shares(BLOCK_SIZE,
                                 MINIMUM_NO_OF_SHARES,
                                 wallet_shamir_data.mnemonic_shares,
                                 wallet_shamir_data.share_x_coords,
                                 secret);
      memzero(wallet_shamir_data.share_encryption_data,
              sizeof(wallet_shamir_data.share_encryption_data));
      mnemonic_clear();
      const char *mnemo =
          mnemonic_from_data(secret, wallet.number_of_mnemonics * 4 / 3);
      HDNode node;
      uint8_t seed[64] = {0};

      memzero(seed, sizeof(seed));

      mnemonic_to_seed(mnemo, wallet_credential_data.passphrase, seed, NULL);
      mnemonic_clear();
      memzero(wallet_credential_data.passphrase,
              sizeof(wallet_credential_data.passphrase));

      uint32_t path[] = {
          BYTE_ARRAY_TO_UINT32(receive_transaction_data.purpose),
          BYTE_ARRAY_TO_UINT32(receive_transaction_data.coin_index),
          BYTE_ARRAY_TO_UINT32(receive_transaction_data.account_index),
          BYTE_ARRAY_TO_UINT32(receive_transaction_data.change_index),
          BYTE_ARRAY_TO_UINT32(receive_transaction_data.address_index),
      };
      derive_hdnode_from_path(path, 5, ED25519_NAME, seed, &node);
      memzero(path, sizeof(path));
      memzero(receive_transaction_data.near_pubkey,
              sizeof(receive_transaction_data.near_pubkey));
      memcpy(receive_transaction_data.near_pubkey,
             node.public_key + 1,
             32 * sizeof(uint8_t));

      if (receive_transaction_data.near_account_type == 0 ||
          receive_transaction_data.near_acc_found) {
        flow_level.level_three = RECV_TXN_DISPLAY_ADDR_NEAR;
        instruction_scr_destructor();
      } else {
        flow_level.level_three = RECV_TXN_WAIT_FOR_LINK_NEAR;
      }
    } break;

    case RECV_TXN_WAIT_FOR_LINK_NEAR: {
      uint8_t *data = NULL;
      uint16_t size = 0;
      if (get_usb_msg_by_cmd_type(
              RECV_TXN_USER_VERIFIED_ACCOUNT, &data, &size)) {
        flow_level.level_three = RECV_TXN_DISPLAY_ACC_NEAR;
        clear_message_received_data();
        instruction_scr_destructor();
      }
    } break;

    case RECV_TXN_DISPLAY_ACC_NEAR: {
      flow_level.level_three = RECV_TXN_DISPLAY_ADDR_NEAR;
    } break;

    case RECV_TXN_DISPLAY_ADDR_NEAR: {
      flow_level.level_three = RECV_TXN_FINAL_SCREEN_NEAR;

      uint8_t data[1 + sizeof(receive_transaction_data.near_pubkey)] = {0};
      data[0] = 1;    // confirmation byte

      if (receive_transaction_data.near_account_type == 1 &&
          !receive_transaction_data.near_acc_found) {
        if (receive_transaction_data.near_acc_count <
            NEAR_REGISTERED_ACCOUNT_COUNT) {
          uint16_t len = 0;
          memzero(coin_specific_data.coin_data, NEAR_COIN_DATA_MAX_LEN);
          get_coin_data(&coin_specific_data, NEAR_COIN_DATA_MAX_LEN, &len);

          char *acc_id[NEAR_REGISTERED_ACCOUNT_COUNT] = {0};

          near_deserialize_account_ids(coin_specific_data.coin_data,
                                       len,
                                       acc_id,
                                       receive_transaction_data.near_acc_count);

          acc_id[receive_transaction_data.near_acc_count] =
              receive_transaction_data.near_registered_account;

          uint8_t new_near_coin_data[NEAR_COIN_DATA_MAX_LEN] = {0};
          len = 0;

          near_serialize_account_ids(
              (const char **)acc_id,
              receive_transaction_data.near_acc_count + 1,
              new_near_coin_data,
              &len);
          memcpy(coin_specific_data.coin_data,
                 new_near_coin_data,
                 NEAR_COIN_DATA_MAX_LEN);

          int status = set_coin_data(&coin_specific_data, len);
          if (status != 0) {
            comm_reject_request(COIN_SPECIFIC_DATA_ERROR, status);
            reset_flow_level();
            return;
          }
        } else {
          data[0] = 2;
          flow_level.level_three = RECV_TXN_WAIT_FOR_REPLACE_NEAR_SCREEN;
        }
      }

      memcpy(data + 1,
             receive_transaction_data.near_pubkey,
             sizeof(receive_transaction_data.near_pubkey));
      transmit_data_to_app(RECV_TXN_USER_VERIFIED_ADDRESS, data, sizeof(data));
    } break;

    case RECV_TXN_WAIT_FOR_REPLACE_NEAR_SCREEN: {
      flow_level.level_three = RECV_TXN_WAIT_FOR_REPLACE_NEAR;
    } break;

    case RECV_TXN_WAIT_FOR_REPLACE_NEAR: {
      uint8_t *data = NULL;
      uint16_t size = 0;
      if (get_usb_msg_by_cmd_type(RECV_TXN_REPLACE_ACCOUNT, &data, &size)) {
        flow_level.level_three = RECV_TXN_SELECT_REPLACE_ACC_NEAR;
        clear_message_received_data();
        instruction_scr_destructor();
      }
    } break;

    case RECV_TXN_SELECT_REPLACE_ACC_NEAR: {
      flow_level.level_three = RECV_TXN_VERIFY_SAVE_ACC_NEAR;
    } break;

    case RECV_TXN_VERIFY_SAVE_ACC_NEAR: {
      uint16_t filled_length = 0;
      memzero(coin_specific_data.coin_data, NEAR_COIN_DATA_MAX_LEN);

      get_coin_data(
          &coin_specific_data, NEAR_COIN_DATA_MAX_LEN, &filled_length);
      size_t acc_count = near_get_account_ids_count(
          coin_specific_data.coin_data, filled_length);

      char *acc_id[NEAR_REGISTERED_ACCOUNT_COUNT] = {0};

      near_deserialize_account_ids(
          coin_specific_data.coin_data, filled_length, acc_id, acc_count);

      acc_id[receive_transaction_data.near_acc_index] =
          receive_transaction_data.near_registered_account;

      uint8_t new_near_coin_data[NEAR_COIN_DATA_MAX_LEN] = {0};
      filled_length = 0;
      near_serialize_account_ids(
          (const char **)acc_id, acc_count, new_near_coin_data, &filled_length);
      memcpy(coin_specific_data.coin_data,
             new_near_coin_data,
             NEAR_COIN_DATA_MAX_LEN);

      int status = set_coin_data(&coin_specific_data, filled_length);
      if (status != 0) {
        comm_reject_request(COIN_SPECIFIC_DATA_ERROR, status);
        reset_flow_level();
        return;
      }

      transmit_one_byte_confirm(RECV_TXN_REPLACE_ACCOUNT);
      reset_flow_level();
    } break;

    case RECV_TXN_FINAL_SCREEN_NEAR: {
      reset_flow_level();
    } break;

    default:
      break;
  }

  return;
}
