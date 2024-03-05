/**
 * @file    ui_input_text.c
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
#include "ui_input_text.h"

#include "ui_events_priv.h"
#include "utils.h"

static struct Input_Text_Data *data = NULL;
static struct Input_Text_Object *obj = NULL;

/**
 * @brief Create input text UI
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
static void input_text_create();

int get_entered_text_px_width() {
  lv_obj_t *label = lv_obj_get_child(obj->text_entered, NULL);
  lv_label_ext_t *ext = lv_obj_get_ext_attr(label);
  const lv_style_t *style = lv_obj_get_style(label);
  const lv_font_t *font = style->text.font;
  lv_coord_t max_w = lv_obj_get_width(label);
  lv_txt_flag_t flag = LV_TXT_FLAG_EXPAND;

  lv_point_t size;
  lv_txt_get_size(&size,
                  ext->text,
                  font,
                  style->text.letter_space,
                  style->text.line_space,
                  max_w,
                  flag);

  return size.x;
}

void ui_input_text(const char *input_list,
                   uint8_t initial_input_index,
                   char *input_text_ptr,
                   uint8_t input_text_buffer_size,
                   const char *initial_heading,
                   const uint8_t min_input_size,
                   const INPUT_DATA_TYPE data_type,
                   const uint8_t max_input_size) {
  ASSERT(input_list != NULL);
  ASSERT(initial_heading != NULL);
  ASSERT(max_input_size != 0);

  lv_obj_clean(lv_scr_act());

  data = malloc(sizeof(struct Input_Text_Data));
  obj = malloc(sizeof(struct Input_Text_Object));

  if (data != NULL) {
    data->input_list = input_list;

    /* TODO: Update after refactor */
    /* As per current scenario, wherever an input is required, applications use
     * input_text_init() which fills the input in global buffer
     * flow_level.screen_input.input_text. This will be fixed after refactor so
     * we shall remove below check once refactor is complete */
    if (input_text_ptr == NULL) {
      data->input_text_ptr = flow_level.screen_input.input_text;
      data->input_text_buffer_size = sizeof(flow_level.screen_input.input_text);
    } else {
      data->input_text_ptr = input_text_ptr;
      data->input_text_buffer_size = input_text_buffer_size;
    }

    data->input_list_size = strnlen(input_list, MAX_CHARACTER_INPUT_LIST);
    ASSERT(initial_input_index < data->input_list_size);

    data->initial_heading = (char *)initial_heading;
    data->min_input_size = min_input_size;
    data->current_index = initial_input_index;
    data->current_text[0] = input_list[data->current_index];
    data->current_display_index = 0;
    data->current_text[1] = '\0';
    data->max_input_size = max_input_size - 1;
    strncpy(data->entered_text, "", 1);
    data->data_type = data_type;
    strncpy(data->password_text, "", 1);
  }

  // Set light theme if the input data type is PASSPHRASE or PIN
  if (data_type == DATA_TYPE_PASSPHRASE || data_type == DATA_TYPE_PIN)
    set_theme(LIGHT);

  input_text_create();
  reset_theme();
}

void input_text_init(const char *input_list,
                     uint8_t initial_input_index,
                     const char *initial_heading,
                     const uint8_t min_input_size,
                     const INPUT_DATA_TYPE data_type,
                     const uint8_t max_input_size) {
  /* In order to support current calls to input_text_init, set the argument
   * `input_text_ptr` as NULL and `input_text_buffer_size` as 0*/
  ui_input_text(input_list,
                initial_input_index,
                NULL,
                0,
                initial_heading,
                min_input_size,
                data_type,
                max_input_size);
}

/**
 * @brief Clear screen
 *
 */
void input_text_destructor(void) {
  lv_obj_clean(lv_scr_act());
  if (data != NULL) {
    memzero(data, sizeof(struct Input_Text_Data));
    free(data);
    data = NULL;
  }
  if (obj != NULL) {
    free(obj);
    obj = NULL;
  }
}

