/**
 * @file    old_wallet_menu.c
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
#include "wallet_menu.h"

#include "constant_texts.h"
#include "ui_menu.h"
#include "wallet_selector.h"

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
  VIEW_WALLET_SEED,
  DELETE_WALLET,
} wallet_options_e;

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static const evt_config_t wallet_evt_config = {
    .abort_disabled = false,
    .evt_selection.byte = EVT_CONFIG_UI_EVT,
    .timeout = MAX_INACTIVITY_TIMEOUT};

const flow_step_t wallet_menu_step = {.step_init_cb = wallet_menu_initialize,
                                      .p0_cb = NULL,
                                      .ui_cb = wallet_menu_handler,
                                      .usb_cb = NULL,
                                      .nfc_cb = NULL,
                                      .evt_cfg_ptr = &wallet_evt_config,
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
void wallet_menu_initialize(const engine_ctx_t *ctx, const void *data_ptr) {
  /* Create an array of pointers holding the string to display on the menu. */
  char *menu_option_ptr_array[NUMBER_OF_OPTIONS_OLD_WALLET];

  for (uint8_t menu_index = 0; menu_index < NUMBER_OF_OPTIONS_OLD_WALLET;
       menu_index++) {
    menu_option_ptr_array[menu_index] =
        (char *)ui_text_options_old_wallet[menu_index];
  }

  char menu_heading[NAME_SIZE];
  snprintf(
      menu_heading, sizeof(menu_heading), "%s", selected_wallet_get_name());

  menu_init((const char **)menu_option_ptr_array,
            NUMBER_OF_OPTIONS_OLD_WALLET,
            menu_heading,
            true);

  return;
}

const flow_step_t *wallet_menu_lookup_choice(uint16_t selection) {
  switch ((wallet_options_e)selection) {
    case VIEW_WALLET_SEED: {
      return NULL;
      break;
    }
    case DELETE_WALLET: {
      return NULL;
      break;
    }
  }
}

void wallet_menu_handler(const engine_ctx_t *ctx,
                         ui_event_t ui_event,
                         const void *data_ptr) {
  switch (ui_event.event_type) {
    case UI_EVENT_LIST_CHOICE: {
      const flow_step_t *next_step =
          wallet_menu_lookup_choice(ui_event.list_selection - 1);

      engine_add_next_flow_step(ctx, next_step);
      engine_goto_next_flow_step(ctx);
      break;
    }
    case UI_EVENT_REJECT: {
      /* If the back button is pressed, we go back to the previous step:
       * probably it should be the main menu */
      engine_delete_current_flow_step(ctx);
      break;
    }
    case UI_EVENT_CONFIRM:
    case UI_EVENT_TEXT_INPUT:
    case UI_EVENT_SKIP_EVENT:
    default:
      /* Fall through from cases UI_EVENT_CONFIRM, UI_EVENT_TEXT_INPUT and
       * UI_EVENT_SKIP_EVENT is intentional, as these events are not expected
       * from the wallet menu */
      break;
  }

  return;
}

const flow_step_t *wallet_menu_get_step(uint8_t menu_wallet_index) {
  /* Set wallet context in selected_wallet, no need to check the return from
   * this function, as it is guaranteed that the wallet existed on
   * menu_wallet_index */
  (void)select_wallet_with_index(menu_wallet_index);

  // TODO: Fix invalid cases
  if (INVALID_WALLET == selected_wallet_get_state()) {
    return NULL;
  }

  if (true == selected_wallet_is_locked()) {
    return NULL;
  }

  if (true == selected_wallet_is_partial()) {
    return NULL;
  }

  if (UNVERIFIED_VALID_WALLET == selected_wallet_get_state()) {
    return NULL;
  }

  if (VALID_WALLET_WITHOUT_DEVICE_SHARE == selected_wallet_get_state()) {
    return NULL;
  }

  return &wallet_menu_step;
}
