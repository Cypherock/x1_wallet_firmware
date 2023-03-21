/**
 * @file    export_wallet_controller.c
 * @author  Cypherock X1 Team
 * @brief   Export wallet next controller.
 *          Handles post event (only next events) operations for export wallet flow initiated by desktop app.
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
#include "communication.h"
#include "controller_level_four.h"
#include "ui_instruction.h"

Export_Wallet_Data data;

uint8_t serialize_cmd_export_wallet(uint8_t serialized_data[],
                                    Cmd_Export_Wallet_t raw_data);

void export_wallet_controller() {
  switch (flow_level.level_three) {
    case EXPORT_WALLET_SELECT_WALLET: {
      data.chosen_wallet_index = flow_level.screen_input.list_choice - 1;
      if (get_ith_wallet_to_export(flow_level.screen_input.list_choice - 1,
                                   &data.chosen_wallet_index) != SUCCESS_) {
        LOG_ERROR("ERR: xx1");
        comm_reject_request(SEND_WALLET_TO_DESKTOP, 0);
        reset_flow_level();
        mark_error_screen(ui_text_something_went_wrong);
        flow_level.show_error_screen = true;
        break;
      }
      Cmd_Export_Wallet_t out_data;
      memcpy(out_data.wallet_name, get_wallet_name(data.chosen_wallet_index),
             NAME_SIZE);
      out_data.wallet_info = get_wallet_info(data.chosen_wallet_index);
      memcpy(out_data.wallet_id, get_wallet_id(data.chosen_wallet_index),
             WALLET_ID_SIZE);

      uint8_t out_arr[sizeof(Cmd_Export_Wallet_t)];
      serialize_cmd_export_wallet(out_arr, out_data);
      transmit_data_to_app(SEND_WALLET_TO_DESKTOP, out_arr, sizeof(out_arr));
      flow_level.level_three = EXPORT_WALLET_FINAL_SCREEN;
    } break;

    case EXPORT_WALLET_FINAL_SCREEN:
      reset_flow_level();
      break;
    default:
      break;
  }
}

/** Format :
 * uint8_t serialize_name_of_struct(...)
 */
uint8_t serialize_cmd_export_wallet(uint8_t serialized_data[],
                                    Cmd_Export_Wallet_t raw_data) {
  uint8_t index = 0;

  memcpy(serialized_data + index, raw_data.wallet_name, NAME_SIZE);
  index += NAME_SIZE;

  memcpy(serialized_data + index, &raw_data.wallet_info, 1);
  index++;

  memcpy(serialized_data + index, raw_data.wallet_id, WALLET_ID_SIZE);
  index += WALLET_ID_SIZE;

  return index;
}
