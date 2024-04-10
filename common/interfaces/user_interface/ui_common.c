/**
 * @file    ui_common.c
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
#include "ui_common.h"

static struct UI *ui;

const char *ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *ALPHA_NUMERIC = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const char *NUMBERS = "0123456789";
const char *HEX = "0123456789ABCDEF";
const char *PASSPHRASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0"
                         "123456789 !\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~";

/// Global callback method called when an event is marked over
void (*ui_mark_event_over)();

/// Global callback method called when an event is cancel
void (*ui_mark_event_cancel)();

/// Global callback method called to mark user list choice on screen
void (*ui_mark_list_choice)(uint16_t);

void ui_rotate() {
#if USE_SIMULATOR == 0
  SSD_1306_rotate_display();
  invert_key_pad();
#endif
}

void ui_heading(lv_obj_t *heading,
                const char text[],
                const uint8_t width,
                const lv_label_align_t align) {
  ASSERT(heading != NULL);
  ASSERT(text != NULL);
  ASSERT(width != 0);

  if (strnlen(text, MAX_HEADING_LEN) < SCROLLING_THRESHOLD) {
    lv_label_set_long_mode(heading, LV_LABEL_LONG_SROLL);
  } else {
    lv_label_set_long_mode(heading, LV_LABEL_LONG_SROLL_CIRC);
  }
  /**
   * When the text is too long, scrolling speed is increased.
   * lvgl time variable is only 16 bits
   * https://forum.lvgl.io/t/animation-lv-label-long-sroll-lv-label-long-sroll-circ-stop-for-large-strings/1802/2
   */
  if (strnlen(text, MAX_HEADING_LEN) > SCROLLING_LEN_THRESHOLD) {
    lv_label_set_anim_speed(heading, 20);
  } else {
    lv_label_set_anim_speed(heading, 15);
  }

  lv_obj_set_width(heading, width);
  lv_label_set_align(heading, align);
  lv_label_set_text(heading, text);
  lv_obj_align(heading, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
}

