/**
 * @file    ui_input_mnemonics.c
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
#include "ui_input_mnemonics.h"

#include "bip39.h"
#include "ui_events_priv.h"

static void ui_mnem_create();
static void refresh_screen_texts();
static void shrink();
static void expand();

/// enum to mark the current state of input mnemonic screen
enum { ENTERING_CHAR_ONE, ENTERING_CHAR_TWO, SHOWING_SUGGESTIONS, EXIT };

/**
 * @brief struct to store the mnemonic data for each word
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
struct Data {
  char heading[20];
  char text_entered[3];
  uint8_t state;
  uint16_t index;    // between 0-25 when state is ENTERING_CHAR_ONE/TWO. else
                     // between 0-2047
  uint16_t ind_low;
  uint16_t ind_high;
  uint32_t first_char_ind;     // stores the position bits for first characters
                               // possible in word list
  uint32_t second_char_ind;    // stores the position bits for second character
                               // possible in word list
};
#pragma pack(pop)

/**
 * @brief struct to store the lvgl objects for mnemonic input screen
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
struct LvObjects {
  lv_obj_t *text_entered;
  lv_obj_t *center_screen;
  lv_obj_t *left_arrow;
  lv_obj_t *right_arrow;
  lv_obj_t *backspace;
  lv_obj_t *cancel_btn;
};
#pragma pack(pop)

static struct Data *data = NULL;
static struct LvObjects *obj = NULL;

/**
 * @brief Get the first index for the mnemonic word matching the passed first 2
 * characters of the passed word
 *
 * @param word First 2 characters entered by user
 * @return int index or -1 for no matching word
 */
int get_first_index(const char word[3]) {
  ASSERT(word != NULL);

  for (int i = 0; i <= 2047; i++) {
    if ((word[0] + 32 == mnemonic_get_word(i)[0]) &&
        (word[1] + 32 == mnemonic_get_word(i)[1])) {
      return i;
    }
  }
  return -1;
}

/**
 * @brief Get the last index for the mnemonic word matching the passed first 2
 * characters of the passed word
 *
 * @param word First 2 characters entered by user
 * @return int index or -1 for no matching word
 */
int get_last_index(const char word[3]) {
  ASSERT(word != NULL);

  for (int i = 2047; i >= 0; --i) {
    if ((word[0] + 32 == mnemonic_get_word(i)[0]) &&
        (word[1] + 32 == mnemonic_get_word(i)[1])) {
      return i;
    }
  }
  return -1;
}

