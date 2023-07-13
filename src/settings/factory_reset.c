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
 * @brief Get the common wallet list from the wallet list retrieved from two X1
 * cards
 *
 * @param wallets_in_card1 Pointer to buffer filled with wallet list in card #1
 * @param wallets_in_card2 Pointer to buffer filled with wallet list in card #2
 * @param common_wallets Pointer to buffer which will be filled with the common
 * wallets in card #1 and card #2
 */
static void get_common_wallet_list(wallet_list_t *wallets_in_card1,
                                   wallet_list_t *wallets_in_card2,
                                   wallet_list_t *common_wallets);

/**
 * @brief Get the uncommon wallet list between the X1 vault and two X1 cards
 *
 * @param wallets_in_vault Pointer to buffer filled with wallet list in X1 vault
 * @param wallets_in_cards Pointer to buffer filled with common wallet list in
 * two cards
 * @param uncommon_wallets Pointer to buffer which will be filled with the
 * wallets present in X1 vault but not in the two cards
 */
static void get_uncommon_wallet_list(wallet_list_t *wallets_in_vault,
                                     wallet_list_t *wallets_in_cards,
                                     wallet_list_t *uncommon_wallets);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void get_uncommon_wallet_list(wallet_list_t *wallets_in_vault,
                                     wallet_list_t *wallets_in_cards,
                                     wallet_list_t *uncommon_wallets) {
  uint8_t uncommon_wallet_count = 0;
  memzero(uncommon_wallets, sizeof(wallet_list_t));

  for (uint8_t index = 0; index < wallets_in_vault->count; index++) {
    bool match = false;

    for (uint8_t i = 0; i < wallets_in_cards->count; i++) {
      if (memcmp(wallets_in_vault->wallet[index].name,
                 wallets_in_vault->wallet[i].name,
                 NAME_SIZE) == 0 &&
          memcmp(wallets_in_cards->wallet[index].id,
                 wallets_in_cards->wallet[i].id,
                 WALLET_ID_SIZE) == 0) {
        match = true;
        break;
      }
    }

    // If the metadata did not match, store it in the uncommon list
    if (!match) {
      uncommon_wallet_count += 1;
      memcpy(&uncommon_wallets->wallet[uncommon_wallet_count - 1],
             &wallets_in_vault->wallet[index],
             sizeof(wallet_metadata_t));
    }
  }

  uncommon_wallets->count = uncommon_wallet_count;
  return;
}

static void get_common_wallet_list(wallet_list_t *wallets_in_card1,
                                   wallet_list_t *wallets_in_card2,
                                   wallet_list_t *common_wallets) {
  uint8_t common_wallet_count = 0;
  memzero(common_wallets, sizeof(wallet_list_t));

  for (uint8_t index = 0; index < wallets_in_card1->count; index++) {
    bool match = false;

    for (uint8_t i = 0; i < wallets_in_card2->count; i++) {
      if (memcmp(wallets_in_card1->wallet[index].name,
                 wallets_in_card2->wallet[i].name,
                 NAME_SIZE) == 0 &&
          memcmp(wallets_in_card1->wallet[index].id,
                 wallets_in_card2->wallet[i].id,
                 WALLET_ID_SIZE) == 0) {
        common_wallet_count += 1;
        match = true;
        break;
      }
    }

    // If the metadata matched, store it in the common list
    if (match) {
      memcpy(&common_wallets->wallet[common_wallet_count - 1],
             &wallets_in_card1->wallet[index],
             sizeof(wallet_metadata_t));
    }
  }

  common_wallets->count = common_wallet_count;
  return;
}

static bool get_wallet_list_from_two_cards(wallet_list_t *wallet_list) {
  wallet_list_t wallets_in_card1 = {0};
  wallet_list_t wallets_in_card2 = {0};

  card_fetch_wallet_list_config_t configuration = {0};
  card_fetch_wallet_list_response_t response = {0};

  configuration.acceptable_cards = 0xF;
  configuration.heading = ui_text_tap_1_2_cards;
  configuration.msg = ui_text_place_card_below;
  configuration.skip_card_removal = false;

  response.wallet_list = &wallets_in_card1;
  response.tapped_card = 0;

  if (!card_fetch_wallet_list(&configuration, &response)) {
    return false;
  }

  // Do not accept the same card again
  configuration.acceptable_cards ^= response.tapped_card;
  configuration.heading = ui_text_tap_2_2_cards;
  configuration.skip_card_removal = true;

  response.wallet_list = &wallets_in_card2;
  response.tapped_card = 0;

  if (!card_fetch_wallet_list(&configuration, &response)) {
    return false;
  }

  get_common_wallet_list(&wallets_in_card1, &wallets_in_card2, wallet_list);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void factory_reset(void) {
  wallet_list_t wallets_in_cards = {0};
  wallet_list_t wallets_in_vault = {0};

  uint8_t valid_wallets = get_valid_wallet_meta_data_list(&wallets_in_vault);

  if (0 < valid_wallets &&
      !core_scroll_page(NULL, ui_text_factory_reset_instruction, NULL)) {
    return;
  }

  if (!core_scroll_page(NULL, ui_text_confirm_factory_reset, NULL)) {
    return;
  }

  // If there is atleast one valid wallet on the device, ensure that it is
  // present in atleast 2 X1 cards
  if (0 < valid_wallets) {
    if (!get_wallet_list_from_two_cards(&wallets_in_cards)) {
      return;
    }

    wallet_list_t uncommon_wallets = {0};
    get_uncommon_wallet_list(
        &wallets_in_vault, &wallets_in_cards, &uncommon_wallets);

    if (0 < uncommon_wallets.count) {
      // Display first wallet not present in cards - just to keep it
      // consistent with current UX
      char msg[64];
      const char *msg_list[3] = {
          msg, ui_text_reset_exit[0], ui_text_reset_exit[1]};

      snprintf(msg,
               sizeof(msg),
               UI_TEXT_FACTORY_RESET_ERROR,
               uncommon_wallets.wallet[0].name);

      multi_instruction_init(msg_list, 3, DELAY_LONG_STRING, true);
      // Do not care about the return value from confirmation screen
      (void)get_state_on_confirm_scr(0, 0, 0);
      return;
    }
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