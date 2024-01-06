/**
 * @file    ui_scroll_page.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef UI_SCROLL_PAGE
#define UI_SCROLL_PAGE

#include "ui_common.h"

#define MAXIMUM_CHARACTERS_IN_FOOTNOTE (15)

typedef struct {
  const char *p_ui_heading;
  const char *p_ui_body;
  uint16_t curr_page_num;
  int16_t total_page_num;
  char p_ui_footnote[MAXIMUM_CHARACTERS_IN_FOOTNOTE];
  bool bool_left_arrow_hidden;
  bool bool_right_arrow_hidden;
  bool bool_accept_cancel_visible;
  bool bool_accept_cancel_hidden;
} scrolling_page_data_t;

typedef struct {
  lv_obj_t *p_ui_page_lvgl;
  lv_obj_t *p_ui_header_lvgl;
  lv_style_t ui_header_style;
  lv_obj_t *p_ui_body_lvgl;
  lv_obj_t *p_ui_left_arrow_lvgl;
  lv_obj_t *p_ui_right_arrow_lvgl;
  lv_style_t ui_arrow_pressed_style;
  lv_style_t ui_arrow_released_style;
  lv_obj_t *p_ui_cancel_btn_lvgl;
  lv_obj_t *p_ui_accept_btn_lvgl;
  lv_obj_t *p_ui_footnote_lvgl;
} scrolling_page_lvgl_t;

typedef enum {
  MENU_SCROLL_HORIZONTAL = 0,
  MENU_SCROLL_UNDEFINED,
} e_scrollable_page_orientation_t;

/**
 * @brief This API renders a scrollable UI page with a optional heading, 2-3
 * rows of scrollable text, cancel and accept buttons, and a footnote depicting
 * pagination.
 * @details The UI is displayed in the following format:
 * |***************Floating heading***************|
 * |************** Scrollable text 1**************|
 * |<************* Scrollable text 2*************>|
 * |(cancel)***********Page no************(accept)|
 * In case the pointer to heading is not NULL, then, 2 rows of Scrollable text
 * is visible CENTER aligned In case the pointer to heading is NULL, then, 3
 * rows of Scrollable text is visible TOP_MID aligned
 *
 * @param p_page_ui_heading Pointer to the heading of the screen which is shown
 * as floating text. The heading is an optional field.
 * @param p_page_ui_body Pointer to the body of the screen which is scrollable.
 * It should end with Null (\0) character.
 * @param page_orientation Value of type e_scrollable_page_orientation_t which
 * selects whether scrolling is to be done horizontally or vertically. In
 * reality, scrolling happens vertically but visibility left/right arrow buttons
 * is manipulated
 * @param bool_cancel_accept_btn_visible If true, then accept and cancel button
 * is visible in every page If false, then accept and cancel button is only
 * visible at the last page
 */
void ui_scrollable_page(const char *p_page_ui_heading,
                        const char *p_page_ui_body,
                        e_scrollable_page_orientation_t page_orientation,
                        bool bool_cancel_accept_btn_visible);

#ifdef UI_HOR_SCROLL_PAGE_UNIT_TESTS
/**
 * @brief This function tests the functionality of the ui_scrollable_page() API.
 *
 */
void Ui_HorScrUnitTests(void);
#endif /* UI_HOR_SCROLL_PAGE_UNIT_TESTS */

#endif /* UI_SCROLL_PAGE */
