/**
 * @file    controller_level_two_b.c
 * @author  Cypherock X1 Team
 * @brief   Level two back controller.
 *          Handles post event (only back/cancel events) operations for level two tasks.
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
#include "controller_add_wallet.h"
#include "controller_level_two.h"
#include "controller_main.h"
#include "controller_old_wallet.h"
#include "tasks.h"
#include "controller_level_four.h"
#include "application_startup.h"


void level_two_controller_b()
{
    switch (flow_level.level_one)
    {
        case LEVEL_TWO_OLD_WALLET:
        {
            if (counter.level > LEVEL_TWO)
            {
                level_three_old_wallet_controller_b();
                return;
            }
            
            break;
        }

    case LEVEL_TWO_NEW_WALLET:
    {
        if (counter.level > LEVEL_TWO)
        {
            if (flow_level.level_two == LEVEL_THREE_GENERATE_WALLET)
            {
                generate_wallet_controller_b();
            }
            else
            {
                restore_wallet_controller_b();
            }
            
            return;
        }

        break;
    }


    case LEVEL_TWO_ADVANCED_SETTINGS:
    {
        if (counter.level > LEVEL_TWO)
        {
            if (flow_level.level_two == LEVEL_THREE_RESET_DEVICE_CONFIRM){
            
                comm_reject_request(USER_FIRMWARE_UPGRADE_CHOICE, 0);
                reset_flow_level();
                flow_level.level_one = 6;
                counter.next_event_flag = true;
                return;
            }

            if (flow_level.level_two == LEVEL_THREE_VERIFY_CARD){
            
                comm_reject_request(START_CARD_AUTH, 0);
                reset_flow_level();
                counter.next_event_flag = true;
                return;
            }

            if (IS_TRAINING_COMPLETE == TRAINING_INCOMPLETE)
            {
                if(flow_level.level_two == LEVEL_THREE_SYNC_WALLET_FLOW)
                {
                    sync_cards_controller_b();
                    counter.next_event_flag = true;
                    return;
                }
            }
#ifdef ALLOW_LOG_EXPORT
            if (flow_level.level_two == LEVEL_THREE_FETCH_LOGS_INIT){

                comm_reject_request(APP_LOG_DATA_REJECT, 0);
                reset_flow_level();
                flow_level.level_one = 6; /* TODO: take to get-started screen */
                counter.next_event_flag = true;
                return;
            }
#endif
            if (IS_TRAINING_COMPLETE == TRAINING_COMPLETE)
            {
                if (flow_level.level_two == LEVEL_THREE_PAIR_CARD)
                {
                    if(flow_level.show_error_screen)
                    {
                        reset_flow_level();
                    }
                    else
                    {
                        switch (flow_level.level_four)
                        {
                        case PAIR_CARD_RED_FRONTEND:
                        case PAIR_CARD_BLUE_FRONTEND:
                        case PAIR_CARD_GREEN_FRONTEND:
                        case PAIR_CARD_YELLOW_FRONTEND:
                            /* Operation is sensitive as multiple switch cases have the same body */
                            flow_level.level_four += 2;
                            break;
                        
                        default:
                            reset_flow_level();
                            break;
                        }
                    }
                    return;
                }
                
                if (flow_level.level_two == LEVEL_THREE_FACTORY_RESET)
                {
                    reset_flow_level();
                    return;
                }
            }

            /**
             * As per code review, below statements handle any missed condition
             * and therefore, nesting it in else statement is not required
             * It will not break any existing flow as all happy flows end with return statement
             */
            flow_level.level_one = LEVEL_TWO_ADVANCED_SETTINGS;
            reset_flow_level_greater_than(LEVEL_ONE);
            counter.level = LEVEL_TWO;
            return;
        }
        break;
    }

    default:
    {
        break;
    }
    }
    reset_flow_level();
}
