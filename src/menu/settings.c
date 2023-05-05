/**
 * @file    settings.c
 * @author  Cypherock X1 Team
 * @brief   Populate and handle vault settings
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
#include "settings.h"

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
  SYNC_CARD_CONFIRM,
  CARD_HEALTH_CHECK,
  ROTATE_SCREEN_CONFIRM,
  TOGGLE_LOGGING,
  TOGGLE_PASSPHRASE,
  FACTORY_RESET,
  VIEW_DEVICE_VERSION,
  READ_CARD_VERSION,
  REGULATORY_INFO,
#ifdef DEV_BUILD
  ADJUST_BUZZER,
#endif
  SETTINGS_OPTION_MAX,
} settings_options_e;

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static const evt_config_t settings_evt_config = {
    .abort_disabled = false,
    .evt_selection.byte = EVT_CONFIG_UI_EVT,
    .timeout = MAX_INACTIVITY_TIMEOUT};

const flow_step_t settings_step = {.step_init_cb = settings_initialize,
                                   .p0_cb = settings_p0_handler,
                                   .ui_cb = settings_handler,
                                   .usb_cb = NULL,
                                   .nfc_cb = NULL,
                                   .evt_cfg_ptr = &settings_evt_config,
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
void settings_initialize(const engine_ctx_t *ctx, const void *data_ptr) {
  /* Create an array of pointers holding the string to display on the settings.
   */
  char *menu_option_ptr_array[NUMBER_OF_OPTIONS_ADVANCED_OPTIONS];

  settings_options_e setting;
  uint8_t menu_idx = 0;
  uint8_t settings_str_idx = 0;
  for (setting = SYNC_CARD_CONFIRM; setting < SETTINGS_OPTION_MAX; setting++) {
    if (TOGGLE_LOGGING == setting) {
      menu_option_ptr_array[menu_idx] =
          (char *)(is_logging_enabled() ? ui_text_options_logging_export[0]
                                        : ui_text_options_logging_export[1]);

    } else if (TOGGLE_PASSPHRASE == setting) {
      menu_option_ptr_array[menu_idx] =
          (char *)(is_passphrase_enabled() ? ui_text_options_passphrase[0]
                                           : ui_text_options_passphrase[1]);

    } else {
      menu_option_ptr_array[menu_idx] =
          (char *)ui_text_options_advanced_settings[settings_str_idx];
      settings_str_idx += 1;
    }

    menu_idx += 1;
  }

  menu_init((const char **)menu_option_ptr_array,
            NUMBER_OF_OPTIONS_ADVANCED_OPTIONS,
            ui_text_heading_settings,
            true);
}

void settings_p0_handler(const engine_ctx_t *ctx,
                         p0_evt_t p0_event,
                         const void *data_ptr) {
  engine_delete_current_flow_step(ctx);
  return;
}

void settings_handler(const engine_ctx_t *ctx,
                      ui_event_t ui_event,
                      const void *data_ptr) {
  switch (ui_event.event_type) {
    case UI_EVENT_LIST_CHOICE: {
      // settings_get_step();
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
       * from the settings menu */
      break;
  }

  return;
}

const flow_step_t *settings_get_step(void) {
  return &settings_step;
}