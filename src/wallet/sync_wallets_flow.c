/**
 * @file    sync_wallets_flow.c
 * @author  Cypherock X1 Team
 * @brief   Flow to sync X1 Vault with a wallet present on X1 Card
 *
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
#include "sync_wallets_flow.h"

#include "card_flow_reconstruct_wallet.h"
#include "card_operations.h"
#include "constant_texts.h"
#include "flash_api.h"
#include "settings_api.h"
#include "shamir_wrapper.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "ui_state_machine.h"
#include "utils.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/
// TODO: Add these pointers in a common header file
extern char *ALPHABET;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;
extern char *PASSPHRASE;

// TODO: Remove usage of global variables
extern Wallet_shamir_data wallet_shamir_data;
extern Wallet_credential_data wallet_credential_data;
extern Wallet wallet;
extern Flash_Wallet wallet_for_flash;

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static sync_state_e sync_wallet_handler(sync_state_e state) {
  sync_state_e next_state = SYNC_EXIT;
  switch (state) {
    case SYNC_PIN_INPUT: {
      if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
        next_state = SYNC_TAP_CARD_FLOW;
        break;
      }

      input_text_init(
          ALPHA_NUMERIC, 26, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);
      next_state = get_state_on_input_scr(
          SYNC_PIN_INPUT, SYNC_EARLY_EXIT, SYNC_TIMED_OUT);

      if (SYNC_PIN_INPUT == next_state) {
        sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                   strnlen(flow_level.screen_input.input_text,
                           sizeof(flow_level.screen_input.input_text)),
                   wallet_credential_data.password_single_hash);
        sha256_Raw(wallet_credential_data.password_single_hash,
                   SHA256_DIGEST_LENGTH,
                   wallet.password_double_hash);
        next_state = SYNC_TAP_CARD_FLOW;
      }

      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      break;
    }

    case SYNC_TAP_CARD_FLOW: {
      card_error_type_e card_status = card_flow_reconstruct_wallet(2, NULL);

      if (CARD_OPERATION_SUCCESS == card_status) {
        next_state = SYNC_RECONSTRUCT_SEED;
      } else if (CARD_OPERATION_INCORRECT_PIN_ENTERED == card_status) {
        next_state = SYNC_PIN_INPUT;
      } else {
        /* In case of other status code returned by the card operation:
         * CARD_OPERATION_LOCKED_WALLET,
         * CARD_OPERATION_P0_OCCURED,
         * CARD_OPERATION_ABORT_OPERATION
         * These error codes are non-recoverable from this flow. Error message
         * will be displayed to the user before the main menu
         */
        next_state = SYNC_COMPLETED_WITH_ERRORS;
      }

      break;
    }

    case SYNC_RECONSTRUCT_SEED: {
      delay_scr_init(ui_text_processing, DELAY_TIME);

      uint8_t temp_password_hash[SHA256_DIGEST_LENGTH] = {0};
      uint8_t wallet_nonce[PADDED_NONCE_SIZE] = {0};

      memcpy(wallet_nonce,
             wallet_shamir_data.share_encryption_data[0],
             PADDED_NONCE_SIZE);

      if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
        memcpy(temp_password_hash,
               wallet_credential_data.password_single_hash,
               SHA256_DIGEST_LENGTH);
        decrypt_shares();
      }

      recover_share_from_shares(BLOCK_SIZE,
                                MINIMUM_NO_OF_SHARES,
                                wallet_shamir_data.mnemonic_shares,
                                wallet_shamir_data.share_x_coords,
                                wallet_shamir_data.mnemonic_shares[4],
                                5);

      if (WALLET_IS_PIN_SET(wallet.wallet_info)) {
        memcpy(wallet_shamir_data.share_encryption_data[4],
               wallet_nonce,
               PADDED_NONCE_SIZE);
        memcpy(wallet_credential_data.password_single_hash,
               temp_password_hash,
               SHA256_DIGEST_LENGTH);
        memzero(temp_password_hash, SHA256_DIGEST_LENGTH);
        encrypt_shares();
        memzero(wallet_credential_data.password_single_hash,
                sizeof(wallet_credential_data.password_single_hash));
      }

      uint32_t wallet_index;
      Flash_Wallet *flash_wallet;
      get_index_by_name((const char *)wallet.wallet_name,
                        (uint8_t *)(&wallet_index));
      get_flash_wallet_by_name((const char *)wallet.wallet_name, &flash_wallet);
      memcpy(&wallet_for_flash, flash_wallet, sizeof(Flash_Wallet));
      put_wallet_share_sec_flash(
          wallet_index, wallet_shamir_data.mnemonic_shares[4], wallet_nonce);

      next_state = SYNC_COMPLETED;
      break;
    }

    // TODO: Manage states better to indicate error when failure occurs
    case SYNC_COMPLETED:
    case SYNC_COMPLETED_WITH_ERRORS:
    case SYNC_TIMED_OUT:
    case SYNC_EARLY_EXIT:
    case SYNC_EXIT:
    default:
      break;
  }

  return next_state;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
sync_state_e sync_wallets_flow(const uint8_t *wallet_id) {
  clear_wallet_data();

  uint8_t index = MAX_WALLETS_ALLOWED;
  if (SUCCESS_ != get_first_matching_index_by_id(wallet_id, &index) ||
      MAX_WALLETS_ALLOWED <= index) {
    return SYNC_EARLY_EXIT;
  }

  // Populate wallet structure
  memcpy(wallet.wallet_id, get_wallet_id(index), WALLET_ID_SIZE);
  memcpy(wallet.wallet_name, get_wallet_name(index), NAME_SIZE);
  wallet.wallet_info = get_wallet_info(index);

  sync_state_e current_state = SYNC_PIN_INPUT;
  // Generate wallet share by perform 2 card tap flow
  while (1) {
    sync_state_e next_state = sync_wallet_handler(current_state);
    current_state = next_state;

    if (SYNC_COMPLETED <= next_state) {
      break;
    }
  }

  clear_wallet_data();

  return current_state;
}
