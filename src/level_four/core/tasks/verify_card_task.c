/**
 * @file    verify_card_task.c
 * @author  Cypherock X1 Team
 * @brief   Verify card task (main).
 *          This file contains the main function of the verify card task for main application.
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
#include "tasks_level_four.h"
#include "ui_instruction.h"
#include "ui_delay.h"
#include "ui_confirmation.h"
#include "communication.h"
#include "application_startup.h"

extern Flow_level flow_level;
extern Counter counter;
extern Wallet wallet;

#ifndef PROVISIONING_FIRMWARE
static lv_task_t *random_number_task;
static lv_task_t *session_timeout_task;
static void __timeout_listener(lv_task_t *task);
static void __desktop_listener(lv_task_t *task);
#endif /* PROVISIONING_FIRMWARE */

extern uint8_t auth_card_number;

void verify_card_task(void)
{
#ifndef PROVISIONING_FIRMWARE
	char display[40];
#endif /* PROVISIONING_FIRMWARE */
	switch(flow_level.level_three)
	{
		case VERIFY_CARD_START_MESSAGE:
		{
#ifndef PROVISIONING_FIRMWARE
			if (TRAINING_COMPLETE == IS_TRAINING_COMPLETE)
			{
				snprintf(display, sizeof(display), ui_text_place_card_wait_for_beep, 2);
				instruction_scr_init(display, ui_text_tap_1_2_cards);
			}
			else
#endif /* PROVISIONING_FIRMWARE */
			{
				char str[50], heading[50];
				snprintf(str, sizeof(str), ui_text_place_card_wait_for_beep, 3);
				snprintf(heading, sizeof(heading), ui_text_place_card_x_heading, auth_card_number);
				instruction_scr_init(str,heading);
			}

			mark_event_over();
			break;
		}

		case VERIFY_CARD_ESTABLISH_CONNECTION_FRONTEND:
		{
#ifndef PROVISIONING_FIRMWARE
			if (TRAINING_COMPLETE == IS_TRAINING_COMPLETE)
			{
				instruction_scr_destructor();
				snprintf(display, sizeof(display), ui_text_place_card_wait_for_beep, 2);
				instruction_scr_init(display, ui_text_tap_1_2_cards);
				mark_event_over();
			}
			else
#endif /* PROVISIONING_FIRMWARE */
			{
				char str[50], heading[50];
				snprintf(str, sizeof(str), ui_text_place_card_wait_for_beep, 3);
				snprintf(heading, sizeof(heading), ui_text_place_card_x_heading,
						auth_card_number);
				instruction_scr_destructor();
				instruction_scr_init("Dummy", heading);
				instruction_scr_change_text(str,true);
				BSP_DelayMs(500);
			}

			mark_event_over();
			break;
		}

		case VERIFY_CARD_ESTABLISH_CONNECTION_BACKEND:
		{
			mark_event_over();
			break;
		}

		case VERIFY_CARD_FETCH_RANDOM_NUMBER:
		{
#ifndef PROVISIONING_FIRMWARE
			if (TRAINING_COMPLETE == IS_TRAINING_COMPLETE)
			{
				random_number_task = lv_task_create(__desktop_listener, 80, LV_TASK_PRIO_MID, NULL);
				lv_task_ready(random_number_task);
				session_timeout_task = lv_task_create(__timeout_listener, 10000, LV_TASK_PRIO_MID, NULL);
				lv_task_once(session_timeout_task);
			}
			else
#endif /* PROVISIONING_FIRMWARE */
			{
				mark_event_over();
			}

			break;
		}

		case VERIFY_CARD_SIGN_RANDOM_NUMBER_FRONTEND:
		{
#ifndef PROVISIONING_FIRMWARE
			if (TRAINING_COMPLETE == IS_TRAINING_COMPLETE)
			{
				instruction_scr_destructor();
				snprintf(display, sizeof(display), ui_text_place_card_wait_for_beep, 1);
				instruction_scr_init(display, ui_text_tap_1_2_cards);
			}
			else
#endif /* PROVISIONING_FIRMWARE */
			{
				char str[50], heading[50];
				snprintf(str, sizeof(str), ui_text_place_card_wait_for_beep, 2);
				snprintf(heading, sizeof(heading), ui_text_place_card_x_heading, auth_card_number);
				instruction_scr_destructor();
				instruction_scr_init("Dummy", heading);
				instruction_scr_change_text(str, true);
				BSP_DelayMs(500);
			}

			mark_event_over();
			break;
		}

		case VERIFY_CARD_SIGN_RANDOM_NUMBER_BACKEND:
		{
			mark_event_over();
			break;
		}

		case VERIFY_CARD_AUTH_STATUS:
		{
			mark_event_over();
			break;
		}

		case VERIFY_CARD_PAIR_FRONTEND:
		{
			char str[50], heading[50];
			snprintf(str, sizeof(str), ui_text_place_card_wait_for_beep, 1);
			snprintf(heading, sizeof(heading), ui_text_place_card_x_heading, auth_card_number);
			instruction_scr_destructor();
			instruction_scr_init("Dummy", heading);
			instruction_scr_change_text(str, true);
			BSP_DelayMs(500);
			mark_event_over();
			break;
		}

		case VERIFY_CARD_PAIR_BACKEND:
		{
			mark_event_over();
			break;
		}

		case VERIFY_CARD_FINAL_MESSAGE:
		{
#ifndef PROVISIONING_FIRMWARE
			if (TRAINING_COMPLETE == IS_TRAINING_COMPLETE)
			{
				random_number_task = lv_task_create(__desktop_listener, 80, LV_TASK_PRIO_MID, NULL);
				lv_task_ready(random_number_task);
				session_timeout_task = lv_task_create(__timeout_listener, 1000, LV_TASK_PRIO_MID, NULL);
				lv_task_once(session_timeout_task);
			}
			else
#endif /* PROVISIONING_FIRMWARE */
			{
				mark_event_over();
			}

			break;
		}

        /** These states are only called if training is complete:
         * @VERIFY_CARD_SUCCESS
         */
		case VERIFY_CARD_SUCCESS:
		{
			instruction_scr_destructor();
			delay_scr_init(ui_text_card_authentication_success, DELAY_TIME);
			CY_Reset_Not_Allow(true);
			break;
		}

		case VERIFY_CARD_FAILED:
		{
			instruction_scr_destructor();
			delay_scr_init(ui_text_card_authentication_failed, DELAY_TIME);

#ifndef PROVISIONING_FIRMWARE
			if (TRAINING_COMPLETE == IS_TRAINING_COMPLETE)
			{
				CY_Reset_Not_Allow(true);
			}
#endif /* PROVISIONING_FIRMWARE */

			break;
		}

		default:
		{
			break;
		}
	}
}

#ifndef PROVISIONING_FIRMWARE
static void __desktop_listener(lv_task_t *task)
{
    if (get_usb_msg_by_cmd_type(APP_SEND_RAND_NUM, NULL, NULL)) {
        lv_task_del(session_timeout_task);
        lv_task_del(random_number_task);            
        mark_event_over();
    }

    if (get_usb_msg_by_cmd_type(STATUS_PACKET, NULL, NULL)) {
        lv_task_del(session_timeout_task);
        lv_task_del(random_number_task);
        mark_event_over();
    }
}

static void __timeout_listener(lv_task_t *task)
{
    mark_error_screen(ui_text_no_response_from_desktop);
    instruction_scr_destructor();
    reset_flow_level();
    lv_task_del(random_number_task);
    lv_task_del(session_timeout_task);
}
#endif /* PROVISIONING_FIRMWARE */
