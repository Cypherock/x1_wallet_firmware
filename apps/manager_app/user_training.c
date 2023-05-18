/**
 * @file    user_training.c
 * @author  Cypherock X1 Team
 * @brief   Joystick training flow for user on-boarding.
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

#include "events.h"
#include "manager/get_device_info.pb.h"
#include "manager/train_user.pb.h"
#include "manager_api.h"
#include "onboarding.h"
#include "status_api.h"
#include "ui_delay.h"
#include "ui_joystick_training.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/

#define JOYSTICK_TRAIN_STEPS 5

#undef SEND_ERROR
#define SEND_ERROR(a, b)                                                       \
  fill_training_error(a, b);                                                   \
  send_msg_to_host(a);

/**
 * @details The TRAINING_STEP macro takes message (m - string), user action (a -
 * joystick_actions_t), context (c - training_context) and flow status update
 * value (f). The macro **ignores timeout events** since currently, there is not
 * way to disable it. Also, in case of abort, it will clear usb event
 * (usb_clear_event) and silently return.
 */
#define TRAINING_STEP(m, a, c, f)                                              \
  do {                                                                         \
    joystick_train_init(m, a);                                                 \
    do {                                                                       \
      (c).events =                                                             \
          get_events((c).evt_config.evt_selection, (c).evt_config.timeout);    \
      if ((c).events.p0_event.abort_evt) {                                     \
        usb_clear_event();                                                     \
        return;                                                                \
      }                                                                        \
    } while (!(c).events.ui_event.event_occured);                              \
    core_status_set_flow_status(f);                                            \
  } while (0);

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct joystick_step {
  char *instruction;
  joystick_actions_t user_action;
  manager_train_user_status_t status;
} joystick_step_t;

typedef struct training_context {
  manager_train_user_response_t training_resp;
  manager_query_t *training_req;

  evt_config_t evt_config;
  evt_status_t events;
  joystick_step_t step[JOYSTICK_TRAIN_STEPS];

  // TODO: add fields for training data if needed
} training_context_t;

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
 * @brief Initializes a new training context.
 * @details The function populates the necessary information into the struct
 * members necessary for the training flow.
 *
 * @param query A reference to the query from host for flow initiation
 *
 * @return An initialized context with events, config, etc for the user training
 */
static training_context_t init_context(manager_query_t *query);

/**
 * @brief Checks validity of the entry-point training context data.
 * @details The function will ensure the following requirements:
 * <ol>
 * <li> stale p0 events do not exist at entry. </li>
 * <li> ensure data validity- received query if & is correctly decoded <li>
 * </ol>
 * In case of failure, the function will convey appropriate error to host. The
 * caller function need not handle the usb event as this function does that for
 * all the early exit cases.
 *
 * @return Whether the check is pass or fail.
 */
static bool validate_training_request(training_context_t *ctx);

/**
 * @brief Fill the error in the structure generated via protobuf.
 */
static void fill_training_error(manager_train_user_response_t *training,
                                uint32_t error_code);

/**
 * @brief Sends the encoded message via usb api to the host.
 * @details The functions takes the value of fields from the provided reference
 * to structure instance and encodes into a buffer before using the usb_send_msg
 * to send data in appropriate format to the host app.
 *
 * @param training Reference to a filled structure instance of
 * manager_train_user_response_t
 */
