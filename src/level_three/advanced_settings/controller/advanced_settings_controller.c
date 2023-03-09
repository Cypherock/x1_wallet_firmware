/**
 * @file    advanced_settings_controller.c
 * @author  Cypherock X1 Team
 * @brief   Advanced settings next controller.
 *          Handles post event (only next events) operations for advanced settings flow.
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
#include "controller_level_four.h"
#include "controller_main.h"
#include "tasks.h"
#include "controller_tap_cards.h"
#include "controller_advanced_settings.h"
#include "application_startup.h"
#include "cy_factory_reset.h"
#include "cy_card_hc.h"
#include <stdio.h>

extern lv_task_t* timeout_task;

void level_three_advanced_settings_controller()
{
    switch (flow_level.level_two) {
    case LEVEL_THREE_RESET_DEVICE_CONFIRM: {
        flow_level.level_two = LEVEL_THREE_RESET_DEVICE;
    } break;
#if X1WALLET_MAIN
    case LEVEL_THREE_SYNC_CARD_CONFIRM: {
        flow_level.level_two = LEVEL_THREE_SYNC_CARD;
    } break;

    case LEVEL_THREE_ROTATE_SCREEN_CONFIRM: {
        
        flow_level.level_two = LEVEL_THREE_ROTATE_SCREEN ;
    } break;

    case LEVEL_THREE_TOGGLE_PASSPHRASE: {
        set_enable_passphrase(is_passphrase_enabled() ? PASSPHRASE_DISABLED : PASSPHRASE_ENABLED, FLASH_SAVE_NOW);
        flow_level.level_two = 1;
        counter.level = LEVEL_TWO;
    } break;

    case LEVEL_THREE_FACTORY_RESET:
        cyc_factory_reset();
        break;

    case LEVEL_THREE_CARD_HEALTH_CHECK:
        cyc_card_hc();
        break;
#endif

    case LEVEL_THREE_VIEW_DEVICE_VERSION:{
        counter.level = LEVEL_TWO;
        flow_level.level_two = 1;
    } break;
    case LEVEL_THREE_VERIFY_CARD:
#if X1WALLET_MAIN
        verify_card_controller();
#elif X1WALLET_INITIAL
        initial_verify_card_controller();
#else
#error Specify what to build (X1WALLET_INITIAL or X1WALLET_MAIN)
#endif
        break;

    case LEVEL_THREE_READ_CARD_VERSION: {
        controller_read_card_id();
    } break;
#if X1WALLET_MAIN
#ifdef DEV_BUILD
    case LEVEL_THREE_UPDATE_CARD_ID: {
        controller_update_card_id();
    } break;

    case LEVEL_THREE_CARD_UPGRADE:
        card_upgrade_controller();
        break;

    case LEVEL_THREE_ADJUST_BUZZER: {
#if USE_SIMULATOR == 0
        buzzer_disabled = flow_level.screen_input.list_choice == 1;
#endif
        counter.level = LEVEL_TWO;
        flow_level.level_two = 1;
    } break;
#endif

    case LEVEL_THREE_SYNC_CARD: {
        tap_a_card_and_sync_controller();
    } break;

    case LEVEL_THREE_SYNC_SELECT_WALLET:{
        uint8_t index = -1;
        flow_level.level_one = LEVEL_TWO_ADVANCED_SETTINGS;
        flow_level.level_two = LEVEL_THREE_SYNC_WALLET_FLOW;
        flow_level.level_three = SYNC_CARDS_SUCCESS;
        if (get_ith_wallet_without_share(0, &index) == SUCCESS_){
            memcpy(
                wallet.wallet_name,
                get_wallet_name(index), NAME_SIZE);
            wallet.wallet_info = get_wallet_info(index);
            flow_level.level_one = LEVEL_TWO_ADVANCED_SETTINGS;
            flow_level.level_two = LEVEL_THREE_SYNC_WALLET_FLOW;
            flow_level.level_three = SYNC_CARDS_START;
            break;
        }
    } break;
    case LEVEL_THREE_SYNC_WALLET_FLOW:{
        sync_cards_controller();
    } break;

    case LEVEL_THREE_ROTATE_SCREEN: {
        flow_level.level_one = LEVEL_TWO_ADVANCED_SETTINGS;
        counter.level = LEVEL_TWO;
    } break;
#endif
    case LEVEL_THREE_RESET_DEVICE: {
    } break;

#ifdef ALLOW_LOG_EXPORT
    case LEVEL_THREE_FETCH_LOGS_INIT: {
        set_start_log_read();
        logger_task();
        flow_level.level_two = LEVEL_THREE_FETCH_LOGS_WAIT;
    } break;

    case LEVEL_THREE_FETCH_LOGS_WAIT: {
        if (get_usb_msg_by_cmd_type(APP_LOG_DATA_SEND, NULL, NULL)) {
            flow_level.level_two = LEVEL_THREE_FETCH_LOGS;
            clear_message_received_data();
        }
    } break;

    case LEVEL_THREE_FETCH_LOGS: {
        if (get_log_read_status() == LOG_READ_FINISH) {
            // logs finished, reset any data and proceed
            flow_level.level_two = LEVEL_THREE_FETCH_LOGS_FINISH;
        } else {
            flow_level.level_two = LEVEL_THREE_FETCH_LOGS_WAIT;
        }
    } break;

    case LEVEL_THREE_FETCH_LOGS_FINISH: {
        reset_flow_level();
#if X1WALLET_INITIAL
        flow_level.level_one = 6;
#endif
    } break;
#endif

#if X1WALLET_INITIAL
    case LEVEL_THREE_START_DEVICE_PROVISION: {
        device_provision_controller();
    } break;

    case LEVEL_THREE_START_DEVICE_AUTHENTICATION: {
        device_authentication_controller();
    } break;
#elif X1WALLET_MAIN
    case LEVEL_THREE_PAIR_CARD: {
        tap_card_pair_card_controller();
    } break;

    case LEVEL_THREE_TOGGLE_LOGGING: {
        set_logging_config(is_logging_enabled() ?  LOGGING_DISABLED : LOGGING_ENABLED, FLASH_SAVE_NOW);
        counter.level = LEVEL_TWO;
        flow_level.level_two = 1;
    } break;
#else
#error Specify what to build (X1WALLET_INITIAL or X1WALLET_MAIN)
#endif
    default:
        break;
    }
}
