/**
 * @file    sync_with_cards.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
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
#include "card_operations.h"
#include "constant_texts.h"
#include "core_error.h"
#include "flash_api.h"
#include "settings_api.h"
#include "sync_wallets_flow.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "utils.h"

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
 * @brief Get the wallets from X1 Card
 *
 * @param wallet_list Reference to buffer which will be filled with the wallet
 * list in tapped card
 * @param tapped_card Reference to buffer which will be filled with the encoded
 * tapped card details.
 * @return true If the operation completed without errors
 * @return false If any error was encountered
 */
static bool get_wallets_from_card(wallet_list_t *wallet_list,
                                  uint8_t *tapped_card);

/**
 * @brief Helper function to store the wallet data in the X1 vault flash memory.
 *
 * @param wallet_list The list of wallets present in an X1 card
 * @param tapped_card The encoded value of the tapped card
 * @return true If the flash write operation was performed successfully
 * @return false If the flash write operation returned some error
 */
static bool sync_wallets_in_flash(const wallet_list_t *wallet_list,
                                  const uint8_t *tapped_card);

/**
 * @brief Helper function to check if X1 Vault can be synced with the cards.
 * @details Checks if atleast 2 cards are paired and no wallet is in partial,
 * invalid or locked state. If the device state is not eligible for syncing an
 * appropriate error message is set
 *
 * @return true If the device state is eligible for syncing
 * @return false If the device state is not eligible for syncing
 */
bool sync_with_card_eligibility(void);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool get_wallets_from_card(wallet_list_t *wallet_list,
                                  uint8_t *tapped_card) {
  card_fetch_wallet_list_config_t configuration = {0};
  card_fetch_wallet_list_response_t response = {0};

  configuration.operation.acceptable_cards = ACCEPTABLE_CARDS_ALL;
  configuration.operation.skip_card_removal = false;
  configuration.operation.expected_family_id = get_family_id();

  configuration.frontend.heading = ui_text_tap_1_2_cards;
  configuration.frontend.msg = ui_text_place_card_below;

  response.wallet_list = wallet_list;
  response.card_info.tapped_card = 0;
  response.card_info.recovery_mode = 0;
  response.card_info.pairing_error = false;
  response.card_info.status = 0;
  response.card_info.tapped_family_id = NULL;

  if (CARD_OPERATION_SUCCESS !=
      card_fetch_wallet_list(&configuration, &response)) {
    return false;
  }

  *tapped_card = response.card_info.tapped_card;
  return true;
}

static bool sync_wallets_in_flash(const wallet_list_t *wallet_list,
                                  const uint8_t *tapped_card) {
  for (uint8_t index = 0; index < wallet_list->count; index++) {
    Flash_Wallet new_wallet = {0};

    new_wallet.state = VALID_WALLET_WITHOUT_DEVICE_SHARE;
    new_wallet.wallet_info = wallet_list->wallet[index].info;
    new_wallet.is_wallet_locked = wallet_list->wallet[index].locked;
    // assuming if person is syncing then all cards have share
    new_wallet.cards_states = 15;

    memcpy(new_wallet.wallet_name, wallet_list->wallet[index].name, NAME_SIZE);
    memcpy(new_wallet.wallet_id, wallet_list->wallet[index].id, WALLET_ID_SIZE);

    // Record card number on which the wallet is locked
    if (new_wallet.is_wallet_locked) {
      new_wallet.challenge.card_locked = *tapped_card;
      memset(new_wallet.challenge.nonce, 0xFF, POW_NONCE_SIZE);
    }

    uint32_t flash_index = MAX_WALLETS_ALLOWED;
    if (SUCCESS_ != add_wallet_to_flash(&new_wallet, &flash_index) ||
        MAX_WALLETS_ALLOWED <= flash_index) {
      return false;
    }
  }

  return true;
}

