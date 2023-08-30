/**
 * @file    tasks_old_wallet.c
 * @author  Cypherock X1 Team
 * @brief   Old wallet task.
 *          Handles pre-processing & display updates for old wallet tasks.
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
#include "tasks_old_wallet.h"

#include "constant_texts.h"
#include "controller_main.h"
#include "tasks.h"
#include "tasks_level_four.h"

void level_three_old_wallet_tasks() {
  // Check if desktop sent an abort command (view seed and delete wallet are not
  // initiated by desktop, therefore excluded here)
  if (flow_level.level_two != LEVEL_THREE_WALLET_LOCKED &&
      flow_level.level_two != LEVEL_THREE_VERIFY_WALLET &&
      CY_Read_Reset_Flow()) {
    mark_error_screen(ui_text_aborted);
    reset_flow_level();
    return;
  }

  switch (flow_level.level_two) {
    case LEVEL_THREE_SEND_TRANSACTION_ETH: {
      send_transaction_tasks_eth();
    } break;

    case LEVEL_THREE_SIGN_MESSAGE_ETH: {
      sign_message_tasks_eth();
    } break;

    case LEVEL_THREE_SEND_TRANSACTION_SOLANA: {
      send_transaction_tasks_solana();
    } break;

    case LEVEL_THREE_RECEIVE_TRANSACTION_ETH: {
      receive_transaction_tasks_eth();
    } break;

    case LEVEL_THREE_RECEIVE_TRANSACTION_NEAR: {
      receive_transaction_tasks_near();
    } break;

    case LEVEL_THREE_RECEIVE_TRANSACTION_SOLANA: {
      receive_transaction_tasks_solana();
    } break;

    case LEVEL_THREE_WALLET_LOCKED:
      wallet_locked_task();
      break;

    case LEVEL_THREE_VERIFY_WALLET:
      verify_wallet_tasks();
      break;
    case LEVEL_THREE_SYNC_WALLET: {
    } break;
    default:
      break;
  }
  return;
}
