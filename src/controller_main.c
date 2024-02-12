/**
 * @file    controller_main.c
 * @author  Cypherock X1 Team
 * @brief   Main controller.
 *          This file defines global variables and functions used by other
 *flows.
 * @details
 * This is main file for controller module.
 *
 * Controller module is divided into levels which are traversed recursively.
 *
 * Each state of the device is uniquely represented by two variables counter
 * flow_level.
 *
 * Each level has a task file and a controller file. The task file contains
 * task such as showing a particular screen. The controller file decides which
 * task it to be executed next. Sometimes the controller file needs to take
 *decision based on the input by user as to which screen needs to be shown next.
 *
 * The change of global Flow_level and Counter variable must be done in
 *controller files if possible.
 *
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

/**
 * @defgroup controller_main The event handler
 * @brief This the heart of event handling logic. All the reactions (handling)
 * to an user action (events) is done via respective controllers.
 * @{
 */

#include "controller_main.h"

#include <string.h>

#include "application_startup.h"
#include "arbitrum.h"
#include "avalanche.h"
#include "bsc.h"
#include "chacha20poly1305.h"
#include "communication.h"
#include "constant_texts.h"
#include "controller_level_four.h"
#include "cryptoauthlib.h"
#include "etc.h"
#include "eth.h"
#include "fantom.h"
#include "harmony.h"
#include "near.h"
#include "optimism.h"
#include "pb_decode.h"
#include "polygon.h"
#include "rfc7539.h"
#include "ui_events.h"
#include "ui_instruction.h"

/**
 * @brief A task declared to execute a callback after a timeout.
 *
 * This task is called when the user is prompted to wait while an action is
 * being performed in background. It executes the callback function
 * _timeout_listener once and resets the flow of the device.
 */
lv_task_t *timeout_task;

/**
 * @brief A task declared to execute a callback after timeout when displaying
 * scrolling address.
 *
 * This timeout task in used in ui_address for showing the hidden buttons after
 * timeout when user verifies the scrolling receipt address.
 */
lv_task_t *address_timeout_task = NULL;

/// Used to determine the state of authentication
uint8_t device_auth_flag = 0;

/// Used to track user inactivity
uint32_t inactivity_counter = 0;

/// Used to determine if the application is ready
bool main_app_ready = false;

/// lvgl task to listen for desktop start command
lv_task_t *listener_task;

/// Stores arbitrary data during flows
char arbitrary_data[4096 / 8 + 1];

// TODO: Variable required for code compilation: Delete after all coins are
// ported
Send_Transaction_Data var_send_transaction_data = {0};
Receive_Transaction_Data receive_transaction_data = {0};

/**
 * @brief Global Flow_level instance.
 *        Will be used in other files using getter function
 */
Flow_level CONFIDENTIAL flow_level;

/**
 * @brief Global Counter instance.
 *        Will be used in other files using getter function
 */
Counter counter;

/**
 * @brief Global Flash_Wallet instance
 *
 */
Flash_Wallet wallet_for_flash;

/**
 * @brief Message data for wallet connect EIP-712 using protobuf
 *
 */
MessageData msg_data;
ui_display_node *current_display_node = NULL;

Flash_Wallet *get_flash_wallet() {
  ASSERT((&wallet_for_flash) != NULL);

  return &wallet_for_flash;
}

void mark_event_over() {
  counter.next_event_flag = true;
}

void mark_list_choice(uint16_t list_choice) {
  flow_level.screen_input.list_choice = list_choice;
}

void mark_event_cancel() {
  counter.next_event_flag = true;
}

void reset_flow_level() {
  // reset device state
  CY_Reset_Not_Allow(true);
  CY_Set_External_Triggered(false);
  if (main_app_ready) {
    mark_device_state(CY_APP_IDLE_TASK | CY_APP_IDLE, 0);
  }

  // clear level one task flags
  counter.next_event_flag = true;
  reset_cancel_event_flag();
  device_auth_flag = false;
  flow_level.show_desktop_start_screen = false;

  // restore flow to main menu
  counter.level = LEVEL_ONE;
  flow_level.level_one = 1;
  flow_level.level_two = 1;
  flow_level.level_three = 1;
  flow_level.level_four = 1;
  flow_level.level_five = 1;

  // clear memory
  memzero(wallet.password_double_hash, sizeof(wallet.password_double_hash));
  memzero(wallet_credential_data.passphrase,
          sizeof(wallet_credential_data.passphrase));
  cy_free();
  pb_release(MessageData_fields, &msg_data);
  current_display_node = NULL;
}

void reset_next_event_flag() {
  counter.next_event_flag = false;
}

void increase_level_counter() {
  counter.level++;
}

void clear_list_choice() {
  flow_level.screen_input.list_choice = 0x00;
}

void decrease_level_counter() {
  counter.level--;
}

void mark_error_screen(const char *error_msg) {
  ASSERT(error_msg != NULL);

  flow_level.show_error_screen = true;
  snprintf(flow_level.error_screen_text,
           sizeof(flow_level.error_screen_text),
           "%s",
           error_msg);
}