/**
 * @brief Initialize screen for input second character of the mnemonic word
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
static void second_char_init() {
  ASSERT(data != NULL);

  data->second_char_ind = 0;
  int16_t low = -1;
  int16_t high = 0;

  for (int i = 0; i <= 2047; i++) {
    if (data->text_entered[0] + 32 == mnemonic_get_word(i)[0]) {
      if (low < 0)
        low = mnemonic_get_word(i)[1] - 'a';
      data->second_char_ind |= 1 << (mnemonic_get_word(i)[1] - 'a');
      high = mnemonic_get_word(i)[1] - 'a';
    }
  }

  data->ind_low = low;
  data->ind_high = high;

  int16_t index = 0;
  while ((data->second_char_ind & (1 << index)) == 0)
    index++;

  data->index = index;
}

void ui_mnem_init(const char *heading) {
  ASSERT(heading != NULL);

  lv_obj_clean(lv_scr_act());

  data = malloc(sizeof(struct Data));
  obj = malloc(sizeof(struct LvObjects));

  if (data != NULL) {
    snprintf(data->heading, sizeof(data->heading), "%s", heading);
    data->text_entered[0] = '\0';
    data->state = ENTERING_CHAR_ONE;
    data->index = 0;
    data->ind_low = 0;
    data->ind_high = 25;
    data->first_char_ind = 58720255;
    data->second_char_ind = 0;
    set_theme(LIGHT);
    ui_mnem_create();
    reset_theme();
  }
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
static void mnem_destructor() {
  // TODO: assert(data->state == EXIT)
  if (data != NULL) {
    memzero(data, sizeof(struct Data));
    free(data);
    data = NULL;
  }
  if (obj != NULL) {
    free(obj);
    obj = NULL;
  }
}

/**
 * @brief Move to next character.
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
static void increment_data_index() {
  ASSERT(data != NULL);

  int16_t prev = data->index + 1;

  if (prev > data->ind_high)
    prev = data->ind_low;

  if (data->state == ENTERING_CHAR_ONE) {
    while ((data->first_char_ind & (1 << prev)) == 0)
      if (prev > data->ind_high)
        prev = data->ind_low;
      else
        prev++;

  } else if (data->state == ENTERING_CHAR_TWO) {
    while ((data->second_char_ind & (1 << prev)) == 0)
      if (prev > data->ind_high)
        prev = data->ind_low;
      else
        prev++;
  }
  data->index = prev;
}

/**
 * @brief Move to the previous character.
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
static void decrement_data_index() {
  ASSERT(data != NULL);

  int16_t prev = data->index - 1;

  if (prev < data->ind_low)
    prev = data->ind_high;

  if (data->state == ENTERING_CHAR_ONE) {
    while ((data->first_char_ind & (1 << prev)) == 0)
      if (prev < data->ind_low)
        prev = data->ind_high;
      else
        prev--;
  } else if (data->state == ENTERING_CHAR_TWO) {
    while ((data->second_char_ind & (1 << prev)) == 0)
      if (prev < data->ind_low)
        prev = data->ind_high;
      else
        prev--;
  }
  data->index = prev;
}

/**
 * @brief Input event handler
 * @details
 *
 * @param center lvgl object for center button
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
static void center_event_handler(lv_obj_t *center, const lv_event_t event) {
  switch (event) {
    case LV_EVENT_KEY:
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_RIGHT:
          increment_data_index();
          break;
        case LV_KEY_LEFT:
          decrement_data_index();
          break;
        case LV_KEY_UP:
          if (!lv_obj_get_hidden(obj->backspace))
            lv_group_focus_obj(obj->backspace);
          break;
        case LV_KEY_DOWN:
          lv_group_focus_obj(obj->cancel_btn);
          break;

        default:
          break;
      }
      break;
    case LV_EVENT_CLICKED:
      if (data->state == ENTERING_CHAR_ONE) {
        data->text_entered[0] = (data->index) + 'A';
        data->text_entered[1] = '\0';
        data->state = ENTERING_CHAR_TWO;
        lv_obj_set_hidden(obj->backspace, false);
        second_char_init();
      } else if (data->state == ENTERING_CHAR_TWO) {
        data->text_entered[1] = data->index + 'A';
        data->text_entered[2] = '\0';
        int new_index = get_first_index(data->text_entered);
        if (new_index == -1) {
          data->text_entered[1] = '\0';
          return;
        }
        data->state = SHOWING_SUGGESTIONS;
        data->index = (uint16_t)new_index;
        data->ind_low = (uint16_t)new_index;
        data->ind_high = (uint16_t)get_last_index(
            data->text_entered);    // won't be negative if new_index wasn't
        expand();
      } else if (data->state == SHOWING_SUGGESTIONS) {
        data->state = EXIT;
        ui_set_list_event(data->index);
        return;
      } else {
        // shouldn't come here
      }
      break;
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(center, LV_BTN_STATE_REL);
      break;
    case LV_EVENT_DELETE: {
      /* Destruct object and data variables in case the object is being deleted
       * directly using lv_obj_clean() */
      mnem_destructor();
      break;
    }
    default:
      break;
  }

  if (event != LV_EVENT_DELETE)
    refresh_screen_texts();
}

/**
 * @brief Backspace event handler
 * @details
 *
 * @param backspace lvgl backspace object
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
  switch (event) {
    case LV_EVENT_KEY:
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_DOWN:
          lv_group_focus_obj(obj->center_screen);
          break;

        default:
          break;
      }
      break;
    case LV_EVENT_CLICKED:
      if (data->state == ENTERING_CHAR_ONE) {
        // shouldn't come here. backspace button should be hidden
        // anyways shouldn't do anything even if it's not hidden
      } else if (data->state == ENTERING_CHAR_TWO) {
        data->state = ENTERING_CHAR_ONE;
        data->index = data->text_entered[0] - 'A';
        data->text_entered[0] = '\0';
        data->ind_low = 0;
        data->ind_high = 25;
        lv_obj_set_hidden(obj->backspace, true);
        lv_group_focus_obj(obj->center_screen);
      } else if (data->state == SHOWING_SUGGESTIONS) {
        data->state = ENTERING_CHAR_TWO;
        second_char_init();
        data->index = data->text_entered[1] - 'A';
        data->text_entered[1] = '\0';
        shrink();
      } else {
        // shouldn't come here
      }
      lv_label_set_text(obj->text_entered, data->text_entered);
      break;
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(backspace, LV_BTN_STATE_REL);
      break;
    case LV_EVENT_DELETE: {
      /* Destruct object and data variables in case the object is being deleted
       * directly using lv_obj_clean() */
      mnem_destructor();
      break;
    }
    default:
      break;
  }

  if (event != LV_EVENT_DELETE)
    refresh_screen_texts();
}

