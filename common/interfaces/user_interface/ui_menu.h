/**
 * @file    ui_menu.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_MENU_H
#define UI_MENU_H

#include "lv_obj.h"
#include "ui_common.h"

#define MENU_MAX_NUMBER_OF_OPTIONS 12
#define MAX_LEN_OF_MENU_OPTIONS 25

// wrapper for reducing changes in codebase
#define menu_init(o, n, h, b) menu_init_with_choice(o, n, h, 0, b)

/**
 * @brief struct to store Menu data and meta data
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Menu_Data {
  char *option_list[MAX_LEN_OF_MENU_OPTIONS];
  int number_of_options;
  int current_index;
  char heading[36];
  bool back_button_allowed;
};

/**
 * @brief struct to store menu lvgl objects
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Menu_Object {
  lv_obj_t *heading;
  lv_obj_t *options;
  lv_obj_t *left_arrow;
  lv_obj_t *right_arrow;
  lv_obj_t *back_btn;
};

/**
 * @brief Initialize and create menu UI.
 * back_button_allowed true if back is to be shown
 * @details
 *
 * @param option_list Array of char array for options
 * @param number_of_options Number of options
 * @param heading Heading text
 * @param initial_selection The default selected choice
 * @param back_button_allowed Show a back button
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void menu_init_with_choice(const char *option_list[],
                           const int number_of_options,
                           const char heading[],
                           const int initial_selection,
                           const bool back_button_allowed);
#endif    // UI_MENU_H