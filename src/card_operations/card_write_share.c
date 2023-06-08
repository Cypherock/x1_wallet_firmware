/**
 * @file    write_to_cards_controller.c
 * @author  Cypherock X1 Team
 * @brief   Write to cards controller.
 *          This file contains the functions to write wallet to cards
 *sequentially.
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
#include "card_write_share.h"

#include "card_action_controllers.h"
#include "card_internal.h"
#include "card_utils.h"
#include "controller_main.h"
#include "controller_tap_cards.h"
#include "flash_api.h"
#include "nfc.h"
#include "shamir_wrapper.h"
#include "stdint.h"
#include "tasks.h"
#include "ui_instruction.h"
#include "wallet.h"

extern Wallet_shamir_data wallet_shamir_data;

static uint32_t wallet_index;    // What's the index of the wallet in flash to
                                 // which we are talking to

static void write_share_to_card(uint8_t card_number);

void tap_cards_for_write_and_verify_flow_controller() {
  switch (flow_level.level_four) {
    case CARD_ONE_FRONTEND:
      tap_card_data.tapped_card = 0;
      tap_card_data.desktop_control = false;
      flow_level.level_four = CARD_ONE_WRITE;
      break;

    case CARD_ONE_WRITE:
      write_share_to_card(1);
      break;

    case CARD_ONE_READBACK:
      readback_share_from_card(0);
      break;

    case CARD_TWO_FRONTEND:
      flow_level.level_four = CARD_TWO_WRITE;
      break;

    case CARD_TWO_WRITE:
      write_share_to_card(2);
      break;

    case CARD_TWO_READBACK:
      readback_share_from_card(1);
      break;

    case CARD_THREE_FRONTEND:
      flow_level.level_four = CARD_THREE_WRITE;
      break;

    case CARD_THREE_WRITE:
      write_share_to_card(3);
      break;

    case CARD_THREE_READBACK:
      readback_share_from_card(2);
      break;

    case CARD_FOUR_FRONTEND:
      flow_level.level_four = CARD_FOUR_WRITE;
      break;

    case CARD_FOUR_WRITE:
      write_share_to_card(4);
      break;

    case CARD_FOUR_READBACK:
      readback_share_from_card(3);
      break;

    default:
      reset_flow_level();
      // message_scr_init(ui_text_something_went_wrong);
      break;
  }
}

/**
 * @brief Handles pre-processing required during the wallet share write
 * operation on the X1 card.
 *
 * @param card_num The card number to which the share is written
 */
static void write_card_pre_process(uint8_t card_num);

/**
 * @brief Handles post-processing required during the wallet share write
 * operation on the X1 card.
 *
 * @param card_num The card number to which the share is written
 */
static void write_card_share_post_process(uint8_t card_num);

static void write_card_pre_process(uint8_t card_num) {
  if (WALLET_IS_ARBITRARY_DATA(wallet.wallet_info))
    memcpy(wallet.arbitrary_data_share,
           ((uint8_t *)wallet_shamir_data.arbitrary_data_shares) +
               (card_num - 1) * wallet.arbitrary_data_size,
           wallet.arbitrary_data_size);
  else
    memcpy(wallet.wallet_share_with_mac_and_nonce,
           wallet_shamir_data.mnemonic_shares[card_num - 1],
           BLOCK_SIZE);
  memcpy(wallet.wallet_share_with_mac_and_nonce + BLOCK_SIZE,
         wallet_shamir_data.share_encryption_data[card_num - 1],
         NONCE_SIZE + WALLET_MAC_SIZE);
  return;
}

static void write_card_share_post_process(uint8_t card_num) {
  Flash_Wallet *wallet_for_flash = get_flash_wallet();
  wallet_for_flash->cards_states = (1 << card_num) - 1;

  if (card_num == 1) {
    wallet_for_flash->state = UNVERIFIED_VALID_WALLET;
    add_wallet_to_flash(wallet_for_flash, &wallet_index);
  } else {
    put_wallet_flash(wallet_index, wallet_for_flash);
  }

  if (WALLET_IS_ARBITRARY_DATA(wallet.wallet_info))
    memset(((uint8_t *)wallet_shamir_data.arbitrary_data_shares) +
               (card_num - 1) * wallet.arbitrary_data_size,
           0,
           wallet.arbitrary_data_size);
  else
    memset(wallet_shamir_data.mnemonic_shares[card_num - 1], 0, BLOCK_SIZE);
  memset(wallet_shamir_data.share_encryption_data[card_num - 1],
         0,
         sizeof(wallet_shamir_data.share_encryption_data[card_num - 1]));
  memset(((uint8_t *)wallet_shamir_data.arbitrary_data_shares) +
             (card_num - 1) * wallet.arbitrary_data_size,
         0,
         wallet.arbitrary_data_size);
}

// TODO: This API is deprecated
static void write_share_to_card(uint8_t card_number) {
}

bool write_card_share(uint8_t card_num, const char *heading, const char *msg) {
  bool result = false;
  wallet.xcor = card_num;

  // Render the instruction screen
  instruction_scr_init(msg, heading);

  card_operation_data_t card_data = {0};
  card_data.nfc_data.retries = 5;
  memcpy(card_data.nfc_data.family_id, get_family_id(), FAMILY_ID_SIZE);

  write_card_pre_process(card_num);

  while (1) {
    card_data.nfc_data.acceptable_cards = 1 << (card_num - 1);
    if (card_num == 1)
      card_data.nfc_data.tapped_card = 0;

    card_initialize_applet(&card_data);

    if (CARD_OPERATION_SUCCESS == card_data.error_type) {
      load_card_session_key(card_data.nfc_data.card_key_id);
      card_data.nfc_data.status = nfc_add_wallet(&wallet);

      if (card_data.nfc_data.status == SW_NO_ERROR) {
        write_card_share_post_process(card_num);
        result = true;
        break;
      } else {
        card_handle_errors(&card_data);
      }
    }

    if ((CARD_OPERATION_CARD_REMOVED == card_data.error_type) ||
        (CARD_OPERATION_RETAP_BY_USER_REQUIRED == card_data.error_type)) {
      const char *error_msg = card_data.error_message;
      if (CARD_OPERATION_SUCCESS == indicate_card_error(error_msg)) {
        // Re-render the instruction screen
        instruction_scr_init(msg, heading);
        continue;
      }
    }

    // If control reached here, it is an unrecoverable error, so break
    result = false;
    break;
  }

  nfc_deselect_card();
  return result;
}
