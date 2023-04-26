/**
 * @file    ui_list.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef LIST_H
#define LIST_H

#include "ui_common.h"
// TODO : Add constant

#define MAX_UI_LIST_WORDS MAX_NUMBER_OF_MNEMONIC_WORDS
#define MAX_UI_LIST_CHAR_LEN MAX_MNEMONIC_WORD_LENGTH

// TODO: Update count for higher coin list

/**
 * @brief struct to store list data
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct List_Data {
  char option_list[MAX_UI_LIST_WORDS][MAX_UI_LIST_CHAR_LEN];
  int number_of_options;
  int current_index;
  bool dynamic_heading;
  char *heading;
};

/**
 * @brief struct to store list UI objects
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct List_Object {
  lv_obj_t *heading;
  lv_obj_t *options;
  lv_obj_t *left_arrow;
  lv_obj_t *right_arrow;
  lv_obj_t *back_btn;
  lv_obj_t *next_btn;
};

/**
 * @brief Initialize and create list UI
 * @details
 *
 * @param option_list list options array of char arrays
 * @param number_of_options number of options in the list
 * @param heading heading of the UI
 * @param dynamic_heading if true, the heading will be appended with the current
 * index of the list
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note Do not use this if number of options to be displayed in list is 1.
 */
void list_init(const char option_list[MAX_UI_LIST_WORDS][MAX_UI_LIST_CHAR_LEN],
               int number_of_options,
               const char *heading,
               bool dynamic_heading);

#endif    // !LIST_H
