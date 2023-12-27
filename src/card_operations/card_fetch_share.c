/**
 * @file    card_fetch_share.c
 * @author  Cypherock X1 Team
 * @brief   Implements card operation to handle fetching of wallet share from X1
 *          card
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
#include "card_fetch_share.h"

#include "card_internal.h"
#include "card_utils.h"
#include "constant_texts.h"
#include "core_error.h"
#include "flash_api.h"
#include "nfc.h"
#include "ui_screens.h"

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
 * The function `verify_fetched_wallet` compares the values of a `wallet` object
 * with the values of a `flash_wallet` object and returns a boolean indicating
 * whether they are equal.
 *
 * @param xcor The x-coordinate of the wallet share, this variable represents
 * the share index fetched from the last tapped card.
 *
 * @return a boolean value, which indicates whether the fetched wallet matches
 * the expected values.
 */
static bool verify_fetched_wallet(uint8_t xcor);

/**
 * @brief Helper function that copies wallet share retrieved from X1 card onto
 * the RAM
 *
 * @param xcor The x-coordinate of the wallet share, this variable represents
 * the share index fetched from the last tapped card.
 */
static bool _handle_retrieve_wallet_success(uint8_t xcor);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static uint8_t remaining_cards;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool verify_fetched_wallet(uint8_t xcor) {
  Flash_Wallet *flash_wallet = NULL;
  bool status =
      get_flash_wallet_by_name((const char *)wallet.wallet_name, &flash_wallet);

  ASSERT(SUCCESS == status && NULL != flash_wallet);

  bool compare_status =
      (0 == memcmp(wallet.wallet_id, flash_wallet->wallet_id, WALLET_ID_SIZE));
  compare_status &=
      (0 == memcmp(wallet.wallet_name, flash_wallet->wallet_name, NAME_SIZE));
  compare_status &= (wallet.wallet_info == flash_wallet->wallet_info);

  /**
   * For wallets with device share present on flash, fetched wallet nonce is
   * compared with wallet nonce on flash. In case of sync wallet, the wallet
   * share and encryption data is written on flash after verification. For
   * verifying the wallets in this case, we compare wallet nonce fetched from
   * the two cards.
   */
  if (VALID_WALLET_WITHOUT_DEVICE_SHARE != flash_wallet->state) {
    // Wallet nonce present on flash, so compare current wallet nonce with flash
    // wallet nonce.
    uint8_t wallet_nonce[NONCE_SIZE] = {0};
    ASSERT(SUCCESS ==
           get_flash_wallet_nonce_by_name(
               (const char *)flash_wallet->wallet_name, wallet_nonce));
    compare_status &=
        (0 == memcmp(wallet.wallet_share_with_mac_and_nonce + BLOCK_SIZE,
                     wallet_nonce,
                     NONCE_SIZE));
  } else if (0 < xcor) {
    // Compare nonce from current wallet with nonce of wallet previously
    // fetched.
    compare_status &=
        (0 == memcmp(wallet.wallet_share_with_mac_and_nonce + BLOCK_SIZE,
                     wallet_shamir_data.share_encryption_data[xcor - 1],
                     NONCE_SIZE));
  }
  return compare_status;
}

