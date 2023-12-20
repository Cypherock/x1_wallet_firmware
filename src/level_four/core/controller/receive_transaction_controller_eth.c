/**
 * @file    receive_transaction_controller_eth.c
 * @author  Cypherock X1 Team
 * @brief   Receive transaction next controller for ETH.
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
#include "harmony.h"
#include "sha2.h"
#include "shamir_wrapper.h"
#include "ui_confirmation.h"
#include "ui_instruction.h"

extern Receive_Transaction_Data receive_transaction_data;
extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;

void receive_transaction_controller_eth() {
  switch (flow_level.level_three) {
      // TODO: Rename RECV_TXN_FIND_XPUB in all receive tasks to a relevant name
      // (PROCESS_METADATA)
    case RECV_TXN_FIND_XPUB_ETH: {
      memzero(wallet_credential_data.passphrase,
              sizeof(wallet_credential_data.passphrase));
      if (WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
        flow_level.level_three = RECV_TXN_ENTER_PASSPHRASE_ETH;
      } else {
        flow_level.level_three = RECV_TXN_CHECK_PIN_ETH;
      }
    } break;

    case RECV_TXN_ENTER_PASSPHRASE_ETH: {
      flow_level.level_three = RECV_TXN_CONFIRM_PASSPHRASE_ETH;
    } break;

    case RECV_TXN_CONFIRM_PASSPHRASE_ETH: {
      snprintf(wallet_credential_data.passphrase,
               sizeof(wallet_credential_data.passphrase),
               "%s",
               flow_level.screen_input.input_text);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = RECV_TXN_CHECK_PIN_ETH;
      flow_level.level_one = 1;
    } break;

    case RECV_TXN_CHECK_PIN_ETH: {
      if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
        flow_level.level_three = RECV_TXN_ENTER_PIN_ETH;
      } else {
        flow_level.level_three = RECV_TXN_TAP_CARD_ETH;
      }
    } break;

    case RECV_TXN_ENTER_PIN_ETH: {
      sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                 strnlen(flow_level.screen_input.input_text,
                         sizeof(flow_level.screen_input.input_text)),
                 wallet_credential_data.password_single_hash);
      sha256_Raw(wallet_credential_data.password_single_hash,
                 SHA256_DIGEST_LENGTH,
                 wallet.password_double_hash);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = RECV_TXN_TAP_CARD_ETH;
    } break;

    case RECV_TXN_TAP_CARD_ETH: {
      tap_card_data.desktop_control = true;
      tap_threshold_cards_for_reconstruction_flow_controller(1);
    } break;

    case RECV_TXN_TAP_CARD_SEND_CMD_ETH: {
      flow_level.level_three = RECV_TXN_READ_DEVICE_SHARE_ETH;
    } break;

    case RECV_TXN_READ_DEVICE_SHARE_ETH:
      wallet_shamir_data.share_x_coords[1] = 5;
      get_flash_wallet_share_by_name((const char *)wallet.wallet_name,
                                     wallet_shamir_data.mnemonic_shares[1]);
      memcpy(wallet_shamir_data.share_encryption_data[1],
             wallet_shamir_data.share_encryption_data[0],
             PADDED_NONCE_SIZE + WALLET_MAC_SIZE);
      flow_level.level_three = RECV_TXN_DERIVE_ADD_SCREEN_ETH;
      break;

    case RECV_TXN_DERIVE_ADD_SCREEN_ETH: {
      flow_level.level_three = RECV_TXN_DERIVE_ADD_ETH;
    } break;

    case RECV_TXN_DERIVE_ADD_ETH: {
      uint8_t secret[BLOCK_SIZE];
      if (WALLET_IS_PIN_SET(wallet.wallet_info))
        decrypt_shares();
      recover_secret_from_shares(BLOCK_SIZE,
                                 MINIMUM_NO_OF_SHARES,
                                 wallet_shamir_data.mnemonic_shares,
                                 wallet_shamir_data.share_x_coords,
                                 secret);
      memzero(wallet_shamir_data.mnemonic_shares,
              sizeof(wallet_shamir_data.mnemonic_shares));
      mnemonic_clear();
      const char *mnemo =
          mnemonic_from_data(secret, wallet.number_of_mnemonics * 4 / 3);
      HDNode node;
      uint8_t seed[64];

      memzero(seed, sizeof(seed));
      mnemonic_to_seed(mnemo, wallet_credential_data.passphrase, seed, NULL);
      mnemonic_clear();
      memzero(wallet_credential_data.passphrase,
              sizeof(wallet_credential_data.passphrase));
      hdnode_from_seed(seed, sizeof(seed), SECP256K1_NAME, &node);

      hdnode_private_ckd(
          &node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.purpose));
      hdnode_private_ckd(
          &node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.coin_index));
      hdnode_private_ckd(
          &node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.account_index));

      hdnode_fill_public_key(&node);

      hdnode_public_ckd(
          &node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.change_index));
      hdnode_fill_public_key(&node);
      hdnode_public_ckd(
          &node, BYTE_ARRAY_TO_UINT32(receive_transaction_data.address_index));
      hdnode_fill_public_key(&node);
      hdnode_get_ethereum_pubkeyhash(&node,
                                     receive_transaction_data.eth_pubkeyhash);

      flow_level.level_three = RECV_TXN_DISPLAY_ADDR_ETH;
    } break;

    case RECV_TXN_DISPLAY_ADDR_ETH: {
      uint64_t chain_id = receive_transaction_data.network_chain_id;
      uint8_t data[1 + sizeof(receive_transaction_data.address) +
                   1];    // confirm byte + address length + null byte
      size_t datalen;
      data[0] = 1;    // confirmation byte
      if (chain_id != HARMONY_MAINNET_CHAIN) {
        memcpy(data + 1,
               receive_transaction_data.eth_pubkeyhash,
               sizeof(receive_transaction_data.eth_pubkeyhash));
        datalen = 1 + sizeof(receive_transaction_data.eth_pubkeyhash);
      } else {
        strncpy((char *)data + 1,
                receive_transaction_data.address,
                sizeof(data) - 1);
        datalen = strnlen((char *)data,
                          sizeof(data));    // send excluding the null byte
      }
      transmit_data_to_app(RECV_TXN_USER_VERIFIED_ADDRESS, data, datalen);
      reset_flow_level();
    } break;

    default:
      break;
  }

  return;
}
