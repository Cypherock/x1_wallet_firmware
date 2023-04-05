/**
 * @file    ui_text_slideshow.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_TEXT_SLIDESHOW_H
#define UI_TEXT_SLIDESHOW_H

#include "ui_common.h"

#define MAX_NUM_OF_SLIDESHOWS 6
#define MAX_NUM_OF_CHARS_IN_A_SLIDE 80

/**
 * @brief struct to store Text slideshow data
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Text_Slideshow_Data {
  char strings[MAX_NUM_OF_SLIDESHOWS][MAX_NUM_OF_CHARS_IN_A_SLIDE];
  bool one_cycle_completed;    // if all the text has been shown once then this
                               // variable will be true else false
  bool destruct_on_click;      // if destruct_on_click == true the screen will
                               // destroy on a button click
  uint8_t total_strings;
  uint8_t index_of_current_string;
};

/**
 * @brief Update slideshow UI text
 * @details
 *
 * @param str new text
 * @param str_length new text length
 * @param slide_index slide index to be updated
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_text_slideshow_change_text(const char *str,
                                   uint8_t str_length,
                                   uint8_t slide_index);

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
void ui_text_slideshow_destructor();

/**
 * @brief Initialize and create slide show screen.
 * if destruct_on_click == true the screen will destroy on a button click
 * else call destructor manually
 * @details
 *
 * @param arr array of text for each slide
 * @param count number of slides
 * @param delay_in_ms delay between each slide
 * @param destruct_on_click clear slideshow screen on click
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void ui_text_slideshow_init(const char *arr[MAX_NUM_OF_CHARS_IN_A_SLIDE],
                            uint8_t count,
                            uint16_t delay_in_ms,
                            bool destruct_on_click);

#endif    // UI_TEXT_SLIDESHOW_H