static bool _handle_retrieve_wallet_success(uint8_t xcor) {
  if (!verify_fetched_wallet(xcor)) {
    LOG_ERROR("Verification failed xxx39");
    return false;
  }

  if (WALLET_IS_ARBITRARY_DATA(wallet.wallet_info)) {
    memcpy(((uint8_t *)wallet_shamir_data.arbitrary_data_shares) +
               xcor * wallet.arbitrary_data_size,
           wallet.arbitrary_data_share,
           wallet.arbitrary_data_size);
  } else {
    memcpy(wallet_shamir_data.mnemonic_shares[xcor],
           wallet.wallet_share_with_mac_and_nonce,
           BLOCK_SIZE);
  }

  memcpy(wallet_shamir_data.share_encryption_data[xcor],
         wallet.wallet_share_with_mac_and_nonce + BLOCK_SIZE,
         PADDED_NONCE_SIZE + WALLET_MAC_SIZE);
  memzero(wallet.arbitrary_data_share, sizeof(wallet.arbitrary_data_share));
  memzero(wallet.wallet_share_with_mac_and_nonce,
          sizeof(wallet.wallet_share_with_mac_and_nonce));

  wallet_shamir_data.share_x_coords[xcor] = wallet.xcor;

  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void tap_threshold_cards_for_reconstruction_flow_controller(uint8_t threshold) {
}

card_error_type_e card_fetch_share(const card_fetch_share_config_t *config,
                                   card_fetch_share_response_t *response) {
  card_error_type_e result = CARD_OPERATION_DEFAULT_INVALID;

  // X-Coordinate cannot be more than the number of shares
  if (NULL == config || TOTAL_NUMBER_OF_SHARES <= config->xcor ||
      NULL == config->operation.expected_family_id) {
    return result;
  }

  card_operation_data_t card_data = {0};
  card_data.nfc_data.retries = 5;
  card_data.nfc_data.init_session_keys = true;

  instruction_scr_init(config->frontend.msg, config->frontend.heading);

  while (1) {
    card_data.nfc_data.acceptable_cards = config->operation.acceptable_cards;
    memcpy(card_data.nfc_data.family_id,
           config->operation.expected_family_id,
           FAMILY_ID_SIZE);

    card_initialize_applet(&card_data);

    if (CARD_OPERATION_SUCCESS == card_data.error_type) {
      card_data.nfc_data.status = nfc_retrieve_wallet(&wallet);

      if (card_data.nfc_data.status == SW_NO_ERROR) {
        remaining_cards = card_data.nfc_data.acceptable_cards;
        if (!_handle_retrieve_wallet_success(config->xcor)) {
          result = card_data.error_type = CARD_OPERATION_VERIFICATION_FAILED;
          card_data.nfc_data.status = SW_RECORD_NOT_FOUND;
          mark_core_error_screen(
              ui_text_wallet_verification_failed_in_reconstruction, true);
          break;
        }

        if (config->operation.buzzer_on_success) {
          buzzer_start(BUZZER_DURATION);
        }

        if (false == config->operation.skip_card_removal) {
          wait_for_card_removal();
        }
        result = CARD_OPERATION_SUCCESS;
        break;
      } else {
        card_handle_errors(&card_data);
      }
    }

    if (CARD_OPERATION_CARD_REMOVED == card_data.error_type ||
        CARD_OPERATION_RETAP_BY_USER_REQUIRED == card_data.error_type) {
      const char *error_msg = card_data.error_message;

      /**
       * In case the same card as before is tapped, the user should be told to
       * tap a different card instead of the default message "Wrong card
       * sequence"
       */
      if (SW_CONDITIONS_NOT_SATISFIED == card_data.nfc_data.status) {
        error_msg = config->frontend.unexpected_card_error;
      }

      if (CARD_OPERATION_SUCCESS == indicate_card_error(error_msg)) {
        // Re-render the instruction screen
        instruction_scr_init(config->frontend.msg, config->frontend.heading);
        continue;
      }
    }

    result = handle_wallet_errors(&card_data, &wallet);
    if (CARD_OPERATION_SUCCESS != result) {
      break;
    }

    // If control reached here, it is an unrecoverable error, so break
    result = card_data.error_type;
    break;
  }

  if (response->card_info.tapped_family_id) {
    memcpy(card_data.nfc_data.family_id,
           config->operation.expected_family_id,
           FAMILY_ID_SIZE);
  }
  response->card_info.pairing_error = card_data.nfc_data.pairing_error;
  response->card_info.tapped_card = card_data.nfc_data.tapped_card;
  response->card_info.recovery_mode = card_data.nfc_data.recovery_mode;
  response->card_info.status = card_data.nfc_data.status;

  nfc_deselect_card();
  return result;
}