void reset_cancel_event_flag() {
  counter.previous_event_flag = 0;
}

void mark_input(char *text) {
  ASSERT(text != NULL);
  snprintf(flow_level.screen_input.input_text,
           sizeof(flow_level.screen_input.input_text),
           "%s",
           text);
}

void mark_expected_list_choice(uint8_t expected_list_choice) {
  flow_level.screen_input.expected_list_choice = expected_list_choice;
}

// TODO: fill all these values from flash
void set_wallet_init() {
  wallet.number_of_mnemonics = MAX_NUMBER_OF_MNEMONIC_WORDS;
  wallet.minimum_number_of_shares = MINIMUM_NO_OF_SHARES;
  wallet.total_number_of_shares = TOTAL_NUMBER_OF_SHARES;
}

void reset_flow_level_greater_than(enum LEVEL level) {
  switch (level) {
    default:
      reset_flow_level();
      break;

    case LEVEL_ONE:
      flow_level.level_two = 1;
    case LEVEL_TWO:
      flow_level.level_three = 1;
    case LEVEL_THREE:
      flow_level.level_four = 1;
    case LEVEL_FOUR:
      flow_level.level_five = 1;
      break;
  }
}

void _timeout_listener(lv_task_t *task) {
  mark_error_screen(ui_text_no_response_from_desktop);
  instruction_scr_destructor();
  reset_flow_level();
}

void desktop_listener_task(lv_task_t *data) {
  En_command_type_t command;
  uint8_t *data_array = NULL;
  uint16_t msg_size = 0;
  if (is_device_ready() && get_usb_msg(&command, &data_array, &msg_size)) {
    switch (command) {
#if X1WALLET_MAIN
#ifdef DEV_BUILD
      case START_CARD_UPGRADE: {
        CY_Reset_Not_Allow(false);
        snprintf(flow_level.confirmation_screen_text,
                 sizeof(flow_level.confirmation_screen_text),
                 "%s",
                 ui_text_start_card_update);
        flow_level.level_one = LEVEL_TWO_ADVANCED_SETTINGS;
        flow_level.show_desktop_start_screen = true;
        flow_level.level_two = LEVEL_THREE_CARD_UPGRADE;
        clear_message_received_data();
      } break;
#endif
      case SEND_TXN_START: {
        clear_message_received_data();
      } break;

      case SIGN_MSG_START: {
        clear_message_received_data();
      } break;

      case RECV_TXN_START: {
        clear_message_received_data();
      } break;
#elif X1WALLET_INITIAL
      case START_DEVICE_PROVISION: {    // 81,02 success and external keys sent
                                        // to device, 81,03 failure
        CY_Set_External_Triggered(true);
        switch (data_array[0]) {
          case 1: {
            if (msg_size < 5)
              break;
            lv_obj_clean(lv_scr_act());
            memcpy(provision_date, data_array + 1, 4);
            counter.level = LEVEL_THREE;
            flow_level.level_one = LEVEL_TWO_ADVANCED_SETTINGS;
            flow_level.level_two = LEVEL_THREE_START_DEVICE_PROVISION;
            flow_level.level_three = data_array[0];
            counter.next_event_flag = true;
            lv_task_set_prio(listener_task,
                             LV_TASK_PRIO_OFF);    // Tasks will now not run
          } break;
          case 2:
            if (msg_size < 183)
              break;
            memcpy(&provision_keys_data,
                   data_array + 1,
                   sizeof(Provision_Data_struct));
          case 3:
            counter.level = LEVEL_THREE;
            flow_level.level_one = LEVEL_TWO_ADVANCED_SETTINGS;
            flow_level.level_two = LEVEL_THREE_START_DEVICE_PROVISION;
            flow_level.level_three = data_array[0];
            counter.next_event_flag = true;
            lv_task_set_prio(listener_task,
                             LV_TASK_PRIO_OFF);    // Tasks will now not run
            break;

          default:
            cy_exit_flow();
        }
        clear_message_received_data();
      } break;
#else
#error Specify what to build (X1WALLET_INITIAL or X1WALLET_MAIN)
#endif
      case LIST_SUPPORTED_COINS: {
        clear_message_received_data();
      } break;

      default:
        clear_message_received_data();
        comm_reject_invalid_cmd();
        break;
    }

    if (flow_level.show_desktop_start_screen) {
      CY_Set_External_Triggered(true);
      counter.level = LEVEL_THREE;
      clear_message_received_data();
      counter.next_event_flag = true;
      lv_obj_clean(lv_scr_act());
    }
  }
}

void cy_exit_flow() {
  if (address_timeout_task != NULL)
    address_timeout_task->task_cb(NULL);

  lv_obj_clean(lv_scr_act());
  sys_flow_cntrl_u.bits.reset_flow = false;
  reset_flow_level();
#if X1WALLET_INITIAL
  flow_level.level_one = 6;
#endif
  counter.next_event_flag = true;
}

/** @} */    // end of Controller