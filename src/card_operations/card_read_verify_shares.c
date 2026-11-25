/**
 * @file    card_read_verify_shares.c
 * @author  Cypherock X1 Team
 * @brief   Source file supporting reading of wallet share from the X1 card for
 *          wallet verification
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
#include "bip39.h"
#include "card_internal.h"
#include "card_operation_typedefs.h"
#include "card_read_verify_share.h"
#include "card_utils.h"
#include "core_error.h"
#include "flash_api.h"
#include "nfc.h"
#include "options.h"
#include "shamir_wrapper.h"
#include "ui_instruction.h"
#include "wallet.h"
#include "wallet_utilities.h"

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
/**
 * @brief Handles post-processing required during the wallet share read
 * operation on the X1 card.
 *
 * @param xcor The x-coordinate of the share contained in the X1 card
 */
static void read_card_share_post_process(uint8_t xcor);

/**
 * @brief Checks that the wallet read from the card is the requested wallet and
 * that its share belongs to the tapped card.
 *
 * @param expected_name Name of the requested wallet
 * @param xcor The index of the card that was read (card number - 1)
 * @param tapped_card The encoded number of the tapped card
 */
static bool verify_read_wallet(const uint8_t *expected_name,
                               uint8_t xcor,
                               uint8_t tapped_card);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void read_card_share_post_process(uint8_t xcor) {
  if (WALLET_IS_ARBITRARY_DATA(wallet.wallet_info))
    memcpy(((uint8_t *)wallet_shamir_data.arbitrary_data_shares) +
               xcor * wallet.arbitrary_data_size,
           wallet.arbitrary_data_share,
           wallet.arbitrary_data_size);
  else
    memcpy(wallet_shamir_data.mnemonic_shares[xcor],
           wallet.wallet_share_with_mac_and_nonce,
           BLOCK_SIZE);
  memcpy(wallet_shamir_data.share_encryption_data[xcor],
         wallet.wallet_share_with_mac_and_nonce + BLOCK_SIZE,
         PADDED_NONCE_SIZE + WALLET_MAC_SIZE);
  memzero(wallet.arbitrary_data_share, sizeof(wallet.arbitrary_data_share));
  memzero(wallet.wallet_share_with_mac_and_nonce,
          sizeof(wallet.wallet_share_with_mac_and_nonce));

  wallet_shamir_data.share_x_coords[xcor] = wallet.xcor;
  return;
}

