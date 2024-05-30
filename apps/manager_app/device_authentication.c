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
#include "manager_api.h"
#include "onboarding.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
// TODO: Eventually DEVICE_AUTH_RESPONSE_SIZE will be replaced by
// MANAGER_RESULT_SIZE when all option files for manager app are complete
#define DEVICE_AUTH_RESPONSE_SIZE (MANAGER_AUTH_DEVICE_RESPONSE_SIZE + 20)

// TODO: Make common handler

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
static pb_size_t get_which_request(
    const manager_auth_device_request_t *request);

/**
 * @brief Sends response of type manager_auth_device_response_t to the host. It
 * internally encodes the response to type manager_result_t
 *
 * @param resp Reference to the response to be send to the host.
 */
static void send_auth_device_response(manager_auth_device_response_t *resp);

/**
 * @brief This API sends the response for device authentication flow completion
 * to the host.
 *
 */
static void send_auth_device_flow_complete(void);

/**
 * @brief This function handles the verification result of the device
 * authentication process
 *
 * @param verified true or false, based on boolean result sent by the host.
 */
static void auth_device_handle_response(bool verified);

/**
 * @brief This function is the request handler for the SIGN_SERIAL_NUM state of
 * the device authentication flow. It is made sure that the USB event is
 * consumed in all cases.
 *
 * In SIGN_SERIAL_NUM state: A query of type
 * MANAGER_AUTH_DEVICE_REQUEST_INITIATE_TAG is expected. If this is recieved,
 * then the request handler calls sign_serial_number() API which performs serial
 * number signing from the ATECC and sends the response to the host. It then
 * advances the next_state to SIGN_RANDOM_NUM.
 *
 * In the case any other query is received, then an early exit is triggered by
 * setting the next_state to FLOW_COMPLETE.
 *
 * @param query Reference to query of type manager_query_t as received from the
 * host
 * @return device_auth_state_e The next state of the flow
 */
static device_auth_state_e sign_serial_handler(const manager_query_t *query);

/**
 * @brief This function is the request handler for the SIGN_RANDOM_NUM state of
 * the device authentication flow. It is made sure that the USB event is
 * consumed in all cases.
 *
 * In SIGN_RANDOM_NUM state:
 * A query of type MANAGER_AUTH_DEVICE_REQUEST_CHALLENGE_TAG is expected. If
 * this is recieved, then the request handler calls sign_random_challenge() API
 * which performs random number signing from the ATECC and sends the response to
 * the host.
 *
 * In case a query of type MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG is received,
 * then the request handler assumes that it the serial number signature was NOT
 * verified and sets the device in un-authenticated state and exits the flow by
 * setting next_state = FLOW_COMPLETE.
 *
 * In case a query of type MANAGER_AUTH_DEVICE_REQUEST_INITIATE_TAG is received,
 * then it advances the next_state to FLOW_COMPLETE.
 *
 * @param query Reference to query of type manager_query_t as received from the
 * host
 * @return device_auth_state_e The next state of the flow
 */
static device_auth_state_e sign_random_handler(const manager_query_t *query);

/**
 * @brief This function is the request handler for the RESULT state of
 * the device authentication flow. It is made sure that the USB event is
 * consumed in all cases.
 *
 * In RESULT state:
 * A query of type MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG is expected.
 * It handles the result returned by the host by setting the authentication
 * result on the flash and sends flow completion message to the host and
 * advances the next_state to FLOW_COMPLETE.
 *
 * In case a query of type MANAGER_AUTH_DEVICE_REQUEST_CHALLENGE_TAG or
 * MANAGER_AUTH_DEVICE_REQUEST_INITIATE_TAG is received, then the next_state is
 * advanced to FLOW_COMPLETE.
 *
 * @param query Reference to query of type manager_query_t as received from the
 * host
 * @return device_auth_state_e The next state of the flow
 */
static device_auth_state_e result_handler(const manager_query_t *query);
/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static pb_size_t get_which_request(
    const manager_auth_device_request_t *request) {
  return request->which_request;
}

static void send_auth_device_response(manager_auth_device_response_t *resp) {
  manager_result_t result = init_manager_result(MANAGER_RESULT_AUTH_DEVICE_TAG);
  memcpy(&(result.auth_device), resp, sizeof(manager_auth_device_response_t));
  manager_send_result(&result);
  return;
}

static void send_auth_device_flow_complete(void) {
  manager_auth_device_response_t response =
      MANAGER_AUTH_DEVICE_RESPONSE_INIT_ZERO;
  response.which_response = MANAGER_AUTH_DEVICE_RESPONSE_FLOW_COMPLETE_TAG;
  response.flow_complete.dummy_field = '\0';
  send_auth_device_response(&response);
  return;
}

static void auth_device_handle_response(bool verified) {
  if (true == verified) {
    /* Update onboarding status to save progress */
    onboarding_set_step_done(MANAGER_ONBOARDING_STEP_DEVICE_AUTH);
    set_auth_state(DEVICE_AUTHENTICATED);
  } else {
    set_auth_state(DEVICE_NOT_AUTHENTICATED);
  }

  return;
}

