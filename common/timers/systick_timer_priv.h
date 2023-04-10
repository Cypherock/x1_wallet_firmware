/**
 * @file    systick_timer_priv.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef SYSTICK_TIMER_PRIV_H
#define SYSTICK_TIMER_PRIV_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief This API resets the inactivity timer.
 *
 */
void systick_reset_timer(void);

/**
 * @brief This API sets the timeout of the inactivity timer.
 *
 * @param inactivity_timeout The timeout in milliseconds (ms) to be configured.
 */
void systick_set_timeout(uint32_t inactivity_timeout);

/**
 * @brief This API enables or disables the timer increments or timeout
 * comparisons. These operations must be only enabled when the system is waiting
 * for any such events.
 *
 * @param enable true or false, depicting whether the timer operations needs to
 * be enabled or not.
 */
void systick_set_timeout_config(bool enable);

#endif /* SYSTICK_TIMER_PRIV_H */
