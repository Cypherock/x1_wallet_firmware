/**
 * @file    reconstruct_from_four_cards_to_verify_controller.c
 * @author  Cypherock X1 Team
 * @brief   Reconstruct from all cards.
 *          This file contains the implementation of the function that
 *          reconstructs from all cards for the verification of the wallet.
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
#include "controller_main.h"
#include "controller_tap_cards.h"
#include "card_action_controllers.h"

extern Wallet_shamir_data wallet_shamir_data;

void tap_cards_for_verification_flow_controller()
{
    switch (flow_level.level_four) {
    case TAP_CARD_ONE_FRONTEND:
        tap_card_data.desktop_control = false;
        tap_card_data.tapped_card = 0;
        flow_level.level_four = TAP_CARD_ONE_BACKEND;
        break;

    case TAP_CARD_ONE_BACKEND:
        readback_share_from_card(0);
        break;

    case TAP_CARD_TWO_FRONTEND:
        flow_level.level_four = TAP_CARD_TWO_BACKEND;
        break;

    case TAP_CARD_TWO_BACKEND:
        readback_share_from_card(1);
        break;

    case TAP_CARD_THREE_FRONTEND:
        flow_level.level_four = TAP_CARD_THREE_BACKEND;
        break;

    case TAP_CARD_THREE_BACKEND:
        readback_share_from_card(2);
        break;

    case TAP_CARD_FOUR_FRONTEND:
        flow_level.level_four = TAP_CARD_FOUR_BACKEND;
        break;

    case TAP_CARD_FOUR_BACKEND:
        readback_share_from_card(3);
        break;

    default:
        LOG_CRITICAL("###31");
        reset_flow_level();
        break;
    }
}
