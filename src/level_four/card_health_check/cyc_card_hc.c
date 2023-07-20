/**
 * @file    cyc_card_hc.c
 * @author  Cypherock X1 Team
 * @brief   Card health check next controller.
 *          This file contains the functions to handle the card health check
 *next events.
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
#include "app_error.h"
#include "controller_tap_cards.h"
#include "cy_card_hc.h"
#include "ui_instruction.h"

uint8_t *wallet_list[MAX_WALLETS_ALLOWED][2];
uint8_t wallet_count;
uint32_t card_fault_status = 0;
#if X1WALLET_MAIN
static uint16_t wallet_data_len;
static uint8_t wallet_data[300];
static uint16_t decode_wallet_info_list(
    uint8_t *wallet_info[MAX_WALLETS_ALLOWED][2],
    uint8_t *data,
    uint16_t len);
static void tap_card_backend(uint8_t *recv_apdu, uint16_t *recv_len);
#endif

void cyc_card_hc() {
#if X1WALLET_MAIN
  switch (flow_level.level_three) {
    case CARD_HC_START:
      wallet_count = 0xFF;
      card_fault_status = 0;
      wallet_data_len = 0;
      memzero(wallet_data, sizeof(wallet_data));
      tap_card_data.retries = 5;
      flow_level.level_three = CARD_HC_TAP_CARD;
      break;

    case CARD_HC_TAP_CARD:
      tap_card_data.lvl4_retry_point = 1;
      tap_card_data.lvl3_retry_point = CARD_HC_TAP_CARD;
      tap_card_backend(wallet_data, &wallet_data_len);
      ASSERT(wallet_data_len < sizeof(wallet_data));
      break;

    case CARD_HC_DISPLAY_CARD_HEALTH: {
      flow_level.level_three = CARD_HC_DISPLAY_WALLETS;
    } break;

    case CARD_HC_DISPLAY_WALLETS: {
      flow_level.level_one = LEVEL_TWO_ADVANCED_SETTINGS;
      reset_flow_level_greater_than(LEVEL_ONE);
      counter.level = LEVEL_TWO;
    } break;

    default:
      reset_flow_level();
  }
}

static void tap_card_backend(uint8_t *recv_apdu, uint16_t *recv_len) {
  // All card errors are abstracted from the NFC handler and added to the health
  // report.
  while (1) {
    memcpy(tap_card_data.family_id, get_family_id(), FAMILY_ID_SIZE);
    tap_card_data.acceptable_cards = 15;
    tap_card_data.tapped_card = 0;
    tap_card_data.card_absent_retries = 10;
    if (!tap_card_applet_connection()) {
      switch ((uint32_t)tap_card_data.status) {
        case SW_FILE_NOT_FOUND:
        case SW_INCOMPATIBLE_APPLET:
          counter.level = LEVEL_THREE;
          flow_level.level_one = LEVEL_TWO_ADVANCED_SETTINGS;
          flow_level.level_two = LEVEL_THREE_CARD_HEALTH_CHECK;
        case NFC_CARD_ABSENT:
          flow_level.show_error_screen = false;
          card_fault_status = tap_card_data.status;
          flow_level.level_three = CARD_HC_DISPLAY_CARD_HEALTH;
        default:
          return;
      }
    }
    // tap_card_data.status = nfc_list_all_wallet(recv_apdu, recv_len);
    *recv_len -= 2;
    switch ((uint32_t)tap_card_data.status) {
      case SW_NO_ERROR:
      case SW_RECORD_NOT_FOUND: {
        flow_level.level_three = CARD_HC_DISPLAY_CARD_HEALTH;
        wallet_count =
            decode_wallet_info_list(wallet_list,
                                    wallet_data,
                                    wallet_data_len);    // wallet list
        buzzer_start(BUZZER_DURATION);
        instruction_scr_destructor();
        if (tap_card_data.recovery_mode) {
          card_fault_status = NFC_NULL_PTR_ERROR;
          flow_level.show_error_screen = false;
        }
        return;
      } break;

      case SW_NULL_POINTER_EXCEPTION:
      case SW_OUT_OF_BOUNDARY:
      case SW_INVALID_INS:
      case SW_INS_BLOCKED:
        card_fault_status = tap_card_data.status;
        instruction_scr_destructor();
        flow_level.level_three = CARD_HC_DISPLAY_CARD_HEALTH;
        flow_level.show_error_screen = false;
        return;

      default:
        if (tap_card_handle_applet_errors()) {
          // Do not throw error; instead silently use them for deciding card
          // health.
          switch ((uint32_t)tap_card_data.status) {
            case NFC_CARD_ABSENT:
              flow_level.show_error_screen = false;
              card_fault_status = tap_card_data.status;
              flow_level.level_three = CARD_HC_DISPLAY_CARD_HEALTH;
              break;
            default:
              break;
          }
        }
        break;
    }
  }
}

static uint16_t decode_wallet_info_list(
    uint8_t *wallet_info[MAX_WALLETS_ALLOWED][2],
    uint8_t *data,
    uint16_t len) {
  uint16_t count = 0;
  memzero(wallet_info, MAX_WALLETS_ALLOWED * 2 * sizeof(size_t));
  for (uint16_t index = 1 + 4; index < len; count++) {
    ASSERT(data[index++] == INS_NAME);
    wallet_info[count][0] = &data[++index];
    index += data[index - 1];
    ASSERT(data[index++] == INS_WALLET_ID);
    wallet_info[count][1] = &data[++index];
    index += data[index - 1] + 4;
  }
  return count;
#endif
}