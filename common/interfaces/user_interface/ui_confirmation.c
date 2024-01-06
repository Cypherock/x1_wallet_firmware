/**
 * @file    ui_confirmation.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
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
#include "ui_confirmation.h"

#include "ui_events_priv.h"

#ifdef DEV_BUILD
#include "dev_utils.h"
#endif

static struct Confirm_Data *data = NULL;
static struct Confirm_Object *obj = NULL;

void confirm_scr_init(const char *text) {
  ASSERT(text != NULL);

  lv_obj_clean(lv_scr_act());

  data = malloc(sizeof(struct Confirm_Data));
  obj = malloc(sizeof(struct Confirm_Object));

  if (data != NULL) {
    data->text = (char *)text;
  }

#ifdef DEV_BUILD
  ekp_enqueue(LV_KEY_UP, DEFAULT_DELAY);
  ekp_enqueue(LV_KEY_ENTER, DEFAULT_DELAY);
#endif
  confirm_scr_create();
}

/**
 * @brief Clear confirm screen
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
static void confirm_scr_destructor() {
  if (data != NULL) {
    memzero(data, sizeof(struct Confirm_Data));
    free(data);
    data = NULL;
  }
  if (obj != NULL) {
    free(obj);
    obj = NULL;
  }
}

/**
 * @brief Next button event handler.
 * @details
 *
 * @param next_btn Next button lvgl object.
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
static void next_btn_event_handler(lv_obj_t *next_btn, const lv_event_t event) {
  switch (event) {
    case LV_EVENT_KEY:
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_LEFT:
          lv_group_focus_obj(obj->cancel_btn);
          break;

        default:
          break;
      }
      break;
    case LV_EVENT_CLICKED: {
      ui_set_confirm_event();
      break;
    }
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(next_btn, LV_BTN_STATE_REL);
      break;
    case LV_EVENT_DELETE: {
      /* Destruct object and data variables in case the object is being deleted
       * directly using lv_obj_clean() */
      confirm_scr_destructor();
      break;
    }
    default:
      break;
  }
}

/**
 * @brief Cancel button event handler.
 * @details
 *
 * @param cancel_btn Cancel button lvgl object.
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
static void cancel_btn_event_handler(lv_obj_t *cancel_btn,
                                     const lv_event_t event) {
  switch (event) {
    case LV_EVENT_KEY:
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_RIGHT:
          lv_group_focus_obj(obj->next_btn);
          break;

        default:
          break;
      }
      break;
    case LV_EVENT_CLICKED:
      ui_set_cancel_event();
      break;
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(cancel_btn, LV_BTN_STATE_REL);
      break;
    case LV_EVENT_DELETE: {
      /* Destruct object and data variables in case the object is being deleted
       * directly using lv_obj_clean() */
      confirm_scr_destructor();
      break;
    }
    default:
      break;
  }
}

void confirm_scr_create() {
  ASSERT(obj != NULL);
  ASSERT(data != NULL);

  obj->text = lv_label_create(lv_scr_act(), NULL);
  obj->cancel_btn = lv_btn_create(lv_scr_act(), NULL);
  obj->next_btn = lv_btn_create(lv_scr_act(), NULL);

  ui_paragraph(obj->text, data->text, LV_LABEL_ALIGN_CENTER);
  ui_cancel_btn(obj->cancel_btn, cancel_btn_event_handler, false);
  ui_next_btn(obj->next_btn, next_btn_event_handler, false);

  confirm_scr_focus_next();
}

void confirm_scr_focus_cancel() {
  lv_group_focus_obj(obj->cancel_btn);
}

void confirm_scr_focus_next() {
  lv_group_focus_obj(obj->next_btn);
}