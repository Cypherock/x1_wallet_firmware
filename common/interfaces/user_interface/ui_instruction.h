/**
 * @file    ui_instruction.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_INSTRUCTION_H
#define UI_INSTRUCTION_H

#include "ui_common.h"

/**
 * @brief Initialize and create instruction init
 * @details
 *
 * @param message Instruction message text
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 * This screen displays some text for a given amount of time. Only text is shown
 * in this screen.
 *
 * Instruction Screen vs Delay Screen
 * Instruction screen has the ability to change the text shown on screen
 * without clearing and redoing the whole operation. For changing text
 * instruction screen is much more efficient.
 *
 * You'll have to call lv_task_handler() manually and delete screen
 * when not in use unlike delay screen.
 *
 */
void instruction_scr_init(const char *message, const char *heading_text);

/**
 * @brief Updates the text on the instruction text, lv_task_handler needs to be
 * called to udpate the screen, if immediate is set to true it is called
 * internally
 *
 * @param new_message New instruction message text
 * @param immediate   bool parameter, set true to update the screen immediately
 */
void instruction_scr_change_text(const char *new_message, bool immediate);

/**
 * @brief Clear screen
 *
 */
void instruction_scr_destructor();

#endif