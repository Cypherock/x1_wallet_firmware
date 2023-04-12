/**
 * @file    ui_message.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_MESSAGE_H
#define UI_MESSAGE_H

#include "ui_common.h"

/**
 * @brief struct to store message data
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note Add constant
 */
struct Message_Data {
  char *message;
};

/**
 * @brief struct to store message lvgl object
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Message_Object {
  lv_obj_t *message;
  lv_obj_t *next_btn;
};

/**
 * @brief Initialize and create message UI screen
 * A type of a confirmation screen but without cancel button
 * @details
 *
 * @param message message text
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void message_scr_init(const char *message);
#endif    // !UI_MESSAGE_H
