/**
 * @file    ui_address.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_ADDRESS_H
#define UI_ADDRESS_H

#include "ui_common.h"

/**
 * @brief struct to store address and heading
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Address_Data {
  char text[512];
  char address[512];
};

/**
 * @brief struct to store address screen UI components
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Address_Object {
  lv_obj_t *heading;
  lv_obj_t *address;
  lv_obj_t *cancel_btn;
  lv_obj_t *next_btn;
};

/**
 * @brief Initialize and create an address screen
 * @details
 *
 * @param text Heading
 * @param address Address
 * @param hide_buttons hide the next and cancel buttons
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void address_scr_init(const char text[],
                      const char address[],
                      bool hide_buttons);

/**
 * @brief Add focus on cancel button on screen.
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
void address_scr_focus_cancel();

/**
 * @brief Add focus on next button on screen.
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
void address_scr_focus_next();

#endif    // !UI_ADDRESS_H