bool sync_with_card_eligibility(void) {
  // Ensure there are no partial/invalid/locked wallets in the device
  uint8_t valid_wallets = get_valid_wallet_count();

#ifndef DEV_BUILD
  if (get_wallet_count() != valid_wallets) {
    mark_core_error_screen(ui_text_wallet_partial_fix, false);
    return false;
  }
#endif

  // Ensure that atleast 2 cards are paired
  if (get_keystore_used_count() < MINIMUM_NO_OF_SHARES) {
    mark_core_error_screen(ui_text_error_pair_atleast_2_cards, false);
    return false;
  }

  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void sync_with_cards(void) {
  if (!sync_with_card_eligibility()) {
    return;
  }

  if (!core_scroll_page(NULL, ui_text_sync_x1card_confirm, NULL)) {
    return;
  }

  wallet_list_t wallets_in_card = {0};
  uint8_t tapped_card = 0xFF;

  // Get wallet list from any card
  if (!get_wallets_from_card(&wallets_in_card, &tapped_card)) {
    return;
  }

  // Return if no wallets are found in the card
  if (0 == wallets_in_card.count) {
    mark_core_error_screen(ui_text_wallet_not_found_on_x1card, false);
    return;
  }

  delay_scr_init(ui_text_sync_wallets_next_steps, DELAY_TIME);

  flash_delete_all_wallets();

  if (!sync_wallets_in_flash(&wallets_in_card, &tapped_card)) {
    mark_core_error_screen(
        ui_text_something_went_wrong_contact_support_send_logs, false);
    return;
  }

  uint8_t wallets_synced = 0;
  char msg[100] = "";

  for (uint8_t index = 0; index < MAX_WALLETS_ALLOWED; index++) {
    wallet_state state = VALID_WALLET_WITHOUT_DEVICE_SHARE;
    if (!wallet_is_filled(index, &state)) {
      continue;
    }

    // If the wallet is locked, then move on to the next wallet
    if (is_wallet_locked(index)) {
      snprintf(msg,
               sizeof(msg),
               UI_TEXT_SYNC_WALLET_LOCKED,
               (char *)get_wallet_name(index));
      delay_scr_init(msg, DELAY_TIME);
      continue;
    }

    snprintf(msg,
             sizeof(msg),
             UI_TEXT_SYNC_WALLET_PROMPT,
             (char *)get_wallet_name(index));
    if (!core_scroll_page(NULL, msg, NULL)) {
      continue;
    }

    sync_state_e flow_state = sync_wallets_flow(get_wallet_id(index));
    bool abort = false;

    switch (flow_state) {
      case SYNC_COMPLETED_WITH_ERRORS: {
        // This case will arise if the card operation was aborted or the
        // wallet is locked in cards

        // In case the wallet is locked, then we'll continue with the next
        // wallet, however, if it was an abort error, we exit the sync flow

        if (is_wallet_locked(index)) {
          // Inform user if the wallet is locked in the card and move on to
          // the next card
          clear_core_error_screen();

          snprintf(msg,
                   sizeof(msg),
                   UI_TEXT_SYNC_WALLET_LOCKED,
                   (char *)get_wallet_name(index));
          delay_scr_init(msg, DELAY_TIME);
        } else {
          abort = true;
        }
        break;
      }
      case SYNC_TIMED_OUT: {
        abort = true;
        break;
      }
      case SYNC_COMPLETED: {
        snprintf(msg,
                 sizeof(msg),
                 UI_TEXT_SYNC_WALLET_DONE,
                 (char *)get_wallet_name(index));
        delay_scr_init(msg, DELAY_TIME);
        wallets_synced += 1;
        break;
      }
      // Allow user to sync next wallet if user exitted PIN input
      case SYNC_EARLY_EXIT:
      default:
        break;
    }

    // Return early if we have to abort
    if (abort) {
      return;
    }
  }

  snprintf(msg, sizeof(msg), "%d %s", wallets_synced, ui_text_syncing_complete);
  delay_scr_init(msg, DELAY_TIME);

  return;
}