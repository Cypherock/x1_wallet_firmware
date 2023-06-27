/**
 * @file    verify_card_controller.c
 * @author  Cypherock X1 Team
 * @brief   Verify card next controller.
 *          Handles post event (only next events) operations for verify card
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
#include "buzzer.h"
#include "communication.h"
#include "controller_level_four.h"
#include "controller_tap_cards.h"
#include "nfc.h"
#include "ui_instruction.h"

#define CARD_AUTH_RAND_NUMBER_SIZE 32

static uint8_t signature[ECDSA_SIGNATURE_SIZE];
static uint16_t length;
static uint8_t coded_card_number = 0;

/* Serial Number = Family ID appended with Card Number
 * Return serial number
 */
static uint8_t get_card_serial_number(uint8_t family_id[],
                                      uint8_t cards_state,
                                      uint8_t serial_number_out[]) {
  uint8_t card_number = 1;

  memcpy(serial_number_out, family_id, FAMILY_ID_SIZE);

  cards_state =
      15 ^
      cards_state;    // If 3rd card is tapped 15 ^ cards_state will be (0100)b

  while (cards_state != 1) {
    cards_state = cards_state >> 1;
    card_number++;
  }

  memcpy(serial_number_out + FAMILY_ID_SIZE, &card_number, 1);

  return CARD_ID_SIZE;
}

void verify_card_controller() {
  switch (flow_level.level_three) {
    case VERIFY_CARD_START_MESSAGE:
      tap_card_data.desktop_control = true;
      tap_card_data.active_cmd_type = START_CARD_AUTH;
      flow_level.level_three = VERIFY_CARD_ESTABLISH_CONNECTION_FRONTEND;
      break;

    case VERIFY_CARD_ESTABLISH_CONNECTION_FRONTEND:
      tap_card_data.retries = 5;
      flow_level.level_three = VERIFY_CARD_ESTABLISH_CONNECTION_BACKEND;
      break;

    case VERIFY_CARD_ESTABLISH_CONNECTION_BACKEND:

      memset(tap_card_data.family_id, DEFAULT_VALUE_IN_FLASH, FAMILY_ID_SIZE);

      tap_card_data.lvl3_retry_point =
          VERIFY_CARD_ESTABLISH_CONNECTION_FRONTEND;
      while (1) {
        tap_card_data.acceptable_cards = 15;    // Any card is acceptable
        tap_card_data.tapped_card = 0;
        if (!tap_card_applet_connection())
          break;
        length = get_card_serial_number(tap_card_data.family_id,
                                        tap_card_data.acceptable_cards,
                                        signature /*Serial Number Out*/);
        tap_card_data.status = nfc_ecdsa(signature, &length);

        if (tap_card_data.status == SW_NO_ERROR) {
          uint8_t data_out[ECDSA_SIGNATURE_SIZE + CARD_ID_SIZE];

          memcpy(data_out, signature, ECDSA_SIGNATURE_SIZE);
          get_card_serial_number(tap_card_data.family_id,
                                 tap_card_data.acceptable_cards,
                                 data_out + ECDSA_SIGNATURE_SIZE);
          transmit_data_to_app(SEND_SIGNATURE_TO_APP,
                               data_out,
                               ECDSA_SIGNATURE_SIZE + CARD_ID_SIZE);
          flow_level.level_three = VERIFY_CARD_FETCH_RANDOM_NUMBER;
          buzzer_start(BUZZER_DURATION);

          coded_card_number = tap_card_data.tapped_card;
          break;
        } else if (tap_card_handle_applet_errors()) {
          break;
        }
      }
      break;

    case VERIFY_CARD_FETCH_RANDOM_NUMBER: {
      En_command_type_t msg_type;
      uint8_t *data_array = NULL;
      uint16_t msg_size = 1;
      get_usb_msg(&msg_type, &data_array, &msg_size);

      if (msg_type == APP_SEND_RAND_NUM) {
        memcpy(signature, data_array, CARD_AUTH_RAND_NUMBER_SIZE);
        flow_level.level_three = VERIFY_CARD_SIGN_RANDOM_NUMBER_FRONTEND;
      } else {
        flow_level.level_three = VERIFY_CARD_FAILED;
      }
      clear_message_received_data();
    } break;

    case VERIFY_CARD_SIGN_RANDOM_NUMBER_FRONTEND:
      tap_card_data.retries = 5;
      flow_level.level_three = VERIFY_CARD_SIGN_RANDOM_NUMBER_BACKEND;
      break;

    case VERIFY_CARD_SIGN_RANDOM_NUMBER_BACKEND: {
      length = CARD_AUTH_RAND_NUMBER_SIZE;
      tap_card_data.lvl3_retry_point = VERIFY_CARD_SIGN_RANDOM_NUMBER_FRONTEND;
      while (1) {
        tap_card_data.acceptable_cards = coded_card_number;
        tap_card_data.tapped_card = 0;
        if (!tap_card_applet_connection())
          break;
        else
          tap_card_data.status = nfc_ecdsa(signature, &length);

        if (tap_card_data.status == SW_NO_ERROR) {
          uint8_t data_out[ECDSA_SIGNATURE_SIZE + CARD_ID_SIZE];

          memcpy(data_out, signature, ECDSA_SIGNATURE_SIZE);
          get_card_serial_number(tap_card_data.family_id,
                                 tap_card_data.acceptable_cards,
                                 data_out + ECDSA_SIGNATURE_SIZE);
          transmit_data_to_app(
              SIGNED_CHALLENGE, data_out, ECDSA_SIGNATURE_SIZE + CARD_ID_SIZE);
          buzzer_start(BUZZER_DURATION);
          instruction_scr_change_text(ui_text_remove_card_prompt, true);
          nfc_detect_card_removal();

          flow_level.level_three = VERIFY_CARD_FINAL_MESSAGE;
          break;
        } else if (tap_card_handle_applet_errors()) {
          break;
        }
      }
    } break;

    case VERIFY_CARD_FINAL_MESSAGE: {
      En_command_type_t msg_type;
      uint8_t *data_array = NULL;
      uint16_t msg_size = 1;

      if (!get_usb_msg(&msg_type, &data_array, &msg_size))
        return;
      if (msg_type != STATUS_PACKET) {
        comm_reject_invalid_cmd();
        clear_message_received_data();
        return;
      }

      if (msg_type == STATUS_PACKET && data_array[0] == STATUS_CMD_SUCCESS)
        flow_level.level_three = VERIFY_CARD_SUCCESS;
      else
        flow_level.level_three = VERIFY_CARD_FAILED;
      clear_message_received_data();
      comm_process_complete();
    } break;

    case VERIFY_CARD_SUCCESS:
      if (get_paired_card_index(tap_card_data.card_key_id) == -1) {
        uint32_t stored_family_id = U32_READ_BE_ARRAY(get_family_id());
        if (stored_family_id != U32_READ_BE_ARRAY(tap_card_data.family_id) &&
            stored_family_id != 0xFFFFFFFF) {
          // exit if device already paired with another set
          reset_flow_level();
        } else {
          // initiate pairing if not paired with any card
          tap_card_data.desktop_control =
              false;    // moving to pairing; we don't want to convey anything
                        // to desktop
          tap_card_take_to_pairing();
        }
      } else {
        reset_flow_level();
      }
      break;

    case VERIFY_CARD_FAILED:
      reset_flow_level();
      break;

    default:
      break;
  }
}
