/**
 * @file    ui_menu.c
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
#include "ui_menu.h"

#include "ui_events_priv.h"

static struct Menu_Data *data = NULL;
static struct Menu_Object *obj = NULL;

/**
 * @brief Create menu UI
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
static void menu_create();

void menu_init_with_choice(const char *option_list[],
                           const int number_of_options,
                           const char heading[],
                           const int initial_selection,
                           const bool back_button_allowed) {
  ASSERT(NULL != option_list);
  ASSERT(NULL != heading);

  lv_obj_clean(lv_scr_act());

  data = NULL;
  data = malloc(sizeof(struct Menu_Data));
  obj = NULL;
  obj = malloc(sizeof(struct Menu_Object));
  ASSERT(data != NULL && obj != NULL);

  if (data != NULL) {
    data->number_of_options = number_of_options;
    data->current_index = initial_selection;
    data->back_button_allowed = back_button_allowed;
    snprintf(data->heading, sizeof(data->heading), "%s", heading);

    for (uint8_t i = 0; i < number_of_options; i++) {
      data->option_list[i] = (char *)option_list[i];
    }
  }

  menu_create();
  LOG_INFO("menu %s, %d", heading, number_of_options);
}

/**
 * @brief Clear screen
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
void menu_destructor() {
  if (data != NULL) {
    memzero(data, sizeof(struct Menu_Data));
    free(data);
    data = NULL;
  }
  if (obj != NULL) {
    free(obj);
    obj = NULL;
  }
}

/**
 * @brief Options event handler
 * @details
 *
 * @param options Options lvgl object
 * @param event event type
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void options_event_handler(lv_obj_t *options, const lv_event_t event) {
  switch (event) {
    case LV_EVENT_KEY:
      if (lv_btn_get_state(options) == LV_BTN_STATE_PR) {
        lv_btn_set_state(options, LV_BTN_STATE_REL);
      }
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_RIGHT:
          data->current_index++;
          data->current_index = data->current_index % data->number_of_options;
          lv_label_set_static_text(lv_obj_get_child(options, NULL),
                                   data->option_list[data->current_index]);
          break;
        case LV_KEY_LEFT:
          data->current_index--;
          data->current_index +=
              data->number_of_options;    // Adding this to ensure current_index
                                          // does not return negative number
          data->current_index = data->current_index % data->number_of_options;
          lv_label_set_static_text(lv_obj_get_child(options, NULL),
                                   data->option_list[data->current_index]);
          break;
        case LV_KEY_UP:
          // nothing to focus above this button
          break;
        case LV_KEY_DOWN:
          if (data->back_button_allowed)
            lv_group_focus_obj(obj->back_btn);
          break;
        default:
          break;
      }
      break;
    case LV_EVENT_CLICKED: {
      ui_set_list_event(data->current_index + 1);
      break;
    }
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(options, LV_BTN_STATE_REL);
      break;
    case LV_EVENT_DELETE: {
      /* Destruct object and data variables in case the object is being deleted
       * directly using lv_obj_clean() */
      menu_destructor();
      break;
    }
    default:
      break;
  }
}

/**
 * @brief Back button event handler.
 * @details
 *
 * @param back_btn Back button lvgl object.
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
static void back_btn_event_handler(lv_obj_t *back_btn, const lv_event_t event) {
  switch (event) {
    case LV_EVENT_KEY:
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_UP:
          lv_group_focus_obj(obj->options);
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
      lv_btn_set_state(back_btn, LV_BTN_STATE_REL);
      break;
    case LV_EVENT_DELETE: {
      /* Destruct object and data variables in case the object is being deleted
       * directly using lv_obj_clean() */
      menu_destructor();
      break;
    }
    default:
      break;
  }
}

void menu_create() {
  ASSERT(data != NULL);
  ASSERT(obj != NULL);

  obj->heading = lv_label_create(lv_scr_act(), NULL);
  obj->options = lv_btn_create(lv_scr_act(), NULL);
  obj->left_arrow = lv_label_create(lv_scr_act(), NULL);
  obj->right_arrow = lv_label_create(lv_scr_act(), NULL);
  if (data->back_button_allowed)
    obj->back_btn = lv_btn_create(lv_scr_act(), NULL);

  ui_heading(
      obj->heading, data->heading, LV_HOR_RES - 20, LV_LABEL_ALIGN_CENTER);
  ui_options(obj->options,
             options_event_handler,
             obj->right_arrow,
             obj->left_arrow,
             data->option_list[data->current_index]);
  if (data->back_button_allowed)
    ui_back_btn(obj->back_btn, back_btn_event_handler);
  if (data->number_of_options <= 1) {
    lv_obj_set_hidden(obj->left_arrow, true);
    lv_obj_set_hidden(obj->right_arrow, true);
  }
}