static device_auth_state_e sign_serial_handler(const manager_query_t *query) {
  pb_size_t request_type = get_which_request(&query->auth_device);
  device_auth_state_e next_state = FLOW_COMPLETE;

  switch (request_type) {
    case MANAGER_AUTH_DEVICE_REQUEST_INITIATE_TAG: {
      /* We need to get user confirmation only if auth is triggered via
       * settings. This can be determined by ensuring all of the following:
       * 1. onboarding is complete
       * 2. device is already authenticated (i.e., it is on main menu)
       */
      if ((MANAGER_ONBOARDING_STEP_COMPLETE == onboarding_get_last_step() &&
           is_device_authenticated()) &&
          !core_confirmation(ui_text_start_device_verification,
                             manager_send_error)) {
        // re-authentication denied by user
        next_state = FLOW_COMPLETE;
        break;
      }

      /* Set flow status */
      set_app_flow_status(MANAGER_AUTH_DEVICE_STATUS_USER_CONFIRMED);
      delay_scr_init(ui_text_message_device_authenticating, 100);

      manager_auth_device_response_t resp = sign_serial_number();
      send_auth_device_response(&resp);
      next_state = SIGN_RANDOM_NUM;
      break;
    }
    case MANAGER_AUTH_DEVICE_REQUEST_CHALLENGE_TAG:
    case MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG:
    default: {
      /* In case any other request is received, then we exit the flow early */
      manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
      break;
    }
  }

  return next_state;
}

static device_auth_state_e sign_random_handler(const manager_query_t *query) {
  pb_size_t request_type = get_which_request(&query->auth_device);
  device_auth_state_e next_state = FLOW_COMPLETE;

  switch (request_type) {
    case MANAGER_AUTH_DEVICE_REQUEST_INITIATE_TAG: {
      manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
      break;
    }
    case MANAGER_AUTH_DEVICE_REQUEST_CHALLENGE_TAG: {
      uint8_t *challenge =
          (uint8_t *)&(query->auth_device.challenge.challenge[0]);

      manager_auth_device_response_t resp = sign_random_challenge(challenge);
      send_auth_device_response(&resp);
      next_state = RESULT;
      break;
    }
    case MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG: {
      /* If SIGN_RANDOM_NUM == state, but the request_type received is
       * MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG, it's an unexpected step in
       * the flow. The device will treat it as an attempt to force device
       * authentication status */
      auth_device_handle_response(false);
      send_auth_device_flow_complete();

      delay_scr_init(ui_text_message_device_auth_failure, DELAY_TIME);
      next_state = FLOW_COMPLETE;
      break;
    }
    default: {
      manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
      usb_clear_event();
      break;
    }
  }

  return next_state;
}

static device_auth_state_e result_handler(const manager_query_t *query) {
  pb_size_t request_type = get_which_request(&query->auth_device);
  device_auth_state_e next_state = FLOW_COMPLETE;

  switch (request_type) {
    case MANAGER_AUTH_DEVICE_REQUEST_INITIATE_TAG:
    case MANAGER_AUTH_DEVICE_REQUEST_CHALLENGE_TAG: {
      manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
      break;
    }
    case MANAGER_AUTH_DEVICE_REQUEST_RESULT_TAG: {
      bool verified = query->auth_device.result.verified;
      auth_device_handle_response(verified);
      send_auth_device_flow_complete();

      if (true == verified) {
        delay_scr_init(ui_text_message_device_auth_success, DELAY_TIME);
      } else {
        delay_scr_init(ui_text_message_device_auth_failure, DELAY_TIME);
      }

      next_state = FLOW_COMPLETE;
      break;
    }
    default: {
      manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
      break;
    }
  }

  return next_state;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void device_authentication_flow(manager_query_t *query) {
  /* Validate if this flow is allowed */
  if (!onboarding_step_allowed(MANAGER_ONBOARDING_STEP_DEVICE_AUTH)) {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_QUERY_NOT_ALLOWED);
    return;
  }

  /* First state of the device authentication would be SIGN_SERIAL_NUMBER */
  device_auth_state_e state = SIGN_SERIAL_NUM;

  while (1) {
    switch (state) {
      case SIGN_SERIAL_NUM: {
        state = sign_serial_handler(query);
        break;
      }
      case SIGN_RANDOM_NUM: {
        state = sign_random_handler(query);
        break;
      }
      case RESULT: {
        state = result_handler(query);
        break;
      }
      default: {
        usb_clear_event();
        break;
      }
    }

    if (FLOW_COMPLETE == state) {
      break;
    }

    /* Zeroize decoded_query structure */
    memzero(query, sizeof(query));

    /* Get next query */
    if (!manager_get_query(query, MANAGER_QUERY_AUTH_DEVICE_TAG)) {
      break;
    }
  }

  return;
}
