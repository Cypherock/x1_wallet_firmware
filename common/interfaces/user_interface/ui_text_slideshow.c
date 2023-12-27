/**
 * @file    ui_text_slideshow.c
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
#include "ui_text_slideshow.h"

#include "ui_events_priv.h"

static struct Text_Slideshow_Data *data;
static lv_task_t *slideshow_task = NULL;
static lv_obj_t *text;

/**
 * @brief Callback to update slide show text
 *
 * @param task Task variable passed by callback caller
 */
void change_text(lv_task_t *task) {
  ASSERT(data != NULL);

  data->index_of_current_string =
      (data->index_of_current_string + 1) % data->total_strings;
  lv_label_set_static_text(text, data->strings[data->index_of_current_string]);
  lv_obj_align(text, NULL, LV_ALIGN_CENTER, 0, 0);

  if (data->index_of_current_string == (data->total_strings - 1)) {
    data->one_cycle_completed = true;
  }
}

void ui_text_slideshow_change_text(const char *str,
                                   const uint8_t str_length,
                                   const uint8_t slide_index) {
  ASSERT(data != NULL);
  ASSERT(str != NULL);
  ASSERT(strnlen(str, MAX_NUM_OF_CHARS_IN_A_SLIDE) == str_length);

  // If checks, to prevent runtime errors
  if (data != NULL) {
    if (slide_index < data->total_strings) {
      // (str_lenght + 1) adjust length for NULL character
      snprintf(data->strings[slide_index], str_length + 1, "%s", str);
    }
  }
}

void ui_text_slideshow_destructor() {
  if (slideshow_task != NULL) {
    lv_task_set_prio(slideshow_task, LV_TASK_PRIO_OFF);
    lv_task_del(slideshow_task);
    slideshow_task = NULL;
  }

  if (data != NULL) {
    memzero(data, sizeof(struct Text_Slideshow_Data));
    free(data);
    data = NULL;
  }
}

/**
 * @brief Slideshow event handler
 * @details
 *
 * @param obj slideshow lvgl object
 * @param event type of event
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void event_handler(lv_obj_t *obj, const lv_event_t event) {
  switch (event) {
    case LV_EVENT_CLICKED: {
      if (data->one_cycle_completed && data->destruct_on_click) {
        ui_set_confirm_event();
      }
      break;
    }
    case LV_EVENT_DELETE: {
      ui_text_slideshow_destructor();
      break;
    }
    default:
      break;
  }
}

void ui_text_slideshow_init(const char *arr[MAX_NUM_OF_CHARS_IN_A_SLIDE],
                            const uint8_t count,
                            const uint16_t delay_in_ms,
                            const bool destruct_on_click) {
  ASSERT(arr != NULL);

  lv_obj_clean(lv_scr_act());

  if (count > MAX_NUM_OF_SLIDESHOWS)
    return;

  data = malloc(sizeof(struct Text_Slideshow_Data));

  for (uint8_t i = 0; i < count; i++) {
    snprintf(data->strings[i], sizeof(data->strings[i]), "%s", arr[i]);
  }

  data->index_of_current_string = 0;
  data->total_strings = count;
  data->one_cycle_completed = false;
  data->destruct_on_click = destruct_on_click;

  text = lv_label_create(lv_scr_act(), NULL);
  ui_paragraph(text, data->strings[0], LV_LABEL_ALIGN_CENTER);
  lv_obj_align(text, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_group_add_obj(ui_get_group(), text);
  lv_indev_set_group(ui_get_indev(), ui_get_group());
  lv_obj_set_event_cb(
      text,
      event_handler);    // The screen will destruct if the button is pressed

  if (slideshow_task == NULL) {
    slideshow_task =
        lv_task_create(change_text, delay_in_ms, LV_TASK_PRIO_MID, NULL);
  } else {
    lv_task_set_prio(slideshow_task, LV_TASK_PRIO_MID);
  }
}
