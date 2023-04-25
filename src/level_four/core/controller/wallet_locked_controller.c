/**
 * @file    wallet_locked_controller.c
 * @author  Cypherock X1 Team
 * @brief   Wallet unlock next controller.
 *          Handles post event (only next events) operations for unlock wallet.
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
#include "constant_texts.h"
#include "controller_level_four.h"
#include "controller_tap_cards.h"
#include "nfc.h"
#include "pow.h"
#include "pow_utilities.h"
#include "sha2.h"
#include "ui_instruction.h"
#include "ui_message.h"
#include "ui_text_slideshow.h"

extern Flow_level flow_level;
extern Counter counter;
extern Wallet wallet;

static void _wallet_locked_tap_card();

void wallet_locked_controller() {
  switch (flow_level.level_three) {
    case WALLET_LOCKED_MESSAGE:
      lv_obj_clean(lv_scr_act());
      tap_card_data.desktop_control = false;
      flow_level.level_three = WALLET_LOCKED_ENTER_PIN;
      break;

    case WALLET_LOCKED_ENTER_PIN:
      sha256_Raw((uint8_t *)flow_level.screen_input.input_text,
                 strnlen(flow_level.screen_input.input_text,
                         sizeof(flow_level.screen_input.input_text)),
                 wallet.password_double_hash);
      sha256_Raw(wallet.password_double_hash,
                 SHA256_DIGEST_LENGTH,
                 wallet.password_double_hash);
      memzero(flow_level.screen_input.input_text,
              sizeof(flow_level.screen_input.input_text));
      flow_level.level_three = WALLET_LOCKED_TAP_CARD_FRONTEND;
      break;

    case WALLET_LOCKED_TAP_CARD_FRONTEND:
      flow_level.level_three = WALLET_LOCKED_TAP_CARD_BACKEND;
      break;

    case WALLET_LOCKED_TAP_CARD_BACKEND:
      _wallet_locked_tap_card();
      break;

    case WALLET_LOCKED_SUCCESS:
      reset_flow_level();
      break;

    default:
      message_scr_init(ui_text_something_went_wrong);
      break;
  }
}

static void _wallet_locked_tap_card() {
  uint8_t wallet_index;

  get_index_by_name((const char *)wallet.wallet_name, &wallet_index);
  memcpy(tap_card_data.family_id, get_family_id(), FAMILY_ID_SIZE);
  tap_card_data.retries = 5;
  while (1) {
    tap_card_data.acceptable_cards = get_wallet_card_locked(wallet_index);
    tap_card_data.lvl3_retry_point = WALLET_LOCKED_TAP_CARD_FRONTEND;
    tap_card_data.tapped_card = 0;
    if (!tap_card_applet_connection())
      return;
    tap_card_data.lvl3_retry_point = WALLET_LOCKED_ENTER_PIN;
    tap_card_data.status = nfc_verify_challenge(wallet.wallet_name,
                                                get_proof_of_work_nonce(),
                                                wallet.password_double_hash);

    if (tap_card_data.status == SW_NO_ERROR ||
        tap_card_data.status == SW_WARNING_STATE_UNCHANGED) {
      update_wallet_locked_flash((const char *)wallet.wallet_name, false);
      flow_level.level_three = WALLET_LOCKED_SUCCESS;
      buzzer_start(BUZZER_DURATION);
      instruction_scr_destructor();
      break;
    } else if (tap_card_data.status == POW_SW_CHALLENGE_FAILED) {
      uint8_t target[SHA256_SIZE], random_number[POW_RAND_NUMBER_SIZE];
      char log[122] = {0}, offset = 0;
      for (int i = 0; i < POW_NONCE_SIZE; i++) {
        offset += snprintf(log + offset,
                           sizeof(log) - offset,
                           "%02X",
                           get_proof_of_work_nonce()[i]);
      }
      LOG_CRITICAL("nonce: %s", log);
      tap_card_data.status =
          nfc_get_challenge(wallet.wallet_name, target, random_number);
      if (tap_card_data.status == SW_NO_ERROR) {
        if (memcmp(get_wallet_by_index(wallet_index)->challenge.random_number,
                   random_number,
                   POW_RAND_NUMBER_SIZE) != 0)
          LOG_CRITICAL("E: pow-rand");
        if (memcmp(get_wallet_by_index(wallet_index)->challenge.target,
                   target,
                   SHA256_SIZE) != 0) {
          int old_lvl = pow_count_set_bits(
              get_wallet_by_index(wallet_index)->challenge.target);
          int new_lvl = pow_count_set_bits(target);
          LOG_CRITICAL("E: pow-tg (o: %d, n: %d)", old_lvl, new_lvl);
        }
        instruction_scr_destructor();
        mark_error_screen(ui_text_pow_challenge_failed);
        add_challenge_flash((const char *)wallet.wallet_name,
                            target,
                            random_number,
                            tap_card_data.tapped_card);
        reset_flow_level();
        buzzer_start(BUZZER_DURATION);
        break;
      }
    } else if (tap_card_handle_applet_errors()) {
      break;
    }
  }
}