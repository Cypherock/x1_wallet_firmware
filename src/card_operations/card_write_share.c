/**
 * @file    write_card_share.c
 * @author  Cypherock X1 Team
 * @brief   Source file supporting writing of wallet share on the X1 card
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
#include "card_internal.h"
#include "card_operation_typedefs.h"
#include "card_utils.h"
#include "flash_api.h"
#include "nfc.h"
#include "ui_instruction.h"
#include "wallet.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/
extern Wallet_shamir_data wallet_shamir_data;

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
 * @brief Handles pre-processing required during the wallet share write
 * operation on the X1 card.
 *
 * @param card_num The card number to which the share is written
 */
static void write_card_pre_process(uint8_t card_num);

/**
 * @brief The function records a card write attempt on the flash memory and
 * updates the state of the flash wallet accordingly.
 *
 * @param card_num represents the number of the card on which write operation is
 * being attempted.
 */
static void record_card_write_attempt_on_flash(uint8_t card_num);

/**
 * @brief Handles post-processing required during the wallet share write
 * operation on the X1 card.
 *
 * @param card_num The card number to which the share is written
 */
static void write_card_share_post_process(uint8_t card_num);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static uint32_t wallet_index;    // What's the index of the wallet in flash to
                                 // which we are talking to

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
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
         PADDED_NONCE_SIZE + WALLET_MAC_SIZE);
  return;
}

static void record_card_write_attempt_on_flash(uint8_t card_num) {
  Flash_Wallet *wallet_for_flash = get_flash_wallet();
  uint8_t attempt_card_state = encode_card_number(card_num) << 4;

  if (attempt_card_state ==
      (attempt_card_state & wallet_for_flash->cards_states)) {
    // Attempt card state already recorded on flash, no need to write to flash
    // again. Reached here probably due to retry attempt on same card.
    return;
  }

  wallet_for_flash->cards_states |= attempt_card_state;
  if (card_num == 1) {
    wallet_for_flash->state = UNVERIFIED_VALID_WALLET;
    add_wallet_to_flash(wallet_for_flash, &wallet_index);
  } else {
    put_wallet_flash(wallet_index, wallet_for_flash);
  }
  return;
}

static void write_card_share_post_process(uint8_t card_num) {
  Flash_Wallet *wallet_for_flash = get_flash_wallet();

  wallet_for_flash->cards_states &= 0x0F;
  wallet_for_flash->cards_states |= encode_card_number(card_num);

  put_wallet_flash(wallet_index, wallet_for_flash);

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
  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
bool write_card_share(uint8_t card_num, const char *heading, const char *msg) {
  bool result = false;
  wallet.xcor = card_num;

  // Render the instruction screen
  instruction_scr_init(msg, heading);

  card_operation_data_t card_data = {0};
  card_data.nfc_data.retries = 5;
  card_data.nfc_data.init_session_keys = true;
  memcpy(card_data.nfc_data.family_id, get_family_id(), FAMILY_ID_SIZE);

  write_card_pre_process(card_num);

  while (1) {
    card_data.nfc_data.acceptable_cards = 1 << (card_num - 1);
    if (card_num == 1)
      card_data.nfc_data.tapped_card = 0;

    card_initialize_applet(&card_data);

    if (CARD_OPERATION_SUCCESS == card_data.error_type) {
      record_card_write_attempt_on_flash(card_num);
      card_data.nfc_data.status = nfc_add_wallet(&wallet);

      if (card_data.nfc_data.status == SW_NO_ERROR) {
        write_card_share_post_process(card_num);
        result = true;
        break;
      } else {
        card_handle_errors(&card_data);
      }
    }

    if (CARD_OPERATION_CARD_REMOVED == card_data.error_type ||
        CARD_OPERATION_RETAP_BY_USER_REQUIRED == card_data.error_type) {
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
