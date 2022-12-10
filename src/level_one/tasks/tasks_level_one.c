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
#include "application_startup.h"
#include "tasks_level_one.h"
#include "flash_api.h"
#include "tasks_level_two.h"
#include "ui_confirmation.h"
#include "ui_menu.h"
#include "ui_message.h"
#include "apdu.h"
#include "controller_level_one.h" /* fixme*/
#include "flash_struct.h"
#include "tasks_tap_cards.h"
#include "ui_delay.h"
#include "ui_instruction.h"
#include "ui_multi_instruction.h"

/*	Global variables
*******************************************************************************/
extern lv_task_t* listener_task;
extern uint8_t device_auth_flag;

static void tasks_level_one_training_FSM(void);
static void tasks_level_one_display_wallet_menu(void);

void level_one_tasks(void)
{
	if (IS_TRAINING_DONE != TRAINING_DONE)
	{
		// It is safe to allow reset as all the flows are desktop triggered.
		CY_Reset_Not_Allow(false);
		// Since all desktop flows are handled via advanced settings, we can safely say that the app is busy.
		mark_device_state(	flow_level.level_one == LEVEL_TWO_ADVANCED_SETTINGS
							? (CY_TRIGGER_SOURCE | CY_APP_IDLE)
							: (CY_APP_IDLE_TASK | CY_APP_IDLE),
							0xFF);
	}


    if (flow_level.show_error_screen)
    {
		if (IS_TRAINING_DONE == TRAINING_DONE)
		{
        	mark_device_state(CY_TRIGGER_SOURCE | CY_APP_WAIT_USER_INPUT, 0xFF);
		}

		message_scr_init(flow_level.error_screen_text);
        return;
    }

	if (IS_TRAINING_DONE == TRAINING_DONE)
	{
		if (get_card_data_health() == DATA_HEALTH_CORRUPT)
		{
			mark_device_state(CY_TRIGGER_SOURCE | CY_APP_WAIT_USER_INPUT, 0xFF);
			message_scr_init(ui_text_unreliable_cards);
			return;
		}
	}

    if (flow_level.show_desktop_start_screen)
    {
		if (IS_TRAINING_DONE == TRAINING_DONE)
		{
			mark_device_state(CY_TRIGGER_SOURCE | CY_APP_WAIT_USER_INPUT, 0);
			confirm_scr_init(flow_level.confirmation_screen_text);
		}
		else
		{
			if(flow_level.level_two == LEVEL_THREE_RESET_DEVICE_CONFIRM)
			{
				message_scr_init(flow_level.confirmation_screen_text);
			}
			else
			{
				confirm_scr_init(flow_level.confirmation_screen_text);
			}
		}
        return;
    }

	if (IS_TRAINING_DONE == TRAINING_DONE)
	{
		if (device_auth_flag)
		{
			mark_event_over();
			return;
		}
	}

    if (counter.level > LEVEL_ONE) {
        level_two_tasks();
        return;
    }

	/** 
	 * If training is complete, X1 Wallet comes in usable state
	 * Eenforce device authentication and display wallet menu
	 */
    if (IS_TRAINING_DONE == TRAINING_DONE)
    {
		if(device_auth_check() != DEVICE_AUTHENTICATED)
		{
			restrict_app();
			return;
		}

		tasks_level_one_display_wallet_menu();
    }
	else
	{
		tasks_level_one_training_FSM();
	}
}

static void tasks_level_one_training_FSM(void)
{
	/* lv_task_set_prio(listener_task, LV_TASK_PRIO_MID); */
	switch (flow_level.level_one)
	{
		case 1:
		{
			delay_scr_init("WELCOME", DELAY_TIME);
			break;
		}

		case 2:
		{
			multi_instruction_init(ui_text_startup_instruction_screen_2, 5U, DELAY_LONG_STRING, true);
			break;
		}

		case 3:
		{
			delay_scr_init(ui_text_tap_a_card_instruction1, DELAY_LONG_STRING);
			break;
		}

		case 4:
		{
			flow_level.level_three = TAP_ONE_CARD_TAP_A_CARD_FRONTEND;
			tasks_read_card_id();
			instruction_scr_destructor();
			instruction_scr_init(ui_text_tap_a_card_instruction2, NULL);
			break;
		}

		case 5:
		{
			flow_level.level_three = TAP_ONE_CARD_TAP_A_CARD_BACKEND;
			mark_event_over();
			break;
		}

		case 6:
		{
			/* Set priority to listener_task, as now the Cypherock X1 Wallet will interact with the desktop app */
			lv_task_set_prio(listener_task, LV_TASK_PRIO_MID);
			
			multi_instruction_init(ui_text_startup_instruction_screen_4, 2U, DELAY_TIME, false);
			break;
		}

		case 7:
		{
			instruction_scr_init(ui_text_check_cysync_app, NULL);
			break;
		}

		case 8:
		{
			instruction_scr_init(ui_text_device_verification_success, NULL);
			break;
		}

		case 9:
		{
			instruction_scr_init(ui_text_device_verification_failure, NULL);
			break;
		}

		case 10:
		{
			instruction_scr_init(ui_text_provision_success, NULL);
			break;
		}

		case 11:
		{
			instruction_scr_init(ui_text_provision_fail, NULL);
			break;
		}

    	default:
      	{
			break;
		}
  	}	
}

static void tasks_level_one_display_wallet_menu(void)
{
	uint8_t number_of_options = get_wallet_count();
	LOG_INFO("wallet count %d", number_of_options);

	// create list for chooses
	char* choices[MAX_LEN_OF_MENU_OPTIONS];

	uint8_t mainMenuIndex = 0;
	uint8_t walletIndex = 0;

	for (; walletIndex < MAX_WALLETS_ALLOWED; walletIndex++)
	{
		if  (
				get_wallet_state(walletIndex) == VALID_WALLET                       || 
				get_wallet_state(walletIndex) == UNVERIFIED_VALID_WALLET            || 
				get_wallet_state(walletIndex) == VALID_WALLET_WITHOUT_DEVICE_SHARE
			)
		{
			choices[mainMenuIndex] = (char*)get_wallet_name(walletIndex);
			mainMenuIndex++;
		}
	}

	if(mainMenuIndex != number_of_options)
	{
		number_of_options = mainMenuIndex;
	}

	number_of_options += NUMBER_OF_OPTIONS_MAIN_MENU;

	uint8_t walletMenuOptionIndex = 0;

	//initialise walletMenuOptionIndex to 0 if wallet limit is exceeded
	if(number_of_options > 5)
	{
		walletMenuOptionIndex = 2;
	}
	else
	{
		walletMenuOptionIndex = 1;
	}

	//check if wallet limit is exceeded or not
	if(number_of_options > 5)
	{
		number_of_options = 5;
	}
	// fill other options
	for (; walletMenuOptionIndex <= NUMBER_OF_OPTIONS_MAIN_MENU; walletMenuOptionIndex++)
	{
		choices[mainMenuIndex] = (char*)ui_text_options_main_menu[walletMenuOptionIndex];
		mainMenuIndex++;
	}

	menu_init((const char **) choices, number_of_options, ui_text_options_main_menu[0], false);
	lv_task_set_prio(listener_task, LV_TASK_PRIO_MID);
	CY_set_app_restricted(false);
	CY_Reset_Not_Allow(true);
	mark_device_state(CY_APP_IDLE_TASK | CY_APP_IDLE, 0xFF);
}
