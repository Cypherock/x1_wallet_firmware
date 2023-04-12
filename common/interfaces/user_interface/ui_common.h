/**
 * @file    ui_common.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_COMMON_H
#define UI_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "assert_conf.h"
#include "board.h"
#include "controller_main.h"
#include "lvgl/lvgl.h"
#define OPTIONS_BTN_SIZE 100
#define SCROLLING_THRESHOLD 28
#define SCROLLING_LEN_THRESHOLD 70
#define MAX_HEADING_LEN 100

extern void (*ui_mark_event_over)();
extern void (*ui_mark_event_cancel)();
extern void (*ui_mark_list_choice)(uint16_t);

/// enum for device theme
typedef enum THEME { LIGHT, DARK } THEME;

/**
 * @brief struct to store device ui objects and properties
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct UI {
  lv_style_t btn_rel;
  lv_group_t *g;
  lv_indev_t *keyboard;
  enum THEME theme;
};

/**
 * @brief Rotate the device UI
 *
 */
void ui_rotate();

/**
 * @brief Create a heading
 * @details
 *
 * @param heading Lvgl object for heading
 * @param text heading text
 * @param width heading width
 * @param align heading alignment
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_heading(lv_obj_t *heading,
                const char text[],
                uint8_t width,
                lv_label_align_t align);

/**
 * @brief Create heading for inout text
 * @details
 *
 * @param heading Lvgl object for heading
 * @param text heading text
 * @param right_event_cb Right arrow event handler
 * @param left_event_cb Left arrow event handler
 * @param left_arrow Lvgl left arrow object
 * @param right_arrow Lvgl right arrow object
 * @param width heading width
 * @param align heading alignment
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_input_heading(lv_obj_t *heading,
                      const char text[],
                      lv_event_cb_t heading_event_cb,
                      uint8_t width,
                      lv_label_align_t align);

/**
 * @brief Create a UI element for paragraph
 * @details
 *
 * @param paragraph Lvgl object for paragraph
 * @param text Paragraph text
 * @param align Paragraph alignment
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_paragraph(lv_obj_t *paragraph,
                  const char text[],
                  lv_label_align_t align);

/**
 * @brief Create UI for list of options
 * @details
 *
 * @param options Lvgl object for options
 * @param event_cb Callback on any event
 * @param right_arrow Lvgl right arrow object
 * @param left_arrow Lvgl left arrow object
 * @param initial_text Options list text
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_options(lv_obj_t *options,
                lv_event_cb_t event_cb,
                lv_obj_t *right_arrow,
                lv_obj_t *left_arrow,
                const char initial_text[]);

/**
 * @brief Create UI for back button
 * @details
 *
 * @param back_btn Lvgl object for back button
 * @param event_cb Event callack for back button
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_back_btn(lv_obj_t *back_btn, lv_event_cb_t event_cb);

/**
 * @brief Create UI for cancel button
 * @details
 *
 * @param cancel_btn Lvgl object for cancel button
 * @param event_cb Event callback for cancel button
 * @param hidden Is the cancel button hidden
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_cancel_btn(lv_obj_t *cancel_btn, lv_event_cb_t event_cb, bool hidden);

/**
 * @brief Create UI for next button
 * @details
 *
 * @param next_btn Lvgl object for next button
 * @param event_cb Event callback fot next button
 * @param hidden Is the next button hidden
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_next_btn(lv_obj_t *next_btn, lv_event_cb_t event_cb, bool hidden);

/**
 * @brief Create UI for skip button
 * @details
 *
 * @param skip_btn Lvgl object for skip button
 * @param event_cb Event callback fot skip button
 * @param hidden Is the skip button hidden
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_skip_btn(lv_obj_t *skip_btn, lv_event_cb_t event_cb, bool hidden);

/**
 * @brief UI for backspace button
 * @details
 *
 * @param backspace Lvgl object for backspace button
 * @param event_cb Event callback for backspace button
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_backspace(lv_obj_t *backspace, lv_event_cb_t event_cb);

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
void ui_destructor();

/**
 * @brief Initialize UI, set default theme, set keypad, ui group and style.
 * @details
 *
 * @param kb_indev
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_init(lv_indev_t *kb_indev);

/**
 * @brief Set event over callback
 * @details
 *
 * @param event_over_cb event over callback function
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_set_event_over_cb(void (*event_over_cb)());

/**
 * @brief Set event cancel callback
 * @details
 *
 * @param event_cancel_cb event cancel callback function
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_set_event_cancel_cb(void (*event_cancel_cb)());

/**
 * @brief Set list choice callback
 * @details
 *
 * @param list_choice_cb List choice callback function taking uiint16_t as
 * parameter
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_set_list_choice_cb(void (*list_choice_cb)(uint16_t));

/**
 * @brief Set the theme of the device
 * @details
 *
 * @param theme THEME enum, LIGHT or DARK
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void set_theme(enum THEME theme);

/**
 * @brief Get the current theme of the device
 * @details
 *
 * @param
 *
 * @return enum THEME of the device
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
enum THEME get_theme();

/**
 * @brief Resets the theme and sets the default theme of the device
 * Sets dark theme to the device
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
void reset_theme();

/**
 * @brief get the UI group of the device
 * @details
 *
 * @param
 *
 * @return UI group lv_group_t*
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
lv_group_t *ui_get_group();

/**
 * @brief get the keyboard of the device
 * @details
 *
 * @param
 *
 * @return keyboard lv_indev_t*
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
lv_indev_t *ui_get_indev();

#endif