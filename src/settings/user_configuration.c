/**
 * @file    user_configuration.c
 * @author  Cypherock X1 Team
 * @brief   Source file containing helper functions to perform store user
 *          dependant configuration on the X1 vault
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
#include <stdbool.h>

#include "constant_texts.h"
#include "flash_api.h"
#include "flash_struct.h"
#include "settings_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void toggle_log_export(void) {
  bool logging_enabled = is_logging_enabled();
  const char *msg = ui_text_enable_log_export;

  if (logging_enabled) {
    msg = ui_text_disable_log_export;
  }

  if (core_confirmation(msg, NULL)) {
    set_logging_config(logging_enabled ? LOGGING_DISABLED : LOGGING_ENABLED,
                       FLASH_SAVE_NOW);
  }

  return;
}

void toggle_passphrase(void) {
  bool passphrase_enabled = is_passphrase_enabled();
  const char *msg = ui_text_enable_passphrase_step;

  if (passphrase_enabled) {
    msg = ui_text_disable_passphrase_step;
  }

  if (core_scroll_page(NULL, msg, NULL)) {
    set_enable_passphrase(
        passphrase_enabled ? PASSPHRASE_DISABLED : PASSPHRASE_ENABLED,
        FLASH_SAVE_NOW);
  }

  return;
}

void rotate_display(void) {
  if (core_confirmation(ui_text_rotate_display_confirm, NULL)) {
    ui_rotate();
    set_display_rotation(get_display_rotation() == LEFT_HAND_VIEW
                             ? RIGHT_HAND_VIEW
                             : LEFT_HAND_VIEW,
                         FLASH_SAVE_NOW);
  }
  return;
}