/**
 * @file    initial_device_provision_tasks.c
 * @author  Cypherock X1 Team
 * @brief   Device provision task (initial).
 *          This file contains the implementation of the device provision task
 *for initial application.
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
#include "constant_texts.h"
#include "controller_level_four.h"
#include "tasks_level_four.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "ui_instruction.h"
#include "ui_menu.h"
#include "ui_message.h"

void task_device_provision() {
#if X1WALLET_INITIAL
  switch (flow_level.level_three) {
    case GENERATE_PROVSION_DATA: {
      ui_text_slideshow_init(ui_text_device_provisioning, 5, 500, false);
      mark_event_over();
    } break;

    case PROVISION_STATUS_WAIT:
    case PROVISION_SAVE_EXT_KEYS: {
      mark_event_over();
    } break;

    case PROVISION_UNSUCCESSFUL: {
      lv_obj_clean(lv_scr_act());
      delay_scr_init(ui_text_provision_fail, DELAY_TIME);
    } break;

    default:
      break;
  }
#endif
}