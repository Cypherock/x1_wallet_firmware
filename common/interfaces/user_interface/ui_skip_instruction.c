/**
 * @file    ui_skip_instruction.c
 * @author  Cypherock X1 Team
 * @brief   Card tap/detect UI.
 *          This file contains UI handlers for card detect and skip screens
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
#include "ui_skip_instruction.h"

#include "nfc.h"
#include "ui_events_priv.h"

static struct Card_Detect_Data *data = NULL;
static struct Card_Detect_Object *obj = NULL;
static void skip_instruction_scr_create();

void skip_instruction_scr_init(const char *text) {
  ASSERT(text != NULL);

  lv_obj_clean(lv_scr_act());

  data = malloc(sizeof(struct Card_Detect_Data));
  obj = malloc(sizeof(struct Card_Detect_Object));

  if (data != NULL) {
    data->text = (char *)text;
  }
  skip_instruction_scr_create();
}

void skip_instruction_scr_destructor() {
  if (data != NULL) {
    memzero(data, sizeof(struct Card_Detect_Data));
    free(data);
    data = NULL;
  }

  if (obj != NULL) {
    free(obj);
    obj = NULL;
  }
}

void skip_instruction_scr_focus_skip() {
  lv_group_focus_obj(obj->skip_btn);
}

/**
 * @brief Skip button event handler.
 * @details
 *
 * @param skip_btn Skip button lvgl object.
 * @param event Type of event.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void skip_btn_event_handler(lv_obj_t *skip_btn, const lv_event_t event) {
  switch (event) {
    case LV_EVENT_CLICKED:
      ui_set_cancel_event();
      break;
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(skip_btn, LV_BTN_STATE_REL);
      break;
    case LV_EVENT_DELETE:
      /* Destruct object and data variables in case the object is being deleted
       * directly using lv_obj_clean() */
      skip_instruction_scr_destructor();
      break;
    default:
      break;
  }
}

/**
 * @brief Create card detect screen
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void skip_instruction_scr_create() {
  ASSERT(obj != NULL);
  ASSERT(data != NULL);

  obj->text = lv_label_create(lv_scr_act(), NULL);
  obj->skip_btn = lv_btn_create(lv_scr_act(), NULL);

  ui_paragraph(obj->text, data->text, LV_LABEL_ALIGN_CENTER);
  ui_skip_btn(obj->skip_btn, skip_btn_event_handler, false);
  skip_instruction_scr_focus_skip();
}