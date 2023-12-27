/**
 * @file    p0_events.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef P0_EVENTS_H
#define P0_EVENTS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "sys_state.h"
#include "systick_timer.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/
// 0xFFFFFFFF ms = 4294967295 / (1000 * 3600 * 24) days ~ 49 days
#define INFINITE_WAIT_TIMEOUT 0xFFFFFFFF

#define MAX_INACTIVITY_TIMEOUT                                                 \
  (300 * 1000) /* As of now, there exists a maximum of 5 minutes of inactivity \
                  timeout */

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef struct {
  bool flag;           /* If true, then represents if a p0 event occurred */
  bool inactivity_evt; /* If true: then an inactivity event occurred
                              after the ctx was initialized, else no */
  bool abort_evt;      /* If true: then a abort event occurred after the ctx
                              was initialized, else no */
} p0_evt_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief This function initializes the context of P0 event getter. It sets the
 * configuration based on the application use case.
 *
 * @param inactivity_timeout Inactivity timeout to be setup for the application.
 * It can range from 1 to MAX_INACTIVITY_TIMEOUT, any invalid value will
 * represent MAX_INACTIVITY_TIMEOUT. Timeout value is in milliseconds (ms)
 */
void p0_ctx_init(uint32_t inactivity_timeout);

/**
 * @brief This function checks if any P0 event was encountered.
 *
 * @param p_p0_evt Pointer to variable of type p0_evt_t which will be filled up
 * by the API.
 * @return true: if any P0 event occurred.
 * @return false: if no P0 event occurred.
 * If p_p0_evt is NULL, the API returns false even if any event occurred.
 */
bool p0_get_evt(p0_evt_t *p_p0_evt);

/**
 * @brief This function resets the p0 events. Whenever a p0 event occurs, it
 * needs to be cleared manually by calling this function.
 *
 * NOTE: This function should be called with handling of p0 occurances.
 */
void p0_reset_evt();

/**
 * @brief This function reset timeout in systick module and disables timer
 * operations in interrupt cb NOTE: Any call to p0_ctx_init must followed with a
 * call to p0_ctx_destroy;
 */
void p0_ctx_destroy(void);

/**
 * @brief This function sets the inactivity event flag for the P0 event getter
 * ctx.
 *
 * @param status
 * true: indicating the event occured.
 * false: indicating the event did not occur.
 */
void p0_set_inactivity_evt(bool status);

/**
 * @brief This function sets the abort event flag for the P0 event getter
 * ctx.
 *
 * @param status
 * true: indicating the event occured.
 * false: indicating the event did not occur.
 */
void p0_set_abort_evt(bool status);

/**
 * @brief This function refreshes the value of timer, in case the application
 * wants to refresh the inactivity timer because of an intermediate event. For
 * example: any USB activity happening can reset the counter.
 *
 */
void p0_refresh_timeout(void);

#endif /* P0_EVENTS_H */
