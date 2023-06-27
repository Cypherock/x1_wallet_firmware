/**
 * @file    ui_multi_instruction.c
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
#include "ui_multi_instruction.h"

#include "assert_conf.h"
#include "ui_events_priv.h"

static struct Multi_Instruction_Data *data;
static struct Multi_Instruction_Object *obj;
static lv_task_t *next_instruction_task = NULL;
static lv_style_t arrow_style_pr;
static lv_style_t arrow_style_rel;

/**
 * @brief Helper function Hide/un-hide arrows based on the current index or
 * completed cycle
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
static void change_arrows() {
  if (data->total_strings == 1)
    return;

  if (data->index_of_current_string <= 0) {
    if (data->one_cycle_completed == false)
      lv_obj_set_hidden(obj->left_arrow, true);
    else
      lv_obj_set_hidden(obj->left_arrow, false);
    lv_obj_set_hidden(obj->right_arrow, false);
  } else {
    lv_obj_set_hidden(obj->left_arrow, false);
    lv_obj_set_hidden(obj->right_arrow, false);
  }
}

/**
 * @brief Helper function to reset arrows style to released
 *
 */
void reset_arrows_styles(void) {
  if (data->index_of_current_string == 0)
    change_arrows();
  lv_label_set_style(obj->left_arrow, LV_LABEL_STYLE_MAIN, &arrow_style_rel);
  lv_label_set_style(obj->right_arrow, LV_LABEL_STYLE_MAIN, &arrow_style_rel);
}

/**
 * @brief Helper function to increment current index.
 * @details
 *
 * @param PRESSED_KEY Current event (LV_KEY_RIGHT or LV_KEY_LEFT)
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void change_current_index(const lv_key_t PRESSED_KEY) {
  if (data->total_strings == 1)
    return;

  switch (PRESSED_KEY) {
    case LV_KEY_RIGHT: {
      if (data->index_of_current_string < (data->total_strings - 1)) {
        data->index_of_current_string++;
        if (!data->one_cycle_completed)
          data->one_cycle_completed =
              data->index_of_current_string == data->total_strings - 1;
      } else {
        data->one_cycle_completed = true;
        data->index_of_current_string = 0;
      }
      break;
    }
    case LV_KEY_LEFT: {
      if (data->index_of_current_string > 0) {
        data->index_of_current_string--;
      } else {
        if (data->one_cycle_completed == true)
          data->index_of_current_string = data->total_strings - 1;
      }
      break;
    }
    default:
      break;
  }
}

static void update_text(uint8_t index) {
  lv_label_set_static_text(obj->text, data->instruction_content[index].text);
  if (data->instruction_content[index].img == NULL) {
    lv_obj_align(obj->text, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_img_set_src(obj->img, LV_SYMBOL_DUMMY "");
    lv_obj_set_pos(obj->img, 0, 0);
    lv_obj_realign(obj->text);
    lv_obj_realign(obj->left_arrow);
    lv_obj_realign(obj->right_arrow);
  } else {
    lv_img_set_src(obj->img, data->instruction_content[index].img);
    lv_img_set_auto_size(obj->img, true);
    lv_obj_set_pos(obj->img,
                   data->instruction_content[index].img_x_offset,
                   data->instruction_content[index].img_y_offset);
    lv_obj_align(
        obj->text, obj->img, data->instruction_content[index].text_align, 0, 2);
  }
}

/**
 * @brief Callback passed to the timeout task to update the text on the screen
 * after delay in ms passed while creating the screen.
 * @details
 *
 * @param task Unused pointer to task
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void change_text_cb(lv_task_t *task) {
  change_current_index(LV_KEY_RIGHT);
  update_text(data->index_of_current_string);
  change_arrows();
}

/**
 * @brief Screen desctructor called on click only when one cycle is completed.
 * Clears the screen, cancels the tasks, clears the structs containing the state
 * of the screen.
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
static void multi_instructor_destructor() {
  if (next_instruction_task != NULL) {
    lv_task_set_prio(next_instruction_task, LV_TASK_PRIO_OFF);
    lv_task_del(next_instruction_task);
    next_instruction_task = NULL;
  }

  if (data != NULL) {
    memzero(data, sizeof(struct Multi_Instruction_Data));
    free(data);
    data = NULL;
  }

  if (obj != NULL) {
    free(obj);
    obj = NULL;
  }
}

/**
 * @brief Event handler for left, right and click event on the screen.
 * @details
 *
 * @param instruction Passed pointer of the text lvgl object on screen.
 * @param event Current event that trigger a call to this event handler. Only
 * LV_KEY_RIGHT, LV_KEY_LEFT and LV_EVENT_CLICKED are handled.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void arrow_event_handler(lv_obj_t *instruction, const lv_event_t event) {
  switch (event) {
    case LV_EVENT_KEY: {
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_RIGHT: {
          lv_label_set_style(
              obj->right_arrow, LV_LABEL_STYLE_MAIN, &arrow_style_pr);
          change_current_index(LV_KEY_RIGHT);
          update_text(data->index_of_current_string);
          change_arrows();
          lv_task_reset(next_instruction_task);
          break;
        }
        case LV_KEY_LEFT: {
          lv_label_set_style(
              obj->left_arrow, LV_LABEL_STYLE_MAIN, &arrow_style_pr);
          change_current_index(LV_KEY_LEFT);
          update_text(data->index_of_current_string);
          change_arrows();
          lv_task_reset(next_instruction_task);
          break;
        }
        case LV_KEY_UP:
        case LV_KEY_DOWN:
        case LV_KEY_ENTER:
          reset_arrows_styles();
          break;

        default:
          break;
      }
      break;
    }

    case LV_EVENT_CLICKED: {
      reset_arrows_styles();
      if (data->destruct_on_click == true &&
          ((data->index_of_current_string == data->total_strings - 1) ||
           data->one_cycle_completed)) {
        ui_set_confirm_event();
      }
      break;
    }

    case LV_EVENT_RELEASED: {
      reset_arrows_styles();
      break;
    }

    case LV_EVENT_DELETE: {
      if (next_instruction_task != NULL) {
        lv_task_set_prio(next_instruction_task, LV_TASK_PRIO_OFF);
        lv_task_del(next_instruction_task);
        next_instruction_task = NULL;
      }
      multi_instructor_destructor();
      break;
    }
    default:
      break;
  }
}

/**
 * @brief Fills the values in the structs and global variables.
 * Assigns width, style and initial text to lvgl objects.
 * Hides the arrows for initial screen.
 *
 */