/**
 * @brief Toggle OK button hidden status
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
static void hide_unhide_ok() {
  ASSERT(data != NULL);
  ASSERT(obj != NULL);

  if (strnlen(data->entered_text, MAX_ARRAY_SIZE) >= data->min_input_size) {
    lv_obj_set_hidden(obj->next_btn, false);
  } else {
    lv_obj_set_hidden(obj->next_btn, true);
  }
}

/**
 * @brief Add blank space in current input text
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
static void add_blank_space() {
  ASSERT(data != NULL);
  ASSERT(obj != NULL);

  if (data->current_text[0] == ' ') {
    char *space = "Blank Space";
    memcpy(data->current_text, space, 11);
    data->current_text[11] = 0;
    lv_obj_set_size(obj->character, LV_DPI / 5 + 52, LV_DPI / 5 - 3);
    lv_obj_realign(obj->character);
    lv_obj_realign(obj->left_arrow);
    lv_obj_realign(obj->right_arrow);
  }
}

/**
 * @brief Removes blank space if present
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
static void remove_blank_space() {
  ASSERT(data != NULL);
  ASSERT(obj != NULL);

  if (data->input_list[data->current_index] == ' ') {
    lv_obj_set_size(obj->character, LV_DPI / 5, LV_DPI / 5 - 3);
    lv_obj_realign(obj->character);
    lv_obj_realign(obj->left_arrow);
    lv_obj_realign(obj->right_arrow);
  }
}

/**
 * @brief Character event handler
 * @details
 *
 * @param character character lvgl object
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
static void character_event_handler(lv_obj_t *character,
                                    const lv_event_t event) {
  ASSERT(data != NULL);
  ASSERT(obj != NULL);
  ASSERT(character != NULL);

  switch (event) {
    case LV_EVENT_KEY:
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_RIGHT:
          remove_blank_space();
          data->current_index =
              (data->current_index + 1) % data->input_list_size;
          data->current_text[0] = data->input_list[data->current_index];
          data->current_text[1] = 0;
          add_blank_space();
          lv_label_set_static_text(lv_obj_get_child(character, NULL),
                                   data->current_text);
          break;
        case LV_KEY_LEFT:
          remove_blank_space();
          data->current_index +=
              data->input_list_size;    // Adding this to ensure current_index
                                        // does not return negative number
          data->current_index =
              (data->current_index - 1) % data->input_list_size;
          data->current_text[0] = data->input_list[data->current_index];
          data->current_text[1] = 0;
          add_blank_space();
          lv_label_set_static_text(lv_obj_get_child(character, NULL),
                                   data->current_text);
          break;
        case LV_KEY_UP:
          lv_group_focus_obj(obj->backspace);
          break;
        case LV_KEY_DOWN:
          if (strnlen(data->entered_text, MAX_ARRAY_SIZE) >=
              data->min_input_size)
            lv_group_focus_obj(obj->next_btn);
          else
            lv_group_focus_obj(obj->cancel_btn);
          break;

        default:
          break;
      }
      break;
    case LV_EVENT_CLICKED:
      if (data->input_list[data->current_index] == ' ') {
        data->current_text[0] = data->input_list[data->current_index];
        data->current_text[1] = 0;
      }

      if (strnlen(data->entered_text, MAX_ARRAY_SIZE) == 0)
        lv_label_set_text(lv_obj_get_child(obj->text_entered, NULL), "");

      // 10 is added to the current pixel width to change the alignment to
      // RIGHT incase the last added character overlaps 10 is considered an
      // average width of a pixel
      char first_char_current_text = data->current_text[0];
      if (get_entered_text_px_width() + 10 <= 98 &&
          strnlen(data->entered_text, MAX_ARRAY_SIZE) <= data->max_input_size) {
        if (data->data_type == DATA_TYPE_PIN &&
            strnlen(data->entered_text, MAX_ARRAY_SIZE) < MAX_PIN_SIZE) {
          strncat(data->entered_text, &first_char_current_text, 1);
          strncat(data->password_text, "*", 2);
          lv_label_set_text(lv_obj_get_child(obj->text_entered, NULL),
                            data->password_text);

          uint8_t temp = 0;
          // Generating a random number so that entering of the pin is random
          random_generate(&temp, 1);
          // Modulus with input_list_size is done as a fallback so that random
          // numbers are within the limits of the list of character choices

          data->current_index = ((temp % 10) + 26) % data->input_list_size;
          data->current_text[0] = data->input_list[data->current_index];
          lv_label_set_static_text(lv_obj_get_child(character, NULL),
                                   data->current_text);
        } else if (data->data_type != DATA_TYPE_PIN) {
          strncat(data->entered_text, &first_char_current_text, 1);
          add_blank_space();
          lv_label_set_text(lv_obj_get_child(obj->text_entered, NULL),
                            data->entered_text);
        }
        hide_unhide_ok();
      } else if (strnlen(data->entered_text, MAX_ARRAY_SIZE) <=
                 data->max_input_size) {
        if (data->data_type != DATA_TYPE_PIN) {
          strncat(data->entered_text, &first_char_current_text, 1);
          add_blank_space();
          lv_label_set_text(lv_obj_get_child(obj->text_entered, NULL),
                            data->entered_text);
          lv_label_set_align(lv_obj_get_child(obj->text_entered, NULL),
                             LV_LABEL_ALIGN_RIGHT);
          hide_unhide_ok();
        }
      }
      break;
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(character, LV_BTN_STATE_REL);
      break;
    default:
      break;
  }
}

/**
 * @brief Backspace event handler
 * @details
 *
 * @param backspace Backspace lvgl object
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
static void backspace_event_handler(lv_obj_t *backspace,
                                    const lv_event_t event) {
  ASSERT(data != NULL);
  ASSERT(obj != NULL);
  ASSERT(backspace != NULL);

  switch (event) {
    case LV_EVENT_KEY:
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_DOWN:
          lv_group_focus_obj(obj->character);
          break;
        case LV_KEY_LEFT:
          lv_group_focus_obj(obj->text_entered);
          break;
        default:
          break;
      }
      break;
    case LV_EVENT_CLICKED: {
      int len = strnlen(data->entered_text, MAX_ARRAY_SIZE);
      if (len == 0)
        break;
      else if (data->data_type == DATA_TYPE_PIN) {
        ASSERT(len < sizeof(data->password_text));
        data->password_text[len - 1] = '\0';
        data->entered_text[len - 1] = '\0';
        if (len <= 1) {
          lv_label_set_text(lv_obj_get_child(obj->text_entered, NULL),
                            data->initial_heading);
        } else
          lv_label_set_text(lv_obj_get_child(obj->text_entered, NULL),
                            data->password_text);
      } else {
        data->entered_text[len - 1] = '\0';
        if (len <= 1) {
          lv_label_set_text(lv_obj_get_child(obj->text_entered, NULL),
                            data->initial_heading);
        } else {
          lv_label_set_text(lv_obj_get_child(obj->text_entered, NULL),
                            data->entered_text);

          if (get_entered_text_px_width() <= 98)
            lv_label_set_align(lv_obj_get_child(obj->text_entered, NULL),
                               LV_LABEL_ALIGN_LEFT);
          else
            lv_label_set_align(lv_obj_get_child(obj->text_entered, NULL),
                               LV_LABEL_ALIGN_RIGHT);
        }
      }

      hide_unhide_ok();
    } break;
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(backspace, LV_BTN_STATE_REL);
      break;

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
  ASSERT(data != NULL);
  ASSERT(obj != NULL);
  ASSERT(cancel_btn != NULL);

  switch (event) {
    case LV_EVENT_KEY:
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_RIGHT:
          if (!lv_obj_get_hidden(obj->next_btn))
            lv_group_focus_obj(obj->next_btn);
          break;
        case LV_KEY_UP:
          lv_group_focus_obj(obj->character);
          break;

        default:
          break;
      }
      break;
    case LV_EVENT_CLICKED:
      ui_set_cancel_event();
      input_text_destructor();
      break;
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(cancel_btn, LV_BTN_STATE_REL);
      break;

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
  ASSERT(data != NULL);
  ASSERT(obj != NULL);
  ASSERT(next_btn != NULL);

  switch (event) {
    case LV_EVENT_KEY:
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_LEFT:
          lv_group_focus_obj(obj->cancel_btn);
          break;
        case LV_KEY_UP:
          lv_group_focus_obj(obj->character);
          break;

        default:
          break;
      }
      break;
    case LV_EVENT_CLICKED:
      /* As of now, all text is stored in the buffer data->entered_text,
       * therefore we must copy data from here into the buffer which the
       * application can access */
      ui_fill_text(data->entered_text,
                   data->input_text_ptr,
                   data->input_text_buffer_size);
      ui_set_text_input_event(data->input_text_ptr);
      input_text_destructor();
      break;
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(next_btn, LV_BTN_STATE_REL);
      break;

    default:
      break;
  }
}

