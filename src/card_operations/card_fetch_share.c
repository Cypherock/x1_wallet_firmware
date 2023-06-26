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
 * @brief Helper function that copies wallet share retrieved from X1 card onto
 * the RAM
 *
 * @param xcor The x-coordinate of the wallet share
 */
static void _handle_retrieve_wallet_success(uint8_t xcor);

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
static void _handle_retrieve_wallet_success(uint8_t xcor) {
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
         NONCE_SIZE + WALLET_MAC_SIZE);
  memzero(wallet.arbitrary_data_share, sizeof(wallet.arbitrary_data_share));
  memzero(wallet.wallet_share_with_mac_and_nonce,
          sizeof(wallet.wallet_share_with_mac_and_nonce));

  wallet_shamir_data.share_x_coords[xcor] = wallet.xcor;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void tap_threshold_cards_for_reconstruction_flow_controller(uint8_t threshold) {
}

card_error_type_e card_fetch_share(card_fetch_share_cfg_t *config) {
  ASSERT(NULL != config);

  card_operation_data_t card_data = {0};
  card_error_type_e result = CARD_OPERATION_DEFAULT_INVALID;
  card_data.nfc_data.retries = 5;
  card_data.nfc_data.init_session_keys = true;

  instruction_scr_init(config->message, config->heading);
  while (1) {
    card_data.nfc_data.acceptable_cards = config->remaining_cards;
    memcpy(card_data.nfc_data.family_id, get_family_id(), FAMILY_ID_SIZE);

    card_initialize_applet(&card_data);

    if (CARD_OPERATION_SUCCESS == card_data.error_type) {
      card_data.nfc_data.status = nfc_retrieve_wallet(&wallet);

      if (card_data.nfc_data.status == SW_NO_ERROR) {
        remaining_cards = card_data.nfc_data.acceptable_cards;
        _handle_retrieve_wallet_success(config->xcor);
        buzzer_start(BUZZER_DURATION);
        if (false == config->skip_card_removal) {
          wait_for_card_removal();
        }

        result = CARD_OPERATION_SUCCESS;
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
        instruction_scr_init(config->message, config->heading);
        continue;
      }
    }

    /* TODO: Consider moving incorrect pin error handling to caller */
    if (CARD_OPERATION_INCORRECT_PIN_ENTERED == card_data.error_type) {
      card_error_type_e temp_error =
          indicate_wrong_pin(card_data.nfc_data.status);
      if (CARD_OPERATION_SUCCESS != temp_error) {
        result = temp_error;
        break;
      }
    }

    // If control reached here, it is an unrecoverable error, so break
    result = card_data.error_type;
    break;
  }

  nfc_deselect_card();
  LOG_ERROR("Card Error type: %d", card_data.error_type);
  return result;
}
