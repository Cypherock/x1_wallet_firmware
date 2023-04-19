/**
 * @file    ui_skip_instruction.h
 * @author  Cypherock X1 Team
 * @brief   Header for Card tap/detect UI.
 *          Headers for card detect and skip screens UI
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_SKIP_INSTRUCTION_H
#define UI_SKIP_INSTRUCTION_H

#include "ui_common.h"

/**
 * @brief struct to store card detect screen data
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Card_Detect_Data {
  char *text;
};

/**
 * @brief struct to store card tap screen ui objects
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Card_Detect_Object {
  lv_obj_t *text;
  lv_obj_t *skip_btn;
};

/**
 * @brief Initialize and create card detect screen
 * @details
 *
 * @param text card detect text
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void skip_instruction_scr_init(const char *text);

/**
 * @brief Clear card detect screen
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
void skip_instruction_scr_destructor();

/**
 * @brief Set focus on skip button
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
void skip_instruction_scr_focus_skip();

#endif