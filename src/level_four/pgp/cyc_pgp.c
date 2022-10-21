/**
 * @file    cyc_sign_data.c
 * @author  Cypherock X1 Team
 * @brief   Sign data controller.
 *          Handles post event (only next events) operations for sign data.
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
#include "application_startup.h"
#include "controller_main.h"
#include "controller_tap_cards.h"
#include "cy_pgp.h"
#include "nist256p1.h"
#include "shamir_wrapper.h"
#include "ui_instruction.h"

static uint8_t chosen_wallet_index = 0;
static En_command_type_t response_cmd_type;
static uint8_t *request = NULL;
static uint8_t request_size = 0;
static uint8_t response[65] = {0};
static uint8_t response_size = 0;
static HDNode node = {0};

void cyc_pgp() {
  switch (flow_level.level_three) {
    case LEVEL_THREE_PGP_INIT:
      response_cmd_type = 0;
      response_size = 0;
      request = NULL;
      request_size = 0;
      chosen_wallet_index = 0;
      memzero(response, sizeof(response));
      transmit_one_byte_confirm(PGP_REQUEST);
      mark_device_state(CY_APP_IDLE | CY_APP_IDLE_TASK, 0xFF);
      flow_level.level_three = LEVEL_THREE_PGP_CHOOSE_WALLET;
      break;

    case LEVEL_THREE_PGP_CHOOSE_WALLET:
      chosen_wallet_index = flow_level.screen_input.list_choice - 1;
      if (get_ith_wallet_to_export(flow_level.screen_input.list_choice - 1,
                                   &chosen_wallet_index) != SUCCESS_) {
        LOG_ERROR("ERR: xx11");
        comm_reject_request(response_cmd_type, 8);
        reset_flow_level();
        mark_error_screen(ui_text_something_went_wrong);
        flow_level.show_error_screen = true;
        break;
      }
      memcpy(
          wallet.wallet_name, get_wallet_name(chosen_wallet_index), NAME_SIZE);
      wallet.wallet_info = get_wallet_info(chosen_wallet_index);
      memcpy(
          wallet.wallet_id, get_wallet_id(chosen_wallet_index), WALLET_ID_SIZE);

      if (WALLET_IS_PASSPHRASE_SET(wallet.wallet_info)) {
        flow_level.level_three = LEVEL_THREE_PGP_ENTER_PASSPHRASE;
      } else {
        flow_level.level_three = LEVEL_THREE_PGP_CHECK_PIN;
      }
      break;

    case LEVEL_THREE_PGP_ENTER_PASSPHRASE:
      flow_level.level_three = LEVEL_THREE_PGP_CONFIRM_PASSPHRASE;
      break;

    case LEVEL_THREE_PGP_CONFIRM_PASSPHRASE:
      snprintf(wallet_credential_data.passphrase,
               sizeof(wallet_credential_data.passphrase),
               "%s",
               flow_level.screen_input.input_text);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = LEVEL_THREE_PGP_CHECK_PIN;
      break;

    case LEVEL_THREE_PGP_CHECK_PIN:
      if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
        flow_level.level_three = LEVEL_THREE_PGP_ENTER_PIN;
      } else {
        flow_level.level_three = LEVEL_THREE_PGP_TAP_CARD;
      }
      break;

    case LEVEL_THREE_PGP_ENTER_PIN:
      sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                 strlen(flow_level.screen_input.input_text),
                 wallet_credential_data.password_single_hash);
      sha256_Raw(wallet_credential_data.password_single_hash,
                 SHA256_DIGEST_LENGTH,
                 wallet.password_double_hash);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = LEVEL_THREE_PGP_TAP_CARD;
      break;

    case LEVEL_THREE_PGP_TAP_CARD:
      tap_card_data.desktop_control = true;
      tap_threshold_cards_for_reconstruction_flow_controller(1);
      break;

    case LEVEL_THREE_PGP_DERIVE_HDNODE: {
      uint8_t seed[64] = {0};
      wallet_shamir_data.share_x_coords[1] = 5;
      get_flash_wallet_share_by_name((const char *)wallet.wallet_name,
                                     wallet_shamir_data.mnemonic_shares[1]);
      memcpy(wallet_shamir_data.share_encryption_data[1],
             wallet_shamir_data.share_encryption_data[0],
             NONCE_SIZE + WALLET_MAC_SIZE);
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
      mnemonic_to_seed(mnemo, wallet_credential_data.passphrase, seed, NULL);
      hdnode_from_seed(seed, 64, NIST256P1_NAME, &node);
      hdnode_private_ckd(&node, 0x8000002C);
      hdnode_private_ckd(&node, 0x80000000);
      hdnode_private_ckd(&node, 0x80000000);
      hdnode_private_ckd(&node, 0x00000000);
      memzero(seed, sizeof(seed));
      memzero(secret, sizeof(secret));
      memzero((void *)mnemo, strlen(mnemo));
      flow_level.level_three = LEVEL_THREE_PGP_IDENTIFY_REQUEST;
    } break;

    case LEVEL_THREE_PGP_IDENTIFY_REQUEST: {
      uint8_t *data_array = NULL;
      uint16_t msg_size = 0;
      En_command_type_t cmd_type;
      if (!get_usb_msg(&cmd_type, &data_array, &msg_size))
        return;
      if (cmd_type == PGP_PUBKEY)
        flow_level.level_three = LEVEL_THREE_PGP_PUBKEY;
      else if (cmd_type == PGP_ECDH_SESSION)
        flow_level.level_three = LEVEL_THREE_PGP_ECDH;
      else if (cmd_type == PGP_SIGNATURE)
        flow_level.level_three = LEVEL_THREE_PGP_SIGN;
      else {
        comm_reject_invalid_cmd();
        clear_message_received_data();
        return;
      }
      if (msg_size > 128) {
        reset_flow_level();
        mark_error_screen("Request too large");
        comm_reject_request(response_cmd_type, 1);
      } else {
        hdnode_private_ckd(&node, U32_READ_BE_ARRAY(data_array));
        hdnode_fill_public_key(&node);
      }
      request = data_array;
      request_size = msg_size;
      response_cmd_type = cmd_type;
    } break;

    case LEVEL_THREE_PGP_PUBKEY:
      if (request_size < 4) {
        reset_flow_level();
        mark_error_screen("Invalid payload length");
        transmit_one_byte(response_cmd_type, 2);
        return;
      }
      memcpy(response, node.public_key, sizeof(node.public_key));
      response_size = sizeof(node.public_key);
      flow_level.level_three = LEVEL_THREE_PGP_FINISH;
      break;

    case LEVEL_THREE_PGP_ECDH:
      response_size = 65;
      if (request_size < 37) {
        reset_flow_level();
        mark_error_screen("Invalid payload length");
        transmit_one_byte(response_cmd_type, 2);
        return;
      }
      ecdh_multiply(&nist256p1, node.private_key, request + 4, response);
      flow_level.level_three = LEVEL_THREE_PGP_FINISH;
      break;

    case LEVEL_THREE_PGP_SIGN:
      response[0] = 0;
      response_size = 65;
      if (request_size < 36) {
        reset_flow_level();
        mark_error_screen("Invalid payload length");
        transmit_one_byte(response_cmd_type, 2);
        return;
      }
      ecdsa_sign_digest(get_curve_by_name(NIST256P1_NAME)->params,
                        node.private_key,
                        request + 4,
                        response + 1,
                        NULL,
                        NULL);
      flow_level.level_three = LEVEL_THREE_PGP_FINISH;
      break;

    case LEVEL_THREE_PGP_FINISH:
      memset(&node, 0, sizeof(node));
      transmit_data_to_app(response_cmd_type, response, response_size);
      memset(response, 0, sizeof(response));
      reset_flow_level();
      break;

    default:
      break;
  }
}