/**
 * @file    controller_level_one_initial.c
 * @author  Cypherock X1 Team
 * @brief   Level one next controller (initial).
 *          Handles post event (only next events) operations for level one
 *          tasks of the initial application.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
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
#include "communication.h"
#include "controller_level_one.h"
#include "controller_level_two.h"
#include "controller_tap_cards.h"
#include "ui_delay.h"

#if USE_SIMULATOR == 0
#include "controller_tap_cards.h"
#include "ui_delay.h"
#include "libusb/libusb.h"

#endif

extern char card_id_fetched[];
extern char card_version[];

void level_one_controller_initial() {
#if X1WALLET_INITIAL
  if (flow_level.show_error_screen) {
    flow_level.show_error_screen = false;
    return;
  }

  if (flow_level.show_desktop_start_screen) {
    flow_level.show_desktop_start_screen = false;
    return;
  }

  if (counter.level > LEVEL_ONE) {
    level_two_controller();
    return;
  }

  switch (flow_level.level_one) {
    case 1:
    case 2: {
      flow_level.level_one++;
    } break;

    case 3: {
      flow_level.level_one = 4;
      mark_device_state(true);
    } break;

    case 4: {
      controller_read_card_id();
      reset_flow_level();
      flow_level.level_one = 5;
    } break;

    case 5: {
      controller_read_card_id();
      char msg[32] = {'\0'};
      snprintf(msg, sizeof(msg), "%s Card Tapped", decode_card_number(card_id_fetched[2 * CARD_ID_SIZE - 1] - '0'));
      delay_scr_init(msg, DELAY_TIME);
      reset_flow_level();
      flow_level.level_one = 6;
#if USE_SIMULATOR == 0
      libusb_init();
#endif
    } break;

    case 6: {
      flow_level.level_one = 6;
    } break;

    case 7: {
      flow_level.level_one = 7;
    } break;

    default:
      break;
  }
#endif
}
