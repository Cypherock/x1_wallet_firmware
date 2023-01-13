/**
 * @file    tasks_level_one.c
 * @author  Cypherock X1 Team
 * @brief   Level one task (main).
 *          Handles display updates for level one tasks of the main application.
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
#include "tasks_level_one.h"
#include "apdu.h"
#include "application_startup.h"
#include "main_menu_tasks.h"
#include "onboarding_tasks.h"
#include "tasks_level_two.h"
#include "tasks_tap_cards.h"
#include "ui_confirmation.h"
#include "ui_delay.h"
#include "ui_instruction.h"
#include "ui_menu.h"
#include "ui_message.h"
#include "ui_multi_instruction.h"

extern uint8_t device_auth_flag;

void level_one_tasks(void) {
#ifdef PROVISIONING_FIRMWARE
    // It is safe to allow reset as all the flows are desktop triggered.
    CY_Reset_Not_Allow(false);
    // Since all desktop flows are handled via advanced settings, we can safely say that the app is busy.
    mark_device_state(flow_level.level_one == LEVEL_TWO_ADVANCED_SETTINGS
                          ? (CY_TRIGGER_SOURCE | CY_APP_IDLE)
                          : (CY_APP_IDLE_TASK | CY_APP_IDLE),
                      0xFF);
#endif /* PROVISIONING_FIRMWARE */

    /**
     * Check if any error occured using flow_level.show_error_screen 
     * and display message using flow_level.error_screen_text and return
     */
    if (flow_level.show_error_screen) {
        if (TRAINING_COMPLETE == IS_TRAINING_COMPLETE) {
            mark_device_state(CY_TRIGGER_SOURCE | CY_APP_WAIT_USER_INPUT, 0xFF);
        }

        message_scr_init(flow_level.error_screen_text);
        return;
    }

#ifndef PROVISIONING_FIRMWARE
    /**
     * Check card health using get_card_data_health(), show warning sign if card health
     * is DATA_HEALTH_CORRUPT and return
     */
    if (get_card_data_health() == DATA_HEALTH_CORRUPT) {
        mark_device_state(CY_TRIGGER_SOURCE | CY_APP_WAIT_USER_INPUT, 0xFF);
        message_scr_init(ui_text_unreliable_cards);
        return;
    }
#endif /* PROVISIONING_FIRMWARE */

    /** 
     * Check if there is any confirmation required for the current task or controller 
     * using flow_level.show_desktop_start_screen and display message using
     * flow_level.confirmation_screen_text and return
     */
    if (flow_level.show_desktop_start_screen) {
        mark_device_state(CY_TRIGGER_SOURCE | CY_APP_WAIT_USER_INPUT, 0);
        confirm_scr_init(flow_level.confirmation_screen_text);
        return;
    }

#ifndef PROVISIONING_FIRMWARE
    if (TRAINING_COMPLETE == IS_TRAINING_COMPLETE) {
        if (device_auth_flag) {
            mark_event_over();
            return;
        }

        if (device_auth_check() != DEVICE_AUTHENTICATED) {
            restrict_app();
            return;
        }
    }

    /** 
     * If LEVEL_ONE == counter.level
     * Check if onboarding is complete and decide to call main_menu_tasks()
     * or onboarding_tasks()
     */
    if (counter.level == LEVEL_ONE) {
        if (TRAINING_COMPLETE == IS_TRAINING_COMPLETE) {
            main_menu_tasks();
        } else {
            onboarding_tasks();
        }
    }
#else
    /* TODO: Revisit provisioning_tasks() */
    if (counter.level == LEVEL_ONE) {
        // onboarding_tasks();
    }   
#endif /* PROVISIONING_FIRMWARE */

    /* Go to higher levels if counter.level > LEVEL_ONE */
    if (counter.level > LEVEL_ONE) {
        level_two_tasks();
        return;
    }
}