static void send_msg_to_host(manager_train_user_response_t *training);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static training_context_t init_context(manager_query_t *query) {
  training_context_t context = {
      .training_resp = MANAGER_TRAIN_USER_RESPONSE_INIT_ZERO,
      .training_req = query,
      .evt_config = {.timeout = 3000, .evt_selection = EVENT_CONFIG_USB},
      .events = {{0}, {0}, {0}, {0}},
      .step = {{.instruction = "Toggle up your joystick",
                .user_action = JS_ACTION_UP,
                .status = MANAGER_USER_TRAINING_UP},
               {.instruction = "Toggle right your joystick",
                .user_action = JS_ACTION_RIGHT,
                .status = MANAGER_USER_TRAINING_RIGHT},
               {.instruction = "Toggle down your joystick",
                .user_action = JS_ACTION_DOWN,
                .status = MANAGER_USER_TRAINING_DOWN},
               {.instruction = "Toggle left your joystick",
                .user_action = JS_ACTION_LEFT,
                .status = MANAGER_USER_TRAINING_LEFT},
               {.instruction = "Center click your joystick",
                .user_action = JS_ACTION_CENTER,
                .status = MANAGER_USER_TRAINING_CENTER}}};
  evt_config_t *e_config = &context.evt_config;
  context.events = get_events(e_config->evt_selection, e_config->timeout);

  return context;
}

static bool validate_training_request(training_context_t *ctx) {
  if (true == ctx->events.p0_event.flag) {
    // timeout cannot happen during flow entry, or host aborted. Both are
    // unexpected p0 event, do not proceed in flow
    if (!ctx->events.p0_event.abort_evt) {
      SEND_ERROR(&ctx->training_resp, 1);    // TODO: use correct error code
    } else {
      usb_clear_event();
    }
    return false;
  }

  // verify host query data is correct
  manager_train_user_request_t *req = &ctx->training_req->train_user;
  if (MANAGER_QUERY_TRAIN_USER_TAG != ctx->training_req->which_request ||
      MANAGER_TRAIN_USER_REQUEST_INITIATE_TAG != req->which_request) {
    SEND_ERROR(&ctx->training_resp, 1);    // TODO: use correct error code
    return false;
  }

  // TODO: check the request with what device knows
  return true;
}

static void fill_training_error(manager_train_user_response_t *training,
                                uint32_t error_code) {
  training->which_response = MANAGER_TRAIN_USER_RESPONSE_CORE_ERROR_TAG;
  training->core_error.which_error = ERROR_CORE_ERROR_DEVICE_SETUP_REQUIRED_TAG;
  training->core_error.device_setup_required = error_code;
}

static void send_msg_to_host(manager_train_user_response_t *training) {
  uint8_t payload[MANAGER_GET_DEVICE_INFO_RESULT_RESPONSE_SIZE] = {0};
  manager_result_t msg = MANAGER_RESULT_INIT_ZERO;
  size_t msg_size = 0;
  msg.which_response = MANAGER_RESULT_TRAIN_USER_TAG;

  memcpy(&msg.train_user, training, sizeof(manager_train_user_response_t));
  encode_manager_result(
      &msg, payload, MANAGER_GET_DEVICE_INFO_RESULT_RESPONSE_SIZE, &msg_size);
  usb_send_msg(payload, msg_size);
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void manager_user_training(manager_query_t *query) {
  training_context_t ctx = init_context(query);
  core_status_set_idle_state(CORE_DEVICE_IDLE_STATE_USB);
  core_status_set_device_waiting_on(CORE_DEVICE_WAITING_ON_IDLE);

  if (false == validate_training_request(&ctx)) {
    return;
  }

  ctx.evt_config.evt_selection = EVENT_CONFIG_UI;
  // start training from requested step by host
  int step_index =
      CY_MAX(ctx.training_req->train_user.initiate.jump_to_state - 1, 0);
  for (; step_index < JOYSTICK_TRAIN_STEPS; step_index++) {
    joystick_step_t *step = &ctx.step[step_index];
    TRAINING_STEP(step->instruction, step->user_action, ctx, step->status)
  }

  // TODO: perform card test and update ctx.training_resp
  core_status_set_flow_status(MANAGER_USER_TRAINING_CARD_TAP);
  ctx.training_resp.which_response = MANAGER_TRAIN_USER_RESPONSE_RESULT_TAG;
  ctx.training_resp.result.is_success = true;
  send_msg_to_host(&ctx.training_resp);
}