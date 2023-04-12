/**
 * @file    card_upgrade_controller.c
 * @author  Cypherock X1 Team
 * @brief   Card upgrade next controller.
 *          Handles post event (only next events) operations for card upgrade
 *flow initiated by desktop app.
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
#include "communication.h"
#include "controller_level_four.h"
#include "nfc.h"
#include "ui_instruction.h"

extern Flow_level flow_level;
extern Counter counter;
extern Wallet wallet;

char *textDisplay;
char text[80];

#ifdef DEV_BUILD
static uint16_t number_of_apdus_sent;
#endif    // DEV_BUILD

void card_upgrade_controller() {
#ifdef DEV_BUILD
  switch (flow_level.level_three) {
    case CARD_UPGRADE_TAP_CARD_MESSAGE:
      flow_level.level_three = CARD_UPGRADE_SELECT_CARD;
      break;

    case CARD_UPGRADE_SELECT_CARD: {
      while (nfc_select_card() != STM_SUCCESS)
        ;
      flow_level.level_three = CARD_UPGRADE_FORWARD_MESSAGE;
      number_of_apdus_sent = 0;
      snprintf(text, sizeof(text), "Sending APDU\n%d", number_of_apdus_sent);
      textDisplay = text;
      instruction_scr_change_text(textDisplay, false);
      uint8_t arr[4];
      arr[0] = STATUS_CMD_SUCCESS;
      transmit_data_to_app(STATUS_PACKET, arr, 4);
    } break;

    case CARD_UPGRADE_FORWARD_MESSAGE: {
      uint8_t *data_array = NULL;
      uint16_t msg_size = 0;
      if (get_usb_msg_by_cmd_type(APDU_PACKET, &data_array, &msg_size)) {
        uint8_t max_tries = 5;
        uint8_t recv_apdu[255], recv_len = 236;
        ret_code_t err_code = STM_ERROR_BUSY;

        while (max_tries--) {
          err_code = adafruit_pn532_in_data_exchange(
              data_array, msg_size, recv_apdu, &recv_len);

          if (err_code == STM_SUCCESS) {
            break;
          }
        }

        if (err_code == STM_SUCCESS) {
          transmit_data_to_app(APDU_PACKET, recv_apdu, recv_len);
          number_of_apdus_sent++;
          snprintf(
              text, sizeof(text), "Sending APDU\n%d", number_of_apdus_sent);
          textDisplay = text;
          instruction_scr_change_text(textDisplay, false);
        } else {
          uint8_t arr[4];
          arr[0] = STATUS_CMD_FAILURE;
          transmit_data_to_app(STATUS_PACKET, arr, sizeof(arr));
          instruction_scr_destructor();
          mark_error_screen(ui_text_card_command_send_error);
          reset_flow_level();
        }
        clear_message_received_data();
      } else if (get_usb_msg_by_cmd_type(STOP_CARD_UPGRADE, NULL, NULL)) {
        mark_error_screen(ui_text_card_update_done);
        reset_flow_level();
        clear_message_received_data();
      }
    } break;

    default:
      break;
  }
#endif
}
