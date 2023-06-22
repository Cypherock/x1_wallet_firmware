/**
 * @file    p0_events.c
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2023 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * "Commons Clause" License Condition v1.0
 *
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "p0_events.h"

#include "status_api.h"
#include "systick_timer_priv.h"
/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static p0_evt_t p0_evt_status;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief This function sets the timeout in the systick module
 *
 * @param inactivity_timeout Value of timeout in milliseconds (ms)
 */
static void p0_set_inactivity_timeout(uint32_t inactivity_timeout);

/**
 * @brief This function sets the p0 event occured flag in the global status ctx
 *
 * @param flag
 * true: indicating a p0 event occured.
 * false: indicating a p0 event did not occur.
 */
static void p0_set_p0_evt_flag(bool flag);

/**
 * @brief This function sets the p0 event occured flag in the global status ctx
 *
 * @return true: indicating any p0 event occurred
 * @return false: indicated no p0 event occurred
 */
static bool p0_get_p0_evt_flag(void);

/**
 * @brief This function fills the pointer with the status held in the global
 * status context
 *
 * @param p_p0_evt Pointer to the variable of type p0_evt_t to be filled.
 */
static void p0_set_evt(p0_evt_t *p_p0_evt);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void p0_set_inactivity_timeout(uint32_t inactivity_timeout) {
  if ((inactivity_timeout == 0 ||
       inactivity_timeout >= MAX_INACTIVITY_TIMEOUT)) {
    inactivity_timeout = MAX_INACTIVITY_TIMEOUT;
  }

  systick_set_timeout(inactivity_timeout);
  p0_refresh_timeout();

  return;
}

static void p0_set_p0_evt_flag(bool flag) {
  p0_evt_status.flag = flag;
  return;
}

static bool p0_get_p0_evt_flag(void) {
  return p0_evt_status.flag;
}

static void p0_set_evt(p0_evt_t *p_p0_evt) {
  if (p_p0_evt != NULL) {
    memcpy(p_p0_evt, &p0_evt_status, sizeof(p0_evt_status));
  }
  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void p0_ctx_init(uint32_t inactivity_timeout) {
  /* Disable timer operations in interrupt cb */
  systick_set_timeout_config(false);

  /* Set inactivity timeout in systick module */
  p0_set_inactivity_timeout(inactivity_timeout);

  /* Enable timer operations in interrupt cb */
  systick_set_timeout_config(true);
  return;
}

bool p0_get_evt(p0_evt_t *p_p0_evt) {
  if (NULL == p_p0_evt) {
    return false;
  }

  /* Initialize event pointer with false flag */
  p_p0_evt->flag = false;
  p_p0_evt->inactivity_evt = false;
  p_p0_evt->abort_evt = false;

  bool p0_evt_occurred = p0_get_p0_evt_flag();
  if (true == p0_evt_occurred) {
    p0_set_evt(p_p0_evt);
  }

  return p0_evt_occurred;
}

void p0_reset_evt() {
  /* Reset event status in global context */
  p0_set_inactivity_evt(false);
  p0_set_abort_evt(false);
  p0_set_p0_evt_flag(false);
}

void p0_ctx_destroy(void) {
  /* Disable timer operations in interrupt cb */
  systick_set_timeout_config(false);

  /* Reset timeout in systick module */
  p0_set_inactivity_timeout(MAX_INACTIVITY_TIMEOUT);
  return;
}

void p0_set_inactivity_evt(bool status) {
  p0_evt_status.inactivity_evt = status;

  if (true == status) {
    p0_set_p0_evt_flag(true);
  }

  return;
}

void p0_set_abort_evt(bool status) {
  p0_evt_status.abort_evt = status;

  if (true == status) {
    p0_set_p0_evt_flag(true);
  }

  return;
}

void p0_refresh_timeout(void) {
  systick_reset_timer();
  return;
}
