/**
 * @file    factory_reset.c
 * @author  Cypherock X1 Team
 * @brief   Source file containing logic and checks to perform factory reset a
 *          X1 vault device.
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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "card_operations.h"
#include "coin_specific_data.h"
#include "constant_texts.h"
#include "core_error.h"
#include "flash_api.h"
#include "flash_struct.h"
#include "settings_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "ui_state_machine.h"
#include "wallet_list.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef bool (*filter)(bool);

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Get the wallet list from two X1 cards
 *
 * @param wallet_list Pointer to buffer which will be filled with the wallet
 * list of type wallet_list_t
 * @return true If the card flow occurred successfully
 * @return false If any terminal error occurred during the card flow
 */
static bool get_wallet_list_from_two_cards(wallet_list_t *wallet_list);

/**
 * @brief Helper function to filter wallets between 2 wallet lists based on a
 * filter callback
 *
 * @param wallet_list1 Pointer to the 1st wallet list
 * @param wallet_list2 Pointer to the 2nd wallet list
 * @param wallet_list_out Pointer to buffer where filtered wallet list will be
 * populated
 * @param filter_cb Callback to function containing logic whether a wallet needs
 * to be added to the list based on match status
 */
static void filter_wallet_list(wallet_list_t *wallet_list1,
                               wallet_list_t *wallet_list2,
                               wallet_list_t *wallet_list_out,
                               filter filter_cb);

/**
 * @brief Callback function to indicate if a wallets needs to be filtered if a
 * match is not found
 *
 * @param match_found Boolean representing whether a match was found in 2 lists
 * @return true If the wallets needs to be filtered
 * @return false If the wallets should not be filtered
 */
static bool filter_uncommon_wallet(bool match_found);

/**
 * @brief Callback function to indicate if a wallets needs to be filtered if a
 * match is found
 *
 * @param match_found Boolean representing whether a match was found in 2 lists
 * @return true If the wallets should not be filtered
 * @return false If the wallets should be filtered
 */
static bool filter_common_wallet(bool match_found);

/**
 * @brief Helper function to check whether wallet share is present in 2 X1 cards
 * for wallets with share on the X1 vault
 * @details An error message is displayed on the device if any share is not
 * found in 2 X1 cards.
 *
 * @param wallets_in_vault
 * @param exit_msgs
 * @return true If wallet share is present in atleast 2 X1 cards for each wallet
 * present on the device.
 * @return false If wallet share is NOT present in atleast 2 X1 cards, or any
 * error occured in the card flow
 */
static bool safe_to_delete_wallet_share(wallet_list_t *wallets_in_vault,
                                        const char *exit_msgs[]);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void filter_wallet_list(wallet_list_t *wallet_list1,
                               wallet_list_t *wallet_list2,
                               wallet_list_t *wallet_list_out,
                               filter filter_cb) {
  memzero(wallet_list_out, sizeof(wallet_list_t));
  wallet_list_out->count = 0;

  for (uint8_t index = 0; index < wallet_list1->count; index++) {
    bool match = false;

    for (uint8_t i = 0; i < wallet_list2->count; i++) {
      if (memcmp(wallet_list1->wallet[index].name,
                 wallet_list2->wallet[i].name,
                 NAME_SIZE) == 0 &&
          memcmp(wallet_list1->wallet[index].id,
                 wallet_list2->wallet[i].id,
                 WALLET_ID_SIZE) == 0) {
        match = true;
        break;
      }
    }

    // If the filter returned true, store it in the output list
    if (filter_cb(match)) {
      memcpy(&wallet_list_out->wallet[wallet_list_out->count],
             &wallet_list1->wallet[index],
             sizeof(wallet_metadata_t));
      wallet_list_out->count += 1;
    }
  }

  return;
}

static bool filter_uncommon_wallet(bool match_found) {
  return (!match_found);
}

static bool filter_common_wallet(bool match_found) {
  return match_found;
}

