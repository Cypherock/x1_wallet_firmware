/**
 * @file    systick_timer.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef SYSTICK_TIMER_H
#define SYSTICK_TIMER_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

#include "lvgl.h"
#include "p0_events.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  uint32_t timer;
  uint32_t timeout;
  bool timer_en;
} timeout_config_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief This callback is called upon a systick timer interrupt, which
 * increments time for the LVGL library and maintains inactivity timer for the
 * core system. In case the inactivity timer exceeds the timeout configured, it
 * registers the event in P0 event module.
 *
 */
void systick_interrupt_cb(void);

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

/**
 * @brief This API returns the current timer value of the timeout counter
 *
 * @return uint32_t
 */
uint32_t systick_get_timer_value(void);

#endif /* SYSTICK_TIMER_H */
