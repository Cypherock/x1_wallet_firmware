/**
 * @file    advanced_settings_tasks.c
 * @author  Cypherock X1 Team
 * @brief   Advanced settings task.
 *          This file contains the pre-processing & rendering of the advanced
 *settings task.
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
#include "board.h"
#include "constant_texts.h"
#include "controller_level_four.h"
#include "controller_main.h"
#include "controller_tap_cards.h"
#include "cy_card_hc.h"
#include "cy_factory_reset.h"
#include "tasks.h"
#include "tasks_level_four.h"
#include "tasks_tap_cards.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_instruction.h"
#include "ui_message.h"
#include "ui_multi_instruction.h"
#include "utils.h"

extern lv_task_t *timeout_task;

extern const char *GIT_REV;
extern const char *GIT_TAG;
extern const char *GIT_BRANCH;

void level_three_advanced_settings_tasks() {
  switch (flow_level.level_two) {
    case LEVEL_THREE_RESET_DEVICE_CONFIRM: {
      transmit_one_byte_confirm(USER_FIRMWARE_UPGRADE_CHOICE);
      instruction_scr_init(ui_text_processing, NULL);
      mark_event_over();
    } break;

#if X1WALLET_MAIN
    case LEVEL_THREE_SYNC_CARD_CONFIRM: {
    } break;

    case LEVEL_THREE_ROTATE_SCREEN_CONFIRM: {
      confirm_scr_init(ui_text_rotate_display_confirm);
    } break;

    case LEVEL_THREE_TOGGLE_PASSPHRASE: {
    } break;

    case LEVEL_THREE_FACTORY_RESET:
      cyt_factory_reset();
      break;

    case LEVEL_THREE_CARD_HEALTH_CHECK:
      cyt_card_hc();
      break;
#endif

    case LEVEL_THREE_VIEW_DEVICE_VERSION: {
    } break;

    case LEVEL_THREE_VERIFY_CARD: {
#if X1WALLET_MAIN
      verify_card_task();
#elif X1WALLET_INITIAL
      initial_verify_card_task();
#else
#error Specify what to build (X1WALLET_INITIAL or X1WALLET_MAIN)
#endif
    } break;

    case LEVEL_THREE_READ_CARD_VERSION: {
    } break;

    case LEVEL_THREE_REGULATORY_INFO: {
    } break;

#if X1WALLET_MAIN
#ifdef DEV_BUILD
    case LEVEL_THREE_UPDATE_CARD_ID: {
      tasks_update_card_id();
    } break;

    case LEVEL_THREE_CARD_UPGRADE:
      card_upgrade_task();
      break;

    case LEVEL_THREE_ADJUST_BUZZER:
      menu_init(&ui_text_options_buzzer_adjust[1],
                2,
                ui_text_options_buzzer_adjust[0],
                false);
      break;
#endif

    case LEVEL_THREE_SYNC_CARD: {
    } break;

    case LEVEL_THREE_SYNC_SELECT_WALLET: {
    } break;

    case LEVEL_THREE_SYNC_WALLET_FLOW: {
    } break;

    case LEVEL_THREE_ROTATE_SCREEN: {
    } break;
#endif

    case LEVEL_THREE_RESET_DEVICE: {
      CY_Reset_Not_Allow(true);
      BSP_DelayMs(
          500);    // Wait for status pull to desktop (which requests at 200ms)
      FW_enter_DFU();
      BSP_reset();
    } break;

#ifdef ALLOW_LOG_EXPORT
    case LEVEL_THREE_FETCH_LOGS_INIT: {
      instruction_scr_init(ui_text_sending_logs, NULL);
      mark_event_over();
    } break;

    case LEVEL_THREE_FETCH_LOGS_WAIT: {
      mark_event_over();
    } break;

    case LEVEL_THREE_FETCH_LOGS: {
      mark_event_over();
    } break;

    case LEVEL_THREE_FETCH_LOGS_FINISH: {
      ui_text_slideshow_destructor();
      delay_scr_init(ui_text_logs_sent, DELAY_TIME);
      CY_Reset_Not_Allow(true);
    } break;
#endif

#if X1WALLET_INITIAL
    case LEVEL_THREE_START_DEVICE_PROVISION: {
      task_device_provision();
    } break;

    case LEVEL_THREE_START_DEVICE_AUTHENTICATION: {
      task_device_authentication();
    } break;
#elif X1WALLET_MAIN
    case LEVEL_THREE_PAIR_CARD: {
    } break;

    case LEVEL_THREE_TOGGLE_LOGGING: {
    } break;
#else
#error Specify what to build (X1WALLET_INITIAL or X1WALLET_MAIN)
#endif
    default:
      break;
  }
}