void ui_input_heading(lv_obj_t *heading,
                      const char text[],
                      const lv_event_cb_t event_cb,
                      const uint8_t width,
                      const lv_label_align_t align) {
  ASSERT(heading != NULL);
  ASSERT(text != NULL);

  lv_obj_set_size(heading, 102, LV_DPI / 5 - 5);
  lv_obj_align(heading, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_set_height(heading, 15);

  lv_obj_t *label = lv_label_create(heading, NULL);
  static lv_style_t style_rel;
  lv_style_copy(&style_rel, &lv_style_btn_rel);
  // style_rel.body.radius = 10;
  style_rel.body.main_color = LV_COLOR_WHITE;
  style_rel.body.grad_color = LV_COLOR_WHITE;
  style_rel.body.border.part = LV_BORDER_NONE;
  style_rel.body.border.width = 1;
  style_rel.text.color = LV_COLOR_BLACK;

  lv_obj_set_style(heading, &style_rel);

  lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
  lv_label_set_anim_speed(label, 14);
  lv_obj_set_width(label, 102 - 4);
  lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
  lv_label_set_text(label, text);
  lv_obj_set_event_cb(heading, event_cb);
  lv_group_add_obj(ui->g, heading);
}

void ui_paragraph(lv_obj_t *paragraph,
                  const char text[],
                  const lv_label_align_t align) {
  ASSERT(paragraph != NULL);
  ASSERT(text != NULL);

  lv_label_set_long_mode(paragraph, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(paragraph, LV_HOR_RES - 5);
  lv_label_set_align(paragraph, align);
  lv_label_set_text(paragraph, text);
  lv_obj_align(paragraph, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
}

void ui_options(lv_obj_t *options,
                const lv_event_cb_t event_cb,
                lv_obj_t *right_arrow,
                lv_obj_t *left_arrow,
                const char initial_text[]) {
  ASSERT(options != NULL);
  ASSERT(right_arrow != NULL);
  ASSERT(left_arrow != NULL);
  ASSERT(initial_text != NULL);

  lv_obj_set_size(options, OPTIONS_BTN_SIZE, LV_DPI / 5 - 5);
  lv_obj_align(options, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_event_cb(options, event_cb);

  lv_obj_t *label = lv_label_create(options, NULL);
  lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL);
  lv_label_set_anim_speed(label, 14);
  lv_obj_set_width(label, OPTIONS_BTN_SIZE - 4);
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_label_set_static_text(label, initial_text);

  static lv_style_t btn_style_rel;
  lv_style_copy(&btn_style_rel, &ui->btn_rel);

  if (get_theme() == LIGHT) {
    btn_style_rel.body.main_color = LV_COLOR_BLACK;
    btn_style_rel.body.grad_color = LV_COLOR_BLACK;
    btn_style_rel.body.border.color = LV_COLOR_WHITE;
    btn_style_rel.body.border.width = 2;
    btn_style_rel.text.color = LV_COLOR_WHITE;
    btn_style_rel.body.shadow.color = LV_COLOR_BLACK;
    btn_style_rel.body.shadow.type = LV_SHADOW_FULL;
    btn_style_rel.body.shadow.width = 2;

    // Not declared outside if because dark theme already has this set by
    // default
    static lv_style_t btn_style_pr;
    lv_style_copy(&btn_style_pr, &ui->btn_rel);
    btn_style_pr.body.main_color = LV_COLOR_WHITE;
    btn_style_pr.body.grad_color = LV_COLOR_WHITE;
    btn_style_pr.text.color = LV_COLOR_BLACK;
    btn_style_pr.body.shadow.color = LV_COLOR_BLACK;
    btn_style_pr.body.shadow.type = LV_SHADOW_FULL;
    btn_style_pr.body.shadow.width = 2;
    lv_btn_set_style(options, LV_BTN_STYLE_PR, &btn_style_pr);
  }

  lv_btn_set_style(options, LV_BTN_STYLE_REL, &btn_style_rel);

  // Left Arrow
  lv_label_set_text(left_arrow, LV_SYMBOL_LEFT);
  lv_obj_align(left_arrow, options, LV_ALIGN_OUT_LEFT_MID, -1, 0);

  // Right Arrow
  lv_label_set_text(right_arrow, LV_SYMBOL_RIGHT);
  lv_obj_align(right_arrow, options, LV_ALIGN_OUT_RIGHT_MID, 1, 0);

  lv_group_add_obj(ui->g, options);
}

void ui_back_btn(lv_obj_t *back_btn, const lv_event_cb_t event_cb) {
  ASSERT(back_btn != NULL);

  lv_obj_set_size(back_btn, 20, LV_DPI / 5 - 5);
  lv_obj_align(back_btn, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_event_cb(back_btn, event_cb);

  lv_obj_t *label = lv_label_create(back_btn, NULL);
  lv_label_set_text(label, MY_BACK_ARROW);

  lv_btn_set_style(back_btn, LV_BTN_STYLE_REL, &ui->btn_rel);

  lv_group_add_obj(ui->g, back_btn);
}

void ui_cancel_btn(lv_obj_t *cancel_btn,
                   const lv_event_cb_t event_cb,
                   const bool hidden) {
  ASSERT(cancel_btn != NULL);

  lv_obj_set_size(cancel_btn, 20, LV_DPI / 5 - 5);
  lv_obj_align(cancel_btn, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_event_cb(cancel_btn, event_cb);

  lv_obj_t *label = lv_label_create(cancel_btn, NULL);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);    // TODO : Change Symbol
  lv_obj_set_hidden(cancel_btn, hidden);

  lv_btn_set_style(cancel_btn, LV_BTN_STYLE_REL, &ui->btn_rel);

  lv_group_add_obj(ui->g, cancel_btn);
}

void ui_next_btn(lv_obj_t *next_btn,
                 const lv_event_cb_t event_cb,
                 const bool hidden) {
  ASSERT(next_btn != NULL);

  lv_obj_set_size(next_btn, 20, LV_DPI / 5 - 5);
  lv_obj_align(next_btn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -3, 0);
  lv_obj_set_event_cb(next_btn, event_cb);

  lv_obj_t *label = lv_label_create(next_btn, NULL);
  lv_label_set_text(label, LV_SYMBOL_OK);
  lv_obj_set_hidden(next_btn, hidden);

  lv_btn_set_style(next_btn, LV_BTN_STYLE_REL, &ui->btn_rel);

  lv_group_add_obj(ui->g, next_btn);
}

void ui_skip_btn(lv_obj_t *skip_btn,
                 const lv_event_cb_t event_cb,
                 const bool hidden) {
  ASSERT(skip_btn != NULL);

  lv_obj_set_size(skip_btn, 46, LV_DPI / 5 - 5);
  lv_obj_align(skip_btn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -3, 0);
  lv_obj_set_event_cb(skip_btn, event_cb);

  lv_obj_t *label = lv_label_create(skip_btn, NULL);
  lv_label_set_text(label, "Skip " LV_SYMBOL_RIGHT);
  lv_obj_set_hidden(skip_btn, hidden);
  lv_group_add_obj(ui->g, skip_btn);

  lv_btn_set_style(skip_btn, LV_BTN_STYLE_REL, &ui->btn_rel);
}

void ui_backspace(lv_obj_t *backspace, const lv_event_cb_t event_cb) {
  ASSERT(backspace != NULL);

  lv_obj_set_size(backspace, LV_DPI / 5 - 1, LV_DPI / 5 - 11);
  lv_obj_align(backspace, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  lv_obj_set_event_cb(backspace, event_cb);

  lv_obj_t *label = lv_label_create(backspace, NULL);
  lv_label_set_text(label, MY_BACKSPACE);

  lv_btn_set_style(backspace, LV_BTN_STYLE_REL, &ui->btn_rel);

  lv_group_add_obj(ui->g, backspace);
}

/**
 * @brief Function to change style of object when it is focused
 *
 * @param group lvgl object group
 * @param style new style
 */
static void default_style_mod_cb(lv_group_t *group, lv_style_t *style) {
  ASSERT(group != NULL);
  ASSERT(style != NULL);

  style->body.border.part = LV_BORDER_FULL;
}

void ui_destructor() {
  ASSERT(ui != NULL);

  if (ui != NULL) {
    free(ui);
    ui = NULL;
  }
}

void ui_init(lv_indev_t *kb_indev) {
  ASSERT(kb_indev != NULL);

  ui = malloc(sizeof(struct UI));
  ASSERT(ui != NULL);

  /****Setting theme of system****/
  lv_theme_mono_init(210, NULL);
  lv_theme_t *th = lv_theme_get_mono();
  lv_theme_set_current(th);

  /****Set style of button when it is not pressed****/
  lv_style_copy(&ui->btn_rel, lv_theme_get_mono()->style.btn.rel);
  ui->btn_rel.body.border.part = LV_BORDER_NONE;

  /****Creating Group****/
  ui->g = lv_group_create();
  lv_indev_set_group(kb_indev, ui->g);
  lv_group_set_style_mod_cb(ui->g, default_style_mod_cb);

  ui->keyboard = kb_indev;
  ui->theme = DARK;
}

lv_group_t *ui_get_group() {
  ASSERT(ui != NULL);

  return ui->g;
}

lv_indev_t *ui_get_indev() {
  ASSERT(ui != NULL);

  return ui->keyboard;
}

inline void ui_set_event_over_cb(void (*event_over_cb)()) {
  ui_mark_event_over = event_over_cb;
}

inline void ui_set_event_cancel_cb(void (*event_cancel_cb)()) {
  ui_mark_event_cancel = event_cancel_cb;
}

inline void ui_set_list_choice_cb(void (*list_choice_cb)(uint16_t)) {
  ui_mark_list_choice = list_choice_cb;
}

void set_theme(enum THEME theme) {
  ASSERT(ui != NULL);

  ui->theme = theme;
}

enum THEME get_theme() {
  ASSERT(ui != NULL);

  return ui->theme;
}

void reset_theme() {
  ASSERT(ui != NULL);

  ui->theme = DARK;
}
