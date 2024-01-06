/**
 * @file    flow_engine.c
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
#include "flow_engine.h"

#include "array_list.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define ENGINE_RUN_INITIALIZE_CB(cb, ctx, data_ptr)                            \
  {                                                                            \
    if (cb) {                                                                  \
      cb(ctx, data_ptr);                                                       \
    }                                                                          \
  }

#define ENGINE_RUN_EVENT_CB(cb, ctx, data_ptr, event)                          \
  {                                                                            \
    if (cb) {                                                                  \
      cb(ctx, event, data_ptr);                                                \
    }                                                                          \
  }

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief This function validates the pointers inside the struct engine_ctx_t
 * and returns true if the input is correct
 *
 * @param ctx Pointer to data of type engine_ctx_t
 * @return true If the input is valid
 * @return false If the input is not valid
 */
static bool engine_check_ctx(engine_ctx_t *ctx);

/**
 * @brief This function returns the a reference an element of type flow_step_t*,
 * from the selected queue which represents the current step of a flow.
 * This API can be used to get the current step of a flow in progress.
 *
 * @param ctx Pointer to data of type engine_ctx_t which holds the correct data
 * for the buffer
 * @param flow_step_dptr A double pointer which will be filled with the element
 * of type flow_step_t*, which can be dereferenced by the caller.
 * @return true If the element was returned successfully
 * @return false If the element was not returned: It could be due to incorrect
 * parameters, or because the buffer is EMPTY.
 */
static bool engine_get_current_flow_step(engine_ctx_t *ctx,
                                         flow_step_t **flow_step_dptr);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool engine_check_ctx(engine_ctx_t *ctx) {
  if ((NULL == ctx) || (NULL == ctx->array)) {
    return false;
  }

  return true;
}

static bool engine_get_current_flow_step(engine_ctx_t *ctx,
                                         flow_step_t **flow_step_dptr) {
  bool result = false;

  if (false == engine_check_ctx(ctx)) {
    return result;
  }

  result = array_list_get_element(ctx, flow_step_dptr);
  return result;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
bool engine_reset_flow(engine_ctx_t *ctx) {
  bool result = false;

  if (false == engine_check_ctx(ctx)) {
    return result;
  }

  /* Clear the buffer and index */
  size_t bytes_to_clear = ctx->max_capacity * (ctx->size_of_element);
  memzero(ctx->array, bytes_to_clear);

  ctx->current_index = 0;
  ctx->num_of_elements = 0;

  result = true;
  return result;
}

bool engine_add_next_flow_step(engine_ctx_t *ctx,
                               const flow_step_t *flow_step_ptr) {
  bool result = false;

  /* Ensure that flow_step_ptr is not NULL */
  if ((false == engine_check_ctx(ctx)) || (NULL == flow_step_ptr)) {
    return result;
  }

  /* Ensure that evt_cfg_ptr is not NULL */
  if (NULL == flow_step_ptr->evt_cfg_ptr) {
    return result;
  }

  /* We want to store the pointer to the flow step inputted from the argument
   * and NOT the actual content, therefore we need to pass the double pointer to
   * the API */
  const flow_step_t **flow_step_dptr = &flow_step_ptr;
  result = array_list_insert(ctx, flow_step_dptr);
  return result;
}

bool engine_goto_next_flow_step(engine_ctx_t *ctx) {
  bool result = false;

  if (false == engine_check_ctx(ctx)) {
    return result;
  }

  result = array_list_iterate_next(ctx);
  return result;
}

bool engine_goto_prev_flow_step(engine_ctx_t *ctx) {
  bool result = false;

  if (false == engine_check_ctx(ctx)) {
    return result;
  }

  result = array_list_iterate_back(ctx);
  return result;
}

bool engine_delete_current_flow_step(engine_ctx_t *ctx) {
  bool result = false;

  if (false == engine_check_ctx(ctx)) {
    return result;
  }

  result = array_list_delete_entry(ctx);
  return result;
}

void engine_run(engine_ctx_t *ctx) {
  if (false == engine_check_ctx(ctx)) {
    return;
  }

  while (1) {
    flow_step_t *current_flow = NULL;

    /* Break if the stack or queue reached an empty state */
    if ((false == engine_get_current_flow_step(ctx, &current_flow)) ||
        (NULL == current_flow)) {
      break;
    }

    /* It is ensured that evt_cfg_ptr is NOT NULL in function
     * engine_add_next_flow_step() */
    const evt_config_t *evt_config_ptr = current_flow->evt_cfg_ptr;

    const void *flow_data_ptr = current_flow->flow_data_ptr;

    /* If code flow reaches this point, it means that the UX flow is still not
     * complete. */
    ENGINE_RUN_INITIALIZE_CB(current_flow->step_init_cb, ctx, flow_data_ptr);

    evt_config_t evt_config = *evt_config_ptr;
    evt_status_t evt_status =
        get_events(evt_config.evt_selection, evt_config.timeout);

    /* Callbacks are called based on the event status returned by the get_events
     * API. It is expected that the evt_config and the callbacks match, meaning
     * that if any flow has requested to listen about x events, it must register
     * a callback for that event. */
    if (true == evt_status.p0_event.flag) {
      ENGINE_RUN_EVENT_CB(
          current_flow->p0_cb, ctx, flow_data_ptr, evt_status.p0_event);
    } else if (true == evt_status.ui_event.event_occured) {
      ENGINE_RUN_EVENT_CB(
          current_flow->ui_cb, ctx, flow_data_ptr, evt_status.ui_event);
    } else if (true == evt_status.usb_event.flag) {
      ENGINE_RUN_EVENT_CB(
          current_flow->usb_cb, ctx, flow_data_ptr, evt_status.usb_event);
    } else if (true == evt_status.nfc_event.event_occured) {
      ENGINE_RUN_EVENT_CB(
          current_flow->nfc_cb, ctx, flow_data_ptr, evt_status.nfc_event);
    } else {
      /* This case should never arise */
    }
  }

  return;
}