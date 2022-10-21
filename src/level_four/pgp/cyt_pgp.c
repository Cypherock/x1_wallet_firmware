/**
 * @file    cyt_sign_data.c
 * @author  Cypherock X1 Team
 * @brief   Sign Data tasks.
 *          Handles pre-processing & display updates for the sign data.
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
#include "controller_main.h"
#include "cy_pgp.h"
#include "tasks_tap_cards.h"
#include "ui_address.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"

extern char *PASSPHRASE;
extern char *ALPHA_NUMERIC;
extern char *NUMBERS;

void cyt_pgp() {
  switch (flow_level.level_three) {
    case LEVEL_THREE_PGP_INIT:
      instruction_scr_init(ui_text_processing, NULL);
      mark_event_over();
      break;

    case LEVEL_THREE_PGP_CHOOSE_WALLET: {
      char *wallet_names[MAX_WALLETS_ALLOWED];

      uint8_t walletAdded = 0;
      uint8_t walletIndex = 0;

      for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++) {
        if (get_wallet_state(walletIndex) == VALID_WALLET &&
            get_wallet_card_state(walletIndex) == 0x0f &&
            get_wallet_locked_status(walletIndex) == 0) {
          wallet_names[walletAdded] = (char *)get_wallet_name(walletIndex);
          walletAdded++;
        }
      }

      menu_init((const char **)wallet_names,
                walletAdded,
                ui_text_choose_wallet,
                true);
    } break;

    case LEVEL_THREE_PGP_ENTER_PASSPHRASE: {
      input_text_init(
          PASSPHRASE, ui_text_enter_passphrase, 0, DATA_TYPE_PASSPHRASE, 64);

    } break;

    case LEVEL_THREE_PGP_CONFIRM_PASSPHRASE: {
      char display[65];
      snprintf(
          display, sizeof(display), "%s", flow_level.screen_input.input_text);
      address_scr_init(ui_text_confirm_passphrase, display, false);
      memzero(display, sizeof(display));
    } break;

    case LEVEL_THREE_PGP_ENTER_PIN: {
      input_text_init(ALPHA_NUMERIC, ui_text_enter_pin, 4, DATA_TYPE_PIN, 8);

    } break;

    case LEVEL_THREE_PGP_TAP_CARD: {
      tap_threshold_cards_for_reconstruction();
    } break;

    case LEVEL_THREE_PGP_DERIVE_HDNODE:
      instruction_scr_destructor();
      instruction_scr_init("", NULL);
      instruction_scr_change_text(ui_text_processing, true);
      mark_event_over();
      break;

    case LEVEL_THREE_PGP_PUBKEY:
      instruction_scr_change_text("Sending public key", true);
      BSP_DelayMs(DELAY_SHORT);
      mark_event_over();
      break;

    case LEVEL_THREE_PGP_ECDH:
      instruction_scr_change_text("Generating session key", true);
      BSP_DelayMs(DELAY_SHORT);
      mark_event_over();
      break;

    case LEVEL_THREE_PGP_SIGN:
      instruction_scr_change_text("Sending signature", true);
      BSP_DelayMs(DELAY_SHORT);
      mark_event_over();
      break;

    default:
      mark_event_over();
      break;
  }
}