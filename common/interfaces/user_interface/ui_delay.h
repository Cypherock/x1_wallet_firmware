/**
 * @file    ui_delay.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_DELAY_H
#define UI_DELAY_H

#include "ui_common.h"
#ifdef DEV_BUILD
#define DELAY_SHORT 500
#define DELAY_TIME 500
#define DELAY_LONG_STRING 500
#else
#define DELAY_SHORT 1000
#define DELAY_TIME 3000
#define DELAY_LONG_STRING 5000
#endif

/**
 * @brief Initialize and create a delay screen
 * @details
 *
 * @param message Message text
 * @param delay_in_ms Delay time in ms
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note This screen displays some text for a given amount of time. Only text is
 * shown in this screen.
 */
void delay_scr_init(const char message[], uint32_t delay_in_ms);

#endif