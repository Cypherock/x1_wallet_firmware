/**
 * @file    restricted_app.c
 * @author  Cypherock X1 Team
 * @brief   Interface to provide restricted functionalities
 *
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
#include "restricted_app.h"

#include "constant_texts.h"
#include "core_error_priv.h"
#include "menu_priv.h"
#include "restricted_host_interface.h"
#include "restricted_priv.h"
#include "status_api.h"
#include "ui_screens.h"

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
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief This p0 event callback function handles clearing p0 events occured
 * while engine is waiting for other events.
 *
 * @details After onboarding initalization, we don't expect p0 events as no
 * operation or flow has been started yet.
 *
 * @param ctx The engine context* from which the flow is invoked
 * @param p0_evt The p0 event object which triggered the callback
 * @param data_ptr Currently unused pointer set by the engine
 */
static void ignore_p0_handler(engine_ctx_t *ctx,
                              p0_evt_t p0_evt,
                              const void *data_ptr);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static const flow_step_t restricted_flow = {
    .step_init_cb = restricted_app_initialize,
    .p0_cb = ignore_p0_handler,
    .ui_cb = NULL,
    .usb_cb = restricted_host_interface,
    .nfc_cb = NULL,
    .evt_cfg_ptr = &main_menu_evt_config,
    .flow_data_ptr = NULL};

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void ignore_p0_handler(engine_ctx_t *ctx,
                              p0_evt_t p0_evt,
                              const void *data_ptr) {
  ignore_p0_event();
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void restricted_app_initialize(engine_ctx_t *ctx, const void *data_ptr) {
  handle_core_errors();

  /* Set core_status to CORE_DEVICE_IDLE_STATE_IDLE as we are entering back to
   * the onboarding menu */
  core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_IDLE);

  /* Reset flow status back to zero */
  set_core_flow_status(0);
  set_app_flow_status(0);

  const char *pptr[2] = {ui_text_authentication_required,
                         ui_text_start_auth_from_CySync};
  multi_instruction_init(pptr, 2, DELAY_TIME, false);
  return;
}

const flow_step_t *restricted_app_get_step(void) {
  return &restricted_flow;
}
