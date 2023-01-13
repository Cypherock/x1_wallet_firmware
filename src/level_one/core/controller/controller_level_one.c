/**
 * @file    controller_level_one.c
 * @author  Cypherock X1 Team
 * @brief   Level one next controller (main).
 *          Handles post event (only next events) operations for level one
 *          tasks of the main application.
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
#include "controller_level_one.h"
#include "application_startup.h"
#include "controller_level_two.h"
#include "main_menu_controller.h"
#include "onboarding_controller.h"
#include "apdu.h"

extern lv_task_t *listener_task;
extern uint8_t device_auth_flag;
extern bool main_app_ready;
extern char card_id_fetched[];
extern char card_version[];

void level_one_controller(void) {
    /** 
     * Reset flow_level.show_error_screen & flow_level.show_desktop_start_screen
     * as the corresponding UI element is already displayed by level_one_tasks()
     * and level_one_controller() is invoked by an input event callback
     */
    if (flow_level.show_error_screen) {
        flow_level.show_error_screen = false;
        return;
    }

    if (flow_level.show_desktop_start_screen) {
        flow_level.show_desktop_start_screen = false;
        return;
    }

#ifndef PROVISIONING_FIRMWARE
    /* Reset card health using reset_card_data_health() if previous state was DATA_HEALTH_CORRUPT */
    if (get_card_data_health() == DATA_HEALTH_CORRUPT) {
        reset_card_data_health();
        return;
    }

    if (!main_app_ready) {
        device_auth_flag = 0;
        return;
    }

    if (device_auth_flag) {
        device_auth();
        return;
    }

    if (LEVEL_ONE == counter.level) {
        if (TRAINING_COMPLETE == IS_TRAINING_COMPLETE) {
            main_menu_controller();
        } else {
            onboarding_controller();
        }
    }
#else
    /* TODO: Revisit provisioning_controller() */
    if (counter.level == LEVEL_ONE) {
        onboarding_controller();
    }
#endif /* PROVISIONING_FIRMWARE */

    /* Go to higher levels if counter.level > LEVEL_ONE */
    if (counter.level > LEVEL_ONE) {
        level_two_controller();
        return;
    }
}