static bool verify_read_wallet(const uint8_t *expected_name,
                               uint8_t xcor,
                               uint8_t tapped_card) {
  Flash_Wallet *flash_wallet = NULL;
  if (SUCCESS != get_flash_wallet_by_name((const char *)expected_name,
                                          &flash_wallet) ||
      NULL == flash_wallet) {
    return false;
  }

  // the shares are later checked against this wallet id, so it must come
  // from flash rather than from the card
  bool status =
      (0 == memcmp(wallet.wallet_name, flash_wallet->wallet_name, NAME_SIZE));
  status &=
      (0 == memcmp(wallet.wallet_id, flash_wallet->wallet_id, WALLET_ID_SIZE));
  status &= (wallet.wallet_info == flash_wallet->wallet_info);
  // the share x-coordinate is the number of the card it was written to
  status &= (wallet.xcor == xcor + 1);
  status &= (encode_card_number(wallet.xcor) == tapped_card);
  return status;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
bool read_card_share(uint8_t xcor, const char *heading, const char *msg) {
  bool result = false;

  card_operation_data_t card_data = {0};
  card_data.nfc_data.retries = 5;
  card_data.nfc_data.init_session_keys = true;

  // the card response overwrites the wallet, so keep the requested name
  uint8_t expected_name[NAME_SIZE] = {0};
  memcpy(expected_name, wallet.wallet_name, NAME_SIZE);

  while (1) {
    memcpy(card_data.nfc_data.family_id, get_family_id(), FAMILY_ID_SIZE);
    card_data.nfc_data.acceptable_cards = encode_card_number(xcor + 1);
    if (xcor == 0)
      card_data.nfc_data.tapped_card = 0;

    card_initialize_applet(&card_data);

    if (CARD_OPERATION_SUCCESS == card_data.error_type) {
      card_data.nfc_data.status = nfc_retrieve_wallet(&wallet);

      if (card_data.nfc_data.status == SW_NO_ERROR) {
        if (!verify_read_wallet(
                expected_name, xcor, card_data.nfc_data.tapped_card)) {
          memzero(wallet.arbitrary_data_share,
                  sizeof(wallet.arbitrary_data_share));
          memzero(wallet.wallet_share_with_mac_and_nonce,
                  sizeof(wallet.wallet_share_with_mac_and_nonce));
          mark_core_error_screen(
              ui_text_wallet_verification_failed_in_reconstruction, false);
          result = false;
          break;
        }

        buzzer_start(BUZZER_DURATION);
        if (xcor != 3) {
          wait_for_card_removal();
        }

        read_card_share_post_process(xcor);
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

    (void)handle_wallet_errors(&card_data, &wallet);

    // If control reached here, it is an unrecoverable error, so break
    result = false;
    break;
  }

  nfc_deselect_card();
  return result;
}

int verify_card_share_data() {
  uint8_t CONFIDENTIAL secret[MAX_ARBITRARY_DATA_SIZE];
  uint8_t status = 0;
  uint8_t wallet_id[WALLET_ID_SIZE] = {0};

  wallet_shamir_data.share_x_coords[4] = 5;
  // without the device share the check below can not pass
  bool have_shares = SUCCESS_ == get_flash_wallet_share_by_name(
                                     (const char *)wallet.wallet_name,
                                     wallet_shamir_data.mnemonic_shares[4]);

  if (have_shares && WALLET_IS_PIN_SET(wallet.wallet_info))
    have_shares = decrypt_shares();
  if (!have_shares) {
    status = 0;
    memzero(wallet_credential_data.password_single_hash,
            sizeof(wallet_credential_data.password_single_hash));
    memzero(wallet_shamir_data.share_encryption_data,
            sizeof(wallet_shamir_data.share_encryption_data));
    memzero(wallet_shamir_data.arbitrary_data_shares,
            sizeof(wallet_shamir_data.arbitrary_data_shares));
    memzero(wallet_shamir_data.mnemonic_shares,
            sizeof(wallet_shamir_data.mnemonic_shares));
  } else if (WALLET_IS_ARBITRARY_DATA(wallet.wallet_info)) {
    status = generate_data_5C2(wallet.arbitrary_data_size,
                               wallet_shamir_data.arbitrary_data_shares,
                               wallet_shamir_data.share_x_coords,
                               secret);
    memzero(wallet_shamir_data.arbitrary_data_shares,
            sizeof(wallet_shamir_data.arbitrary_data_shares));
  } else {
    status = generate_shares_5C2(wallet_shamir_data.mnemonic_shares,
                                 wallet_shamir_data.share_x_coords,
                                 secret);
    memzero(wallet_shamir_data.mnemonic_shares,
            sizeof(wallet_shamir_data.mnemonic_shares));
  }

  if (status == 1) {
    // verify wallet id only if secret successfully regenerated
    mnemonic_clear();
    const char *mnemo =
        mnemonic_from_data(secret, wallet.number_of_mnemonics * 4 / 3);
    ASSERT(mnemo != NULL);
    calculate_wallet_id(wallet_id, mnemo);
    status = memcmp(wallet.wallet_id, wallet_id, WALLET_ID_SIZE);
    LOG_INFO("xxx36: %d", status);
    status = (status == 0) ? 1 : 0;
    mnemonic_clear();
  }

  // both checks complete; accordingly update on flash (only the slot that
  // holds this wallet)
  uint8_t wallet_index = MAX_WALLETS_ALLOWED;
  if (SUCCESS_ ==
      get_index_by_name((const char *)wallet.wallet_name, &wallet_index)) {
    set_wallet_state(wallet_index, status == 1 ? VALID_WALLET : INVALID_WALLET);
  } else {
    status = 0;
  }

  memzero(secret, sizeof(secret));
  return status;
}
