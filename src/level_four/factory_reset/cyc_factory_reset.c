/**
 * @file    cyc_factory_reset.c
 * @author  Cypherock X1 Team
 * @brief   Factory reset next controller.
 *          This file contains the functions to handle the factory reset next
 *events.
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
#include "coin_specific_data.h"
#include "controller_tap_cards.h"
#include "cy_factory_reset.h"
#include "ui_instruction.h"

static uint8_t acceptable_cards = 15;
static uint16_t card1_len = 0, card2_len = 0;
static uint8_t card1[300] = {0}, card2[300] = {0};
static uint8_t wallets_check = 0;

uint8_t factory_reset_mismatch_wallet_index = 0;

void cyc_factory_reset() {
  switch (flow_level.level_three) {
    case FACTORY_RESET_INIT:
      wallets_check = 0;
      for (int i = 0; i < get_wallet_count(); i++) {
        // Cards needed only if any valid wallets exist
        if (get_wallet_state(i) == VALID_WALLET)
          wallets_check = 1;
      }
#if DEV_BUILD
      flow_level.level_three = FACTORY_RESET_ERASING;
#else
      flow_level.level_three =
          wallets_check ? FACTORY_RESET_INFO : FACTORY_RESET_CONFIRM;
#endif
      break;

    case FACTORY_RESET_INFO:
      flow_level.level_three = FACTORY_RESET_CONFIRM;
      break;

    case FACTORY_RESET_CONFIRM:
      acceptable_cards = 15;
      card1_len = 0;
      card2_len = 0;
      memzero(card1, sizeof(card1));
      memzero(card2, sizeof(card2));
      tap_card_data.retries = 5;
      flow_level.level_three =
          wallets_check ? FACTORY_RESET_TAP_CARD1 : FACTORY_RESET_ERASING;
      break;

    case FACTORY_RESET_TAP_CARD1:
      tap_card_data.lvl4_retry_point = 1;
      tap_card_data.lvl3_retry_point = FACTORY_RESET_TAP_CARD1;
      ASSERT(card1_len < sizeof(card1));
      break;

    case FACTORY_RESET_TAP_CARD2:
      tap_card_data.lvl4_retry_point = 1;
      tap_card_data.lvl3_retry_point = FACTORY_RESET_TAP_CARD2;
      ASSERT(card2_len < sizeof(card2));
      break;

    case FACTORY_RESET_CHECK: {
      const uint8_t *wallet_name, *wallet_id;
      uint8_t *wallet_info[MAX_WALLETS_ALLOWED][2] = {0};
      uint8_t total_wallets = get_wallet_count(), state;
      for (uint8_t i = 0; i < total_wallets; i++) {
        uint8_t match = 0;
        state = get_wallet_state(i);
        if (state != VALID_WALLET)
          continue;
        wallet_name = get_wallet_name(i);
        wallet_id = get_wallet_id(i);
        for (int j = 0; j < MAX_WALLETS_ALLOWED; j++) {
          if (!wallet_info[j][0] || !wallet_info[j][1])
            continue;
          if (memcmp(wallet_name, wallet_info[j][0], NAME_SIZE) == 0 &&
              memcmp(wallet_id, wallet_info[j][1], WALLET_ID_SIZE) == 0) {
            match = 1;
            break;
          }
        }
        if (!match) {
          factory_reset_mismatch_wallet_index = i;
          flow_level.level_three = FACTORY_RESET_CANCEL;
          return;
        }
      }
      flow_level.level_three = FACTORY_RESET_ERASING;
    } break;

    case FACTORY_RESET_ERASING:
      if (get_display_rotation() == LEFT_HAND_VIEW)
        ui_rotate();
      sec_flash_erase();
      flash_erase();
      erase_flash_coin_specific_data();
      logger_reset_flash();
      flow_level.level_three = FACTORY_RESET_DONE;
      break;

    case FACTORY_RESET_DONE:
      BSP_reset();
      break;

    default:
      reset_flow_level();
  }
}
