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
} device_auth_state_e;

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
 * @brief This API sends the response for device authentication flow completion
 * to the host.
 *
 */
static manager_auth_device_response_t send_flow_complete(void);

/**
 * @brief This function is the request handler for the SIGN_SERIAL_NUM state of
 * the device authentication flow
 *
 * @param query Reference to query of type manager_query_t as received from the
 * host
 * @param result Reference to the result of type manager_result_t, which will be
 * populated by the API. This API will only fill the auth_device field of the
 * manager_result_t.
 * @param result_valid If the result returned from the API is valid.
 * @return device_auth_state_e The next state of the flow
 */
static device_auth_state_e sign_serial_handler(const manager_query_t *query,
                                               manager_result_t *result,
                                               bool *result_valid);

/**
 * @brief This function is the request handler for the SIGN_RANDOM_NUM state of
 * the device authentication flow
 *
 * @param query Reference to query of type manager_query_t as received from the
 * host
 * @param result Reference to the result of type manager_result_t, which will be
 * populated by the API. This API will only fill the auth_device field of the
 * manager_result_t.
 * @param result_valid If the result returned from the API is valid.
 * @return device_auth_state_e The next state of the flow
 */
static device_auth_state_e sign_random_handler(const manager_query_t *query,
                                               manager_result_t *result,
                                               bool *result_valid);

/**
 * @brief This function is the request handler for the RESULT state of
 * the device authentication flow
 *
 * @param query Reference to query of type manager_query_t as received from the
 * host
 * @param result Reference to the result of type manager_result_t, which will be
 * populated by the API. This API will only fill the auth_device field of the
 * manager_result_t.
 * @param result_valid If the result returned from the API is valid.
 * @return device_auth_state_e The next state of the flow
 */
static device_auth_state_e result_handler(const manager_query_t *query,
                                          manager_result_t *result,
                                          bool *result_valid);
/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static pb_size_t get_request_type(
    const manager_auth_device_request_t *request) {
  return request->which_request;
}

static manager_auth_device_response_t send_flow_complete(void) {
  manager_auth_device_response_t response =
      MANAGER_AUTH_DEVICE_RESPONSE_INIT_ZERO;
  response.which_response = MANAGER_AUTH_DEVICE_RESPONSE_FLOW_COMPLETE_TAG;
  response.flow_complete.dummy_field = '\0';
  return response;
}

static device_auth_state_e sign_serial_handler(const manager_query_t *query,
                                               manager_result_t *result,
                                               bool *result_valid) {
  pb_size_t request_type = get_request_type(&query->auth_device);
  device_auth_state_e next_state = SIGN_SERIAL_NUM;

  switch (request_type) {
    case MANAGER_AUTH_DEVICE_REQUEST_INITIATE_TAG: {
      result->auth_device = sign_serial_number();
      *result_valid = true;
      next_state = SIGN_RANDOM_NUM;
      break;
    }
    case MANAGER_AUTH_DEVICE_REQUEST_CHALLENGE_TAG:
    case MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG:
    default: {
      usb_clear_event();
      break;
    }
  }

  return next_state;
}

static device_auth_state_e sign_random_handler(const manager_query_t *query,
                                               manager_result_t *result,
                                               bool *result_valid) {
  pb_size_t request_type = get_request_type(&query->auth_device);
  device_auth_state_e next_state = SIGN_RANDOM_NUM;

  switch (request_type) {
    case MANAGER_AUTH_DEVICE_REQUEST_INITIATE_TAG: {
      usb_clear_event();
      break;
    }
    case MANAGER_AUTH_DEVICE_REQUEST_CHALLENGE_TAG: {
      uint8_t *challenge =
          (uint8_t *)&(query->auth_device.challenge.challenge[0]);
      result->auth_device = sign_random_challenge(challenge);
      *result_valid = true;
      next_state = RESULT;
      break;
    }
    case MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG: {
      /* If SIGN_RANDOM_NUM == state, but the request_type received is
       * MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG, it's an unexpected step in
       * the flow. The device will treat it as an attempt to force device
       * authentication status */
      device_auth_handle_response(false);
      result->auth_device = send_flow_complete();
      *result_valid = true;
      next_state = FLOW_COMPLETE;
      break;
    }
    default: {
      usb_clear_event();
      break;
    }
  }

  return next_state;
}

static device_auth_state_e result_handler(const manager_query_t *query,
                                          manager_result_t *result,
                                          bool *result_valid) {
  pb_size_t request_type = get_request_type(&query->auth_device);
  device_auth_state_e next_state = FLOW_COMPLETE;

  switch (request_type) {
    case MANAGER_AUTH_DEVICE_REQUEST_INITIATE_TAG:
    case MANAGER_AUTH_DEVICE_REQUEST_CHALLENGE_TAG: {
      usb_clear_event();
      break;
    }
    case MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG: {
      bool verified = query->auth_device.result.verified;
      device_auth_handle_response(verified);
      result->auth_device = send_flow_complete();
      *result_valid = true;
      next_state = FLOW_COMPLETE;
      break;
    }
    default: {
      usb_clear_event();
      break;
    }
  }

  return next_state;
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

  /* Set flow status */
  core_status_set_flow_status(MANAGER_AUTH_DEVICE_STATUS_USER_CONFIRMED);
  delay_scr_init(ui_text_message_device_authenticating, 100);

  /* First state of the device authentication would be SIGN_SERIAL_NUMBER */
  device_auth_state_e state = SIGN_SERIAL_NUM;
  evt_status_t event = {0};

  /* Query to be decoded on USB event reception */
  manager_query_t decoded_query = MANAGER_QUERY_INIT_ZERO;

  while (FLOW_COMPLETE != state) {
    manager_result_t result =
        get_manager_result_template(MANAGER_RESULT_AUTH_DEVICE_TAG);

    bool result_valid = false;

    switch (state) {
      case SIGN_SERIAL_NUM: {
        state = sign_serial_handler(query, &result, &result_valid);
        break;
      }
      case SIGN_RANDOM_NUM: {
        state = sign_random_handler(query, &result, &result_valid);
        break;
      }
      case RESULT: {
        state = result_handler(query, &result, &result_valid);
        break;
      }
      default: {
        usb_clear_event();
        break;
      }
    }

    if (true == result_valid) {
      uint8_t encoded_response[DEVICE_AUTH_RESPONSE_SIZE] = {0};
      ASSERT(encode_and_send_manager_result(
          &result, &encoded_response[0], sizeof(encoded_response)));
    }

    if (FLOW_COMPLETE == state) {
      /* We don't want any events if state == FLOW_COMPLETE */
      /* While loop will end automatically */
      continue;
    }

    event = get_events(EVENT_CONFIG_USB, MAX_INACTIVITY_TIMEOUT);

    if (true == event.p0_event.flag) {
      if (true == event.p0_event.abort_evt) {
        break;
      }
    }

    /* Zeroize decoded_query structure */
    memzero(&decoded_query, sizeof(decoded_query));
    query = &decoded_query;

    if (false == decode_manager_query(event.usb_event.p_msg,
                                      event.usb_event.msg_size,
                                      &decoded_query)) {
      // TODO: Handle bad data error
      // TODO: Early flow exit
      return;
    }

    if (false == check_manager_request(query, MANAGER_QUERY_AUTH_DEVICE_TAG)) {
      // TODO: Handle bad data error
      // TODO: Early flow exit
    }
  }

  return;
}
