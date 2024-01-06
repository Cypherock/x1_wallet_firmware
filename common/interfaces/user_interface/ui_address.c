/**
 * @file    ui_address.c
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
#include "ui_address.h"

#include "ui_events_priv.h"
#ifdef DEV_BUILD
#include "dev_utils.h"
#endif

static struct Address_Data *data = NULL;
static struct Address_Object *obj = NULL;

extern lv_task_t *address_timeout_task;

/**
 * @brief Create address screen
 * @details
 *
 * @param hide_buttons hide the next and cancel buttons
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void address_scr_create(bool hide_buttons);

void address_scr_init(const char text[],
                      const char address[],
                      const bool hide_buttons) {
  ASSERT(text != NULL);
  ASSERT(address != NULL);

  lv_obj_clean(lv_scr_act());

  data = malloc(sizeof(struct Address_Data));
  obj = malloc(sizeof(struct Address_Object));

  if (data != NULL) {
    snprintf(data->text, sizeof(data->text), "%s", text);
    snprintf(data->address, sizeof(data->address), "%s", address);
  }

#ifdef DEV_BUILD
  address_scr_create(false);
  ekp_enqueue(LV_KEY_UP, DEFAULT_DELAY);
  ekp_enqueue(LV_KEY_ENTER, DEFAULT_DELAY);
  return;
#endif
  address_scr_create(hide_buttons);
}

/**
 * @brief Clear address screen.
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
static void address_scr_destructor() {
  if (data != NULL) {
    memzero(data, sizeof(struct Address_Data));
    free(data);
    data = NULL;
  }
  if (obj != NULL) {
    free(obj);
    obj = NULL;
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
  if ((LV_EVENT_DELETE != event) && (lv_obj_get_hidden(cancel_btn))) {
    return;
  }

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
    case LV_EVENT_CLICKED: {
      ui_set_cancel_event();
      break;
    }
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(cancel_btn, LV_BTN_STATE_REL);
      break;
    case LV_EVENT_DELETE: {
      /* Destruct object and data variables in case the object is being deleted
       * directly using lv_obj_clean() */
      address_scr_destructor();
      break;
    }
    default:
      break;
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
  if ((LV_EVENT_DELETE != event) && (lv_obj_get_hidden(next_btn))) {
    return;
  }

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
      address_scr_destructor();
      break;
    }

    default:
      break;
  }
}

/**
 * @brief Callback function called after timeout for hidden buttons.
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
static void address_timeout_listener(lv_task_t *task) {
  lv_obj_set_hidden(obj->cancel_btn, false);
  lv_obj_set_hidden(obj->next_btn, false);
  lv_task_del(address_timeout_task);
  address_timeout_task = NULL;
}

/**
 * @brief Helper function to calculate time for which the buttons should be
 * hidden.
 * @details
 *
 * @param
 *
 * @return uint32_t Time in ms.
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static uint32_t compute_address_timeout() {
  ASSERT(data != NULL);
  ASSERT(obj != NULL);

  lv_label_ext_t *ext = lv_obj_get_ext_attr(obj->address);
  const lv_style_t *style = lv_obj_get_style(obj->address);
  const lv_font_t *font = style->text.font;
  lv_coord_t max_w = lv_obj_get_width(obj->address);
  lv_txt_flag_t flag = LV_TXT_FLAG_EXPAND;

  lv_point_t size;
  lv_txt_get_size(&size,
                  ext->text,
                  font,
                  style->text.letter_space,
                  style->text.line_space,
                  max_w,
                  flag);

  int32_t diff = size.x - max_w;

  if (diff < 0)
    return 0;

  uint32_t time = (int32_t)((int32_t)(diff * 1000) / ext->anim_speed);

  if (time > UINT16_MAX)
    time = UINT16_MAX;

  if (time == 0)
    time++;

  return time;
}

void address_scr_create(const bool hidden_buttons) {
  ASSERT(data != NULL);
  ASSERT(obj != NULL);

  obj->heading = lv_label_create(lv_scr_act(), NULL);
  obj->address = lv_label_create(lv_scr_act(), NULL);
  obj->cancel_btn = lv_btn_create(lv_scr_act(), NULL);
  obj->next_btn = lv_btn_create(lv_scr_act(), NULL);

  ui_paragraph(obj->heading, data->text, LV_LABEL_ALIGN_CENTER);
  ui_heading(
      obj->address, data->address, LV_HOR_RES - 4, LV_LABEL_ALIGN_CENTER);
  ui_cancel_btn(obj->cancel_btn, cancel_btn_event_handler, hidden_buttons);
  ui_next_btn(obj->next_btn, next_btn_event_handler, hidden_buttons);

  lv_obj_align_origo(
      obj->address, obj->heading, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);
  lv_group_focus_obj(obj->next_btn);

  if (hidden_buttons == true) {
    uint32_t time = compute_address_timeout();

    if (time != 0) {
      address_timeout_task = lv_task_create(
          address_timeout_listener, time, LV_TASK_PRIO_HIGH, NULL);
      lv_task_once(address_timeout_task);
    }
  }
}

void address_scr_focus_cancel() {
  ASSERT(obj != NULL);

  lv_group_focus_obj(obj->cancel_btn);
}

void address_scr_focus_next() {
  ASSERT(obj != NULL);

  lv_group_focus_obj(obj->next_btn);
}
