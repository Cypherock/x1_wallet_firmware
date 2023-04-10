/**
 * @file    ui_confirmation.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_CONFIRMATION_H
#define UI_CONFIRMATION_H

#include "ui_common.h"

/**
 * @brief struct to store confirm screen data
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Confirm_Data {
  char *text;
};

/**
 * @brief struct to store confirm screen ui objects
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Confirm_Object {
  lv_obj_t *text;
  lv_obj_t *cancel_btn;
  lv_obj_t *next_btn;
};

/**
 * @brief Initialize and create confirm screen
 * @details
 *
 * @param text Confirm text
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void confirm_scr_init(const char *text);

/**
 * @brief Create confirm screen
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
void confirm_scr_create();

/**
 * @brief Set focus on cancel button
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
void confirm_scr_focus_cancel();

/**
 * @brief Set focus on next button
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
void confirm_scr_focus_next();
#endif