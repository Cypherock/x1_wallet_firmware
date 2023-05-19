/**
 * @file    device_authentication.c
 * @author  Cypherock X1 Team
 * @brief   This file describes the device authentication flow for the X1 vault
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
#include "constant_texts.h"
#include "device_authentication_api.h"
#include "events.h"
#include "manager_api.h"
#include "manager_app.h"
#include "status_api.h"
#include "ui_delay.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
// TODO: Eventually DEVICE_AUTH_RESPONSE_SIZE will be replaced by
// MANAGER_RESULT_SIZE when all option files for manager app are complete
#define DEVICE_AUTH_RESPONSE_SIZE (MANAGER_AUTH_DEVICE_RESPONSE_SIZE + 20)

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef enum {
  SIGN_SERIAL_NUM,
  SIGN_RANDOM_NUM,
  RESULT,
  FLOW_COMPLETE,
} device_auth_flow_states_e;

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
 * @brief This function returns the which_request tag of request of type
 * manager_auth_device_request_t
 *
 * @param request Reference to query
 * @return pb_size_t The which_request tag held in the query
 */
static pb_size_t get_request_type(const manager_auth_device_request_t *request);

/**
 * @brief Helper function to send serial signature to the host.
 *
 * @param state_ptr Pointer to state of type device_auth_flow_states_e depicting
 * the current state of the flow
 */
static void send_serial_signature(device_auth_flow_states_e *state_ptr);

/**
 * @brief This function is the USB handler callback for the device
 * authentication flow. It handles the further USB requests from the host to
 * complete the device authenticaiton flow.
 *
 * @param usb_evt The USB event which invoked this callback
 * @param data_ptr Pointer to state of type device_auth_flow_states_e depicting
 * the current state of the flow
 */
static void device_auth_usb_cb(usb_event_t usb_evt,
                               device_auth_flow_states_e *state_ptr);

/**
 * @brief This API sends the response for device authentication flow completion
 * to the host.
 *
 */
static void send_flow_complete(void);
/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static pb_size_t get_request_type(
    const manager_auth_device_request_t *request) {
  return request->which_request;
}

static void send_flow_complete(void) {
  manager_result_t result =
      get_manager_result_template(MANAGER_RESULT_AUTH_DEVICE_TAG);

  result.auth_device.which_response =
      MANAGER_AUTH_DEVICE_RESPONSE_FLOW_COMPLETE_TAG;
  result.auth_device.flow_complete.dummy_field = '\0';

  uint8_t response[DEVICE_AUTH_RESPONSE_SIZE] = {0};
  ASSERT(
      encode_and_send_manager_result(&result, &response[0], sizeof(response)));
  return;
}

static void send_serial_signature(device_auth_flow_states_e *state_ptr) {
  device_auth_flow_states_e state = *state_ptr;

  /* This will be only called if the correct manager request and
   * device authentication request is received. It is expected to be checked
   * within device_authentication API */

  if (SIGN_SERIAL_NUM == state) {
    /* Fetch and encode serial signature, send it to the host and wait for
     * further USB requests. */
    manager_result_t result =
        get_manager_result_template(MANAGER_RESULT_AUTH_DEVICE_TAG);
    result.auth_device = sign_serial_number();

    uint8_t response[DEVICE_AUTH_RESPONSE_SIZE] = {0};
    ASSERT(encode_and_send_manager_result(
        &result, &response[0], sizeof(response)));

    /* Set the next state to SIGN_RANDOM_NUM */
    *state_ptr = SIGN_RANDOM_NUM;
  }

  return;
}

static void device_auth_usb_cb(usb_event_t usb_evt,
                               device_auth_flow_states_e *state_ptr) {
  device_auth_flow_states_e state = *state_ptr;

  /* Decode recieved query using protobuf helpers */
  manager_query_t query = MANAGER_QUERY_INIT_ZERO;
  if (false == decode_manager_query(usb_evt.p_msg, usb_evt.msg_size, &query)) {
    // TODO: Handle proto decode error code
    usb_clear_event();
    return;
  }

  /* Decode recieved request_type */
  pb_size_t request_type = get_request_type(
      (const manager_auth_device_request_t *)&(query.auth_device));

  if (SIGN_RANDOM_NUM == state) {
    if (MANAGER_AUTH_DEVICE_REQUEST_CHALLENGE_TAG == request_type) {
      /* Fetch and encode random challenge signature, send it to the host and
       * wait for further USB requests. */
      manager_result_t result =
          get_manager_result_template(MANAGER_RESULT_AUTH_DEVICE_TAG);
      result.auth_device =
          sign_random_challenge(&(query.auth_device.challenge.challenge[0]));

      uint8_t response[DEVICE_AUTH_RESPONSE_SIZE] = {0};
      ASSERT(encode_and_send_manager_result(
          &result, &response[0], sizeof(response)));
      *state_ptr = RESULT;
    } else if (MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG == request_type) {
      /* If SIGN_RANDOM_NUM == state, but the request_type received is
       * MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG, it's an unexpected step in
       * the flow. The device will treat it as an attempt to force device
       * authentication status */
      device_auth_handle_response(false);
      send_flow_complete();
      *state_ptr = FLOW_COMPLETE;
    } else {
      // TODO: Handle error scenario in which an unexpected request_type was
      // detected
      usb_clear_event();
    }
  } else if (RESULT == state) {
    bool verified = query.auth_device.result.verified;
    device_auth_handle_response(verified);
    send_flow_complete();
    *state_ptr = FLOW_COMPLETE;
  }

  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void device_authentication_flow(const manager_query_t *query) {
  if (false == check_manager_request(query, MANAGER_QUERY_AUTH_DEVICE_TAG)) {
    // TODO: Handle bad data error
    // TODO: Early flow exit
  }

  pb_size_t request_type = get_request_type(&(query->auth_device));
  if (MANAGER_AUTH_DEVICE_REQUEST_INITIATE_TAG != request_type) {
    // TODO: Handle error scenario in which an unexpected request_type was
    // detected
    // TODO: Early flow exit
  }

  // TODO: Check if it's a forced device authentication, in which case we
  // will take users permission to perform authentication again

  /* Set flow status: Dummy for now! */
  core_status_set_flow_status(MANAGER_AUTH_DEVICE_STATUS_USER_CONFIRMED);

  delay_scr_init(ui_text_message_device_authenticating, 100);

  /* First state of the device authentication would be SIGN_SERIAL_NUMBER */
  device_auth_flow_states_e state = SIGN_SERIAL_NUM;
  send_serial_signature(&state);

  while (FLOW_COMPLETE != state) {
    evt_status_t event = get_events(EVENT_CONFIG_USB, MAX_INACTIVITY_TIMEOUT);
    if (true == event.p0_event.flag) {
      // TODO
    } else {
      device_auth_usb_cb(event.usb_event, &state);
    }
  }

  return;
}
