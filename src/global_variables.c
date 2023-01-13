/**
 * @file    onboarding.c
 * @author  Cypherock X1 Team
 * @brief   Coin specific data.
 *          Contains functions for managing coin specific data.
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
#include "global_variables.h"

/* Global variables
 ******************************************************************************/
#ifndef PROVISIONING_FIRMWARE
Onboarding_StageData gOnboardingStg = {.StageNum = STAGE_0, .StageEncoding = STAGE_n_NOT_STARTED};
#endif /* PROVISIONING_FIRMWARE */

/**
 * Global variable gBoolOnboardingComplete determines if the device training
 * is complete or not
 * TODO: As on today, this is a hardcoded variable, logic to update the status will 
 * be added in subsequent tasks 
 */
volatile uint8_t gBoolOnboardingComplete = TRAINING_INCOMPLETE;

/**
 * @brief A task declared to periodically execute a callback which checks for a success from the desktop.
 * 
 * This task is called when the user is prompted to wait while an action is being performed in background.
 * It executes the callback function _success_listener periodically which checks for a success/abort from the desktop.
 */
lv_task_t* success_task;

/**
 * @brief A task declared to execute a callback after a timeout.
 * 
 * This task is called when the user is prompted to wait while an action is being performed in background.
 * It executes the callback function _timeout_listener once and resets the flow of the device.
 */
lv_task_t* timeout_task;

/**
 * @brief A task declared to execute a callback after timeout when displaying scrolling address.
 *
 * This timeout task in used in ui_address for showing the hidden buttons after timeout when user verifies the scrolling receipt address.
 */
lv_task_t* address_timeout_task = NULL;

/// Used to determine the state of authentication
uint8_t device_auth_flag = 0;

/// Used to track user inactivity
uint32_t inactivity_counter = 0;

/// Used to determine if the application is ready
bool main_app_ready = false;

/// lvgl task to listen for desktop start command
lv_task_t* listener_task;

/// lvgl task to listen for desktop start command in restricted mode
lv_task_t* authentication_task;

/// Stores arbitrary data during flows
char arbitrary_data[4096 / 8 + 1];

/**
 * @brief Global Flow_level instance.
 *        Will be used in other files using getter function
 */
Flow_level flow_level;
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
