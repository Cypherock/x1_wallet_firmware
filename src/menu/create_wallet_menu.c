/**
 * @file    create_wallet_menu.c
 * @author  Cypherock X1 Team
 * @brief   Populate and handle main menu options
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
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

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "create_wallet_menu.h"

#include "constant_texts.h"
#include "ui_menu.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef enum {
  GENERATE_NEW_WALLET,
  RESTORE_FROM_SEED,
} create_wallet_options_e;

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static const evt_config_t create_wallet_evt_config = {
    .abort_disabled = false,
    .evt_selection.byte = EVT_CONFIG_UI_EVT,
    .timeout = MAX_INACTIVITY_TIMEOUT};

const flow_step_t create_wallet_step = {
    .step_init_cb = create_wallet_menu_initialize,
    .p0_cb = NULL,
    .ui_cb = create_wallet_menu_handler,
    .usb_cb = NULL,
    .nfc_cb = NULL,
    .evt_cfg_ptr = &create_wallet_evt_config,
    .flow_data_ptr = NULL};

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void create_wallet_menu_initialize(const engine_ctx_t *ctx,
                                   const void *data_ptr) {
  /* Create an array of pointers holding the string to display on the menu. */
  char *menu_option_ptr_array[NUMBER_OF_OPTIONS_NEW_WALLET];

  for (uint8_t menu_index = 0; menu_index < NUMBER_OF_OPTIONS_NEW_WALLET;
       menu_index++) {
    menu_option_ptr_array[menu_index] =
        (char *)ui_text_options_new_wallet[menu_index];
  }

  menu_init((const char **)menu_option_ptr_array,
            NUMBER_OF_OPTIONS_NEW_WALLET,
            ui_text_heading_new_wallet,
            true);

  return;
}

void create_wallet_menu_handler(const engine_ctx_t *ctx,
                                ui_event_t ui_event,
                                const void *data_ptr) {
  if (ui_event.event_type == UI_EVENT_REJECT) {
    engine_delete_current_flow_step(ctx);
  }
}

const flow_step_t *create_wallet_menu_get_step(void) {
  return &create_wallet_step;
}