/**
 * @brief Left button event handler.
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
static void input_heading_event_handler(lv_obj_t *input_heading,
                                        const lv_event_t event) {
  ASSERT(data != NULL);
  ASSERT(obj != NULL);
  ASSERT(input_heading != NULL);

  switch (event) {
    case LV_EVENT_KEY:
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_RIGHT:
          lv_group_focus_obj(obj->backspace);
          break;
        case LV_KEY_DOWN:
          lv_group_focus_obj(obj->character);
          break;

        default:
          break;
      }
      break;
    case LV_EVENT_FOCUSED: {
      lv_label_set_long_mode(lv_obj_get_child(obj->text_entered, NULL),
                             LV_LABEL_LONG_SROLL);
    } break;
    case LV_EVENT_DEFOCUSED: {
      lv_label_set_long_mode(lv_obj_get_child(obj->text_entered, NULL),
                             LV_LABEL_LONG_CROP);

      if (get_entered_text_px_width() <= 98)
        lv_label_set_align(lv_obj_get_child(obj->text_entered, NULL),
                           LV_LABEL_ALIGN_LEFT);
      else
        lv_label_set_align(lv_obj_get_child(obj->text_entered, NULL),
                           LV_LABEL_ALIGN_RIGHT);

    } break;
    default:
      break;
  }
}

void input_text_create() {
  ASSERT(data != NULL);
  ASSERT(obj != NULL);

  obj->text_entered = lv_btn_create(lv_scr_act(), NULL);
  obj->character = lv_btn_create(lv_scr_act(), NULL);
  obj->left_arrow = lv_label_create(lv_scr_act(), NULL);
  obj->right_arrow = lv_label_create(lv_scr_act(), NULL);
  obj->backspace = lv_btn_create(lv_scr_act(), NULL);
  obj->cancel_btn = lv_btn_create(lv_scr_act(), NULL);
  obj->next_btn = lv_btn_create(lv_scr_act(), NULL);

  ui_options(obj->character,
             character_event_handler,
             obj->right_arrow,
             obj->left_arrow,
             data->current_text);
  // ui_heading(obj->text_entered, data->initial_heading, LV_HOR_RES - 4,
  // LV_LABEL_ALIGN_LEFT)
  ui_input_heading(obj->text_entered,
                   data->initial_heading,
                   input_heading_event_handler,
                   LV_HOR_RES - 4,
                   LV_LABEL_ALIGN_LEFT);
  ui_backspace(obj->backspace, backspace_event_handler);
  ui_next_btn(obj->next_btn, next_btn_event_handler, true);
  ui_cancel_btn(obj->cancel_btn, cancel_btn_event_handler, false);

  lv_obj_set_size(obj->character, LV_DPI / 5, LV_DPI / 5 - 3);
  lv_obj_realign(obj->character);
  lv_obj_realign(obj->left_arrow);
  lv_obj_realign(obj->right_arrow);

  hide_unhide_ok();
}