/**
 * @brief Cancel button event handler
 * @details
 *
 * @param cancel_btn lvgl cancel button
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
static void cancel_btn_event_handler(lv_obj_t *cancel_btn,
                                     const lv_event_t event) {
  switch (event) {
    case LV_EVENT_KEY:
      switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_UP:
          lv_group_focus_obj(obj->center_screen);
          break;

        default:
          break;
      }
      break;
    case LV_EVENT_CLICKED:
      ui_set_cancel_event();
      lv_obj_clean(lv_scr_act());
      break;
    case LV_EVENT_DEFOCUSED:
      lv_btn_set_state(cancel_btn, LV_BTN_STATE_REL);
      break;
    case LV_EVENT_DELETE: {
      /* Destruct object and data variables in case the object is being deleted
       * directly using lv_obj_clean() */
      mnem_destructor();
      break;
    }
    default:
      break;
  }
}

/**
 * @brief Reset screen to first character or first suggestion
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
static void refresh_screen_texts() {
  ASSERT(obj != NULL);
  ASSERT(data != NULL);

  if (data->state == ENTERING_CHAR_ONE) {
    char aux_str[2];
    aux_str[0] = data->index + 'A';
    aux_str[1] = '\0';
    lv_label_set_text(lv_obj_get_child(obj->center_screen, NULL), aux_str);
    lv_label_set_text(obj->text_entered, data->heading);
  } else if (data->state == ENTERING_CHAR_TWO) {
    char aux_str[2];
    aux_str[0] = data->index + 'A';
    aux_str[1] = '\0';
    lv_label_set_text(lv_obj_get_child(obj->center_screen, NULL), aux_str);
    lv_label_set_text(obj->text_entered, data->text_entered);
  } else if (data->state == SHOWING_SUGGESTIONS) {
    lv_label_set_text(lv_obj_get_child(obj->center_screen, NULL),
                      mnemonic_get_word(data->index));
    lv_label_set_text(obj->text_entered, data->text_entered);
  }
}

/**
 * @brief Create mnemonic input screen
 *
 */
static void ui_mnem_create() {
  ASSERT(obj != NULL);
  ASSERT(data != NULL);

  obj->text_entered = lv_label_create(lv_scr_act(), NULL);
  obj->center_screen = lv_btn_create(lv_scr_act(), NULL);
  obj->left_arrow = lv_label_create(lv_scr_act(), NULL);
  obj->right_arrow = lv_label_create(lv_scr_act(), NULL);
  obj->backspace = lv_btn_create(lv_scr_act(), NULL);
  obj->cancel_btn = lv_btn_create(lv_scr_act(), NULL);

  ui_heading(
      obj->text_entered, data->heading, LV_HOR_RES - 4, LV_LABEL_ALIGN_CENTER);
  ui_options(obj->center_screen,
             center_event_handler,
             obj->right_arrow,
             obj->left_arrow,
             "A");
  ui_backspace(obj->backspace, backspace_event_handler);
  ui_back_btn(obj->cancel_btn, cancel_btn_event_handler);
  lv_obj_set_hidden(obj->backspace, true);
  lv_label_set_text(
      obj->text_entered,
      data->heading);    // Vaibhav doesn't do this in ui_input_text but I don't
                         // know why it's not working with me
  shrink();
}

/**
 * @brief Reduce screen object size
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
static void shrink() {
  ASSERT(obj != NULL);

  lv_obj_set_size(obj->center_screen, LV_DPI / 5, LV_DPI / 5 - 3);
  lv_obj_realign(obj->center_screen);
  lv_obj_realign(obj->left_arrow);
  lv_obj_realign(obj->right_arrow);
}

/**
 * @brief Increase screen object size
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
static void expand() {
  ASSERT(obj != NULL);

  lv_obj_set_size(obj->center_screen, OPTIONS_BTN_SIZE, LV_DPI / 5 - 5);
  lv_obj_realign(obj->center_screen);
  lv_obj_realign(obj->left_arrow);
  lv_obj_realign(obj->right_arrow);
}