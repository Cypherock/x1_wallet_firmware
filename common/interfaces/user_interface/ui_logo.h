/**
 * @file    ui_logo.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_LOGO_H
#define UI_LOGO_H

#include "ui_common.h"

/**
 * @brief Create logo screen which clears after a delay
 * @details
 *
 * @param delay_in_ms Delay between display and clear logo in ms
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void logo_scr_init(uint16_t delay_in_ms);

#endif