static bool get_wallet_list_from_two_cards(wallet_list_t *wallet_list) {
  wallet_list_t wallets_in_card1 = {0};
  wallet_list_t wallets_in_card2 = {0};

  card_fetch_wallet_list_config_t configuration = {0};
  card_fetch_wallet_list_response_t response = {0};

  configuration.operation.acceptable_cards = ACCEPTABLE_CARDS_ALL;
  configuration.operation.skip_card_removal = false;
  configuration.operation.expected_family_id = get_family_id();
  configuration.frontend.heading = ui_text_tap_1_2_cards;
  configuration.frontend.msg = ui_text_place_card_below;

  response.wallet_list = &wallets_in_card1;
  response.card_info.tapped_card = 0;
  response.card_info.recovery_mode = 0;
  response.card_info.status = 0;
  response.card_info.tapped_family_id = NULL;

  if (CARD_OPERATION_SUCCESS !=
      card_fetch_wallet_list(&configuration, &response)) {
    return false;
  }

  // Do not accept the same card again
  configuration.operation.acceptable_cards ^= response.card_info.tapped_card;
  configuration.operation.skip_card_removal = true;
  configuration.operation.expected_family_id = get_family_id();
  configuration.frontend.heading = ui_text_tap_2_2_cards;

  response.wallet_list = &wallets_in_card2;
  response.card_info.tapped_card = 0;
  response.card_info.recovery_mode = 0;
  response.card_info.status = 0;
  response.card_info.tapped_family_id = NULL;

  if (CARD_OPERATION_SUCCESS !=
      card_fetch_wallet_list(&configuration, &response)) {
    return false;
  }

  delay_scr_init(ui_text_processing, DELAY_TIME);

  filter_wallet_list(
      &wallets_in_card1, &wallets_in_card2, wallet_list, filter_common_wallet);
  return true;
}

static bool safe_to_delete_wallet_share(wallet_list_t *wallets_in_vault,
                                        const char *exit_msgs[]) {
  wallet_list_t wallets_in_cards = {0};
  if (!get_wallet_list_from_two_cards(&wallets_in_cards)) {
    return false;
  }

  wallet_list_t uncommon_wallets = {0};
  filter_wallet_list(wallets_in_vault,
                     &wallets_in_cards,
                     &uncommon_wallets,
                     filter_uncommon_wallet);

  if (0 < uncommon_wallets.count) {
    // Display first wallet not present in cards - just to keep it
    // consistent with current UX

    // TODO: Update message to show ALL missing wallets instead of just one
    // wallet
    char msg[64];
    const char *msg_list[3] = {msg, exit_msgs[0], exit_msgs[1]};

    snprintf(msg,
             sizeof(msg),
             UI_TEXT_FACTORY_RESET_ERROR,
             uncommon_wallets.wallet[0].name);

    multi_instruction_init(msg_list, 3, DELAY_LONG_STRING, true);
    // Do not care about the return value from confirmation screen
    (void)get_state_on_confirm_scr(0, 0, 0);
    return false;
  }

  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void factory_reset(void) {
  wallet_list_t wallets_in_vault = {0};
  uint8_t valid_wallets = get_filled_wallet_meta_data_list(&wallets_in_vault);

  if (0 < valid_wallets &&
      !core_scroll_page(NULL, ui_text_factory_reset_instruction, NULL)) {
    return;
  }

  if (!core_scroll_page(NULL, ui_text_confirm_factory_reset, NULL)) {
    return;
  }

  // If there is atleast one valid wallet on the device, ensure that it is
  // present in atleast 2 X1 cards
  if (0 < valid_wallets &&
      !safe_to_delete_wallet_share(&wallets_in_vault, ui_text_reset_exit)) {
    return;
  }

  // Safe to erase flash data
  delay_scr_init(ui_text_erasing, DELAY_TIME);

  if (LEFT_HAND_VIEW == get_display_rotation()) {
    ui_rotate();
  }

  sec_flash_erase();
  flash_erase();
  erase_flash_coin_specific_data();
  logger_reset_flash();

  // Reset device to apply factory reset
  BSP_reset();
}

void clear_device_data(void) {
  wallet_list_t wallets_in_vault = {0};
  uint8_t valid_wallets = get_filled_wallet_meta_data_list(&wallets_in_vault);

  if (0 < valid_wallets &&
      !core_scroll_page(NULL, ui_text_clear_device_data_instruction, NULL)) {
    return;
  }

  if (!core_scroll_page(NULL, ui_text_confirm_clear_device_data, NULL)) {
    return;
  }

  // If there is atleast one valid wallet on the device, ensure that it is
  // present in atleast 2 X1 cards
  if (0 < valid_wallets &&
      !safe_to_delete_wallet_share(&wallets_in_vault,
                                   ui_text_clear_wallet_data_exit)) {
    return;
  }

  // Safe to erase flash data
  delay_scr_init(ui_text_erasing_device_data, DELAY_TIME);

  if (LEFT_HAND_VIEW == get_display_rotation()) {
    ui_rotate();
  }

  sec_flash_erase();
  flash_clear_user_data();
  erase_flash_coin_specific_data();
  logger_reset_flash();

  // Reset device to apply new settings
  BSP_reset();
}