void multi_instruction_create() {
  // TODO: Add assertions
  obj->text = lv_label_create(lv_scr_act(), NULL);
  obj->left_arrow = lv_label_create(lv_scr_act(), NULL);
  obj->right_arrow = lv_label_create(lv_scr_act(), NULL);
  obj->img = lv_img_create(lv_scr_act(), NULL);

  lv_label_set_long_mode(obj->text, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(obj->text, LV_HOR_RES - 22);
  lv_label_set_align(obj->text, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_event_cb(obj->text, arrow_event_handler);

  // Style when left or right button is released
  lv_style_copy(&arrow_style_rel, &lv_style_plain);
  arrow_style_rel.body.opa = 0;

  // Style when left or right button is pressed (highlights them)
  lv_style_copy(&arrow_style_pr, &lv_style_plain);
  arrow_style_pr.body.main_color = LV_COLOR_BLACK;
  arrow_style_pr.body.grad_color = LV_COLOR_BLACK;
  arrow_style_pr.body.radius = 100;
  arrow_style_pr.text.color = LV_COLOR_WHITE;
  lv_label_set_body_draw(obj->right_arrow, true);
  lv_label_set_body_draw(obj->left_arrow, true);

  // Left Arrow
  lv_label_set_text(obj->left_arrow, LV_SYMBOL_LEFT);
  lv_obj_align(obj->left_arrow, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
  lv_obj_set_event_cb(obj->left_arrow, arrow_event_handler);

  // Right Arrow
  lv_label_set_text(obj->right_arrow, LV_SYMBOL_RIGHT);
  lv_obj_align(obj->right_arrow, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
  lv_obj_set_event_cb(obj->right_arrow, arrow_event_handler);

  lv_group_add_obj(ui_get_group(), obj->text);
  lv_group_add_obj(ui_get_group(), obj->left_arrow);
  lv_group_add_obj(ui_get_group(), obj->right_arrow);
  lv_group_add_obj(ui_get_group(), obj->img);

  lv_indev_set_group(ui_get_indev(), ui_get_group());
  lv_obj_set_hidden(obj->left_arrow, true);
  if (data->total_strings == 1) {
    lv_obj_set_hidden(obj->right_arrow, true);
  }
  update_text(data->index_of_current_string);
}

/**
 * @brief   Set configuration for multi-instruction screen.
 *          Populates Multi_Instruction_Data object and setup
 *          the next_instruction_task
 */
void multi_instruction_set_config(const uint8_t count,
                                  const uint16_t delay_in_ms,
                                  const bool destruct_on_click) {
  data->destruct_on_click = destruct_on_click;
  data->index_of_current_string = 0;
  data->total_strings = count;
  data->one_cycle_completed = false;

  if (next_instruction_task == NULL) {
    next_instruction_task =
        lv_task_create(change_text_cb, delay_in_ms, LV_TASK_PRIO_MID, NULL);
  } else {
    lv_task_set_prio(next_instruction_task, LV_TASK_PRIO_MID);
  }
}

void multi_instruction_init(const char **arr,
                            const uint8_t count,
                            const uint16_t delay_in_ms,
                            const bool destruct_on_click) {
  ASSERT(arr != NULL && count < MAX_NUM_OF_INSTRUCTIONS);

  lv_obj_clean(lv_scr_act());

  data = NULL;
  data = malloc(sizeof(struct Multi_Instruction_Data));
  ASSERT(data != NULL);
  obj = NULL;
  obj = malloc(sizeof(struct Multi_Instruction_Object));
  ASSERT(obj != NULL);

  memzero(data, sizeof(struct Multi_Instruction_Data));
  for (uint8_t i = 0; i < count; i++) {
    snprintf(data->instruction_content[i].text,
             sizeof(data->instruction_content[i].text),
             "%s",
             arr[i]);
  }

  multi_instruction_set_config(count, delay_in_ms, destruct_on_click);
  multi_instruction_create();
}

void multi_instruction_with_image_init(instruction_content_t content[],
                                       const uint8_t count,
                                       const uint16_t delay_in_ms,
                                       const bool destruct_on_click) {
  ASSERT(content != NULL && count < MAX_NUM_OF_INSTRUCTIONS);

  lv_obj_clean(lv_scr_act());

  data = NULL;
  data = malloc(sizeof(struct Multi_Instruction_Data));
  ASSERT(data != NULL);
  obj = NULL;
  obj = malloc(sizeof(struct Multi_Instruction_Object));
  ASSERT(obj != NULL);

  memzero(data, sizeof(struct Multi_Instruction_Data));
  for (uint8_t i = 0; i < count; i++) {
    memcpy(&(data->instruction_content[i]),
           &content[i],
           sizeof(instruction_content_t));
  }

  multi_instruction_set_config(count, delay_in_ms, destruct_on_click);
  multi_instruction_create();
}