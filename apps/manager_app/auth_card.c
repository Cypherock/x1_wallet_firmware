
/**
 * @file    auth_card.c
 * @author  Cypherock X1 Team
 * @brief   Card authentication flow module
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
#include "app_error.h"
#include "card_operations.h"
#include "common_error.h"
#include "manager_api.h"
#include "manager_app.h"
#include "manager_app_priv.h"
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
#define FLOW_COMPLETE_STATE(auth_card_data)                                    \
  (AUTH_CARD_COMPLETE == (auth_card_data)->state)

#define CHALLENGE_SIZE 32

#define SIGN_SERIAL_BEEP_COUNT(pair_card_required) (pair_card_required) ? 3 : 2
#define SIGN_CHALLENGE_BEEP_COUNT(pair_card_required)                          \
  (pair_card_required) ? 2 : 1

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef struct {
  char heading[30];
  char message[100];
  uint8_t acceptable_cards;
  uint8_t family_id[FAMILY_ID_SIZE];
  bool pair_card_required;
} auth_card_screen_ctx_t;

typedef enum {
  AUTH_CARD_SERIAL = 1,
  AUTH_CARD_CHALLENGE,
  AUTH_CARD_RESULT,
  AUTH_CARD_COMPLETE,
} auth_card_states_e;

typedef struct {
  manager_query_t *query;
  auth_card_screen_ctx_t ctx;
  auth_card_states_e state;
} auth_card_data_t;
/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * The function sends a response to host indicating that a flow has been
 * completed.
 * @param auth_card_data object of auth card data
 */
static void send_flow_complete_response(auth_card_data_t *auth_card_data);

/**
 * @brief Read data from initiate request and prepare context for card
 * authentication
 *
 * @param auth_card_data object of auth card data
 * @return bool true sent encoded data successfully, else return false
 */
static bool prepare_card_auth_context(auth_card_data_t *auth_card_data);

/**
 * @brief Handles signing of authentication data.
 * This function handles the signing of authentication data based on the type of
 * request received.
 *
 * @details This function initializes sign configuration parameters, sets data
 * and sign type based on request type, and returns success or failure based on
 * the status of card_sign_auth_data function.
 *
 * @param auth_card_data Pointer to authentication card data.
 *
 * @return bool Returns true on success, else false
 * on failure.
 */
static bool handle_sign_data(auth_card_data_t *auth_card_data);

/**
 * @brief Helper to return signature of card serial number.
 * Handles serial signing task and populates resp object with appropriate
 * response
 *
 * @param auth_card_data object of auth card data
 * @return bool true if successful, else false
 */
static bool handle_sign_card_serial(auth_card_data_t *auth_card_data);

/**
 * @brief Helper to return signature of server challenge.
 * Handles challenge signing task and populates resp object with appropriate
 * response
 *
 * @param auth_card_data object of auth card data
 * @return bool true if successful, else false
 * code
 */
static bool handle_sign_challenge(auth_card_data_t *auth_card_data);

/**
 * @brief Helper to handle initiate query for auth card.
 * Hanlder performs following actions
 * - Get user's confirmation for proceeding with card auth
 * - Prepare context for auth card flow
 * - returns sign serial response
 *
 * @param auth_card_data object of auth card data
 * @return bool true if successful, else false
 * code
 */
static bool handle_auth_card_initiate_query(auth_card_data_t *auth_card_data);

/**
 * @brief Helper to handle challenge query for auth card.
 * Handles challenge signing task and populates resp object with appropriate
 * response
 *
 * @param auth_card_data object of auth card data
 * @return bool true if successful, else false
 * code
 */
static bool handle_auth_card_challenge_query(auth_card_data_t *auth_card_data);

/**
 * @brief Helper to handle result of auth card.
 * Handles pairing of card based on result, flow state and card pair requirement
 *
 * @param auth_card_data object of auth card data
 * @return bool true if successful, else false
 */
static bool handle_auth_card_result_query(auth_card_data_t *auth_card_data);

/**
 * @brief Helper to parse handle queries for auth card.
 * Hanlder performs following actions
 * - Decode host query for manager app
 * - Verify query is for card auth and type of query
 * - Call relevant handler for queries
 *
 * @param auth_card_data object of auth card data
 * @return bool true if successful, else false
 * code
 */
static bool handle_auth_card_query(auth_card_data_t *auth_card_data);
/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static bool prepare_card_auth_context(auth_card_data_t *auth_card_data) {
  auth_card_data->ctx.acceptable_cards = ACCEPTABLE_CARDS_ALL;

  if (auth_card_data->query->auth_card.initiate.has_card_index) {
    if (4 < auth_card_data->query->auth_card.initiate.card_index ||
        0 == auth_card_data->query->auth_card.initiate.card_index) {
      manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }
    auth_card_data->ctx.acceptable_cards = encode_card_number(
        auth_card_data->query->auth_card.initiate.card_index);
    snprintf(auth_card_data->ctx.heading,
             sizeof(auth_card_data->ctx.heading),
             UI_TEXT_TAP_CARD,
             (uint8_t)auth_card_data->query->auth_card.initiate.card_index);
  } else if (MANAGER_ONBOARDING_STEP_COMPLETE != onboarding_get_last_step()) {
    // In onboading card auth flow, a card index is required, if not sent by
    // host, exit
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_DATA);
    return false;
  } else {
    snprintf(auth_card_data->ctx.heading,
             sizeof(auth_card_data->ctx.heading),
             ui_text_tap_a_card);
  }

  auth_card_data->ctx.pair_card_required = false;
  if (auth_card_data->query->auth_card.initiate.has_is_pair_required) {
    auth_card_data->ctx.pair_card_required =
        auth_card_data->query->auth_card.initiate.is_pair_required;
  }
  snprintf(auth_card_data->ctx.message,
           sizeof(auth_card_data->ctx.message),
           UI_TEXT_PLACE_CARD_TILL_BEEP,
           SIGN_SERIAL_BEEP_COUNT(auth_card_data->ctx.pair_card_required));

  memcpy(auth_card_data->ctx.family_id, get_family_id(), FAMILY_ID_SIZE);
  return true;
}

static void send_flow_complete_response(auth_card_data_t *auth_card_data) {
  manager_result_t result = init_manager_result(MANAGER_RESULT_AUTH_CARD_TAG);
  result.auth_card.which_response =
      MANAGER_AUTH_CARD_RESPONSE_FLOW_COMPLETE_TAG;
  result.auth_card.flow_complete.dummy_field = 0;
  manager_send_result(&result);
  auth_card_data->state = AUTH_CARD_COMPLETE;
}

static bool handle_sign_data(auth_card_data_t *auth_card_data) {
  card_sign_data_config_t sign_config = {0};

  manager_result_t result = init_manager_result(MANAGER_RESULT_AUTH_CARD_TAG);
  manager_auth_card_response_t *resp = &result.auth_card;

  sign_config.acceptable_cards = auth_card_data->ctx.acceptable_cards;
  sign_config.family_id = auth_card_data->ctx.family_id;

  if (MANAGER_AUTH_CARD_REQUEST_INITIATE_TAG ==
      auth_card_data->query->auth_card.which_request) {
    sign_config.data = resp->serial_signature.serial;
    sign_config.sign_type = CARD_SIGN_SERIAL;
  } else if (MANAGER_AUTH_CARD_REQUEST_CHALLENGE_TAG ==
             auth_card_data->query->auth_card.which_request) {
    sign_config.data = auth_card_data->query->auth_card.challenge.challenge;
    sign_config.data_size = CHALLENGE_SIZE;
    sign_config.sign_type = CARD_SIGN_CUSTOM;
  } else {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  instruction_scr_init(auth_card_data->ctx.message,
                       auth_card_data->ctx.heading);
  card_error_type_e status = card_sign_auth_data(&sign_config);

  switch (status) {
    case CARD_OPERATION_SUCCESS:
      if (MANAGER_AUTH_CARD_REQUEST_INITIATE_TAG ==
          auth_card_data->query->auth_card.which_request) {
        memcpy(resp->serial_signature.signature,
               sign_config.signature,
               sizeof(resp->serial_signature.signature));
        result.auth_card.which_response =
            MANAGER_AUTH_CARD_RESPONSE_SERIAL_SIGNATURE_TAG;
        if (ACCEPTABLE_CARDS_ALL == auth_card_data->ctx.acceptable_cards) {
          auth_card_data->ctx.acceptable_cards =
              ACCEPTABLE_CARDS_ALL ^ sign_config.acceptable_cards;
        }
        manager_send_result(&result);
      } else if (MANAGER_AUTH_CARD_REQUEST_CHALLENGE_TAG ==
                 auth_card_data->query->auth_card.which_request) {
        memcpy(resp->challenge_signature.signature,
               sign_config.signature,
               sizeof(resp->challenge_signature.signature));
        result.auth_card.which_response =
            MANAGER_AUTH_CARD_RESPONSE_CHALLENGE_SIGNATURE_TAG;
        manager_send_result(&result);
      }
      return true;
      break;

    case CARD_OPERATION_P0_OCCURED:
      return false;
      break;

    default:
      manager_send_error(
          ERROR_COMMON_ERROR_CARD_ERROR_TAG,
          (uint32_t)get_card_error_from_nfc_status(sign_config.status));
      return false;
      break;
  }
}

static bool handle_sign_card_serial(auth_card_data_t *auth_card_data) {
  if (!handle_sign_data(auth_card_data)) {
    return false;
  }

  set_app_flow_status(MANAGER_AUTH_CARD_STATUS_SERIAL_SIGNED);

  // Display text for challenge sign screen
  snprintf(auth_card_data->ctx.message,
           sizeof(auth_card_data->ctx.message),
           UI_TEXT_PLACE_CARD_TILL_BEEP,
           SIGN_CHALLENGE_BEEP_COUNT(auth_card_data->ctx.pair_card_required));
  instruction_scr_init(auth_card_data->ctx.message,
                       auth_card_data->ctx.heading);
  auth_card_data->state = AUTH_CARD_CHALLENGE;
  return true;
}

static bool handle_sign_challenge(auth_card_data_t *auth_card_data) {
  if (!handle_sign_data(auth_card_data)) {
    return false;
  }

  set_app_flow_status(MANAGER_AUTH_CARD_STATUS_CHALLENGE_SIGNED);

  if (auth_card_data->ctx.pair_card_required) {
    snprintf(auth_card_data->ctx.message,
             sizeof(auth_card_data->ctx.message),
             UI_TEXT_PLACE_CARD_TILL_BEEP,
             1);
  } else {
    snprintf(auth_card_data->ctx.message,
             sizeof(auth_card_data->ctx.message),
             ui_text_processing);
  }

  instruction_scr_init(auth_card_data->ctx.message,
                       auth_card_data->ctx.heading);
  auth_card_data->state = AUTH_CARD_RESULT;
  return true;
}

static bool handle_auth_card_initiate_query(auth_card_data_t *auth_card_data) {
  if (AUTH_CARD_SERIAL != auth_card_data->state) {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  if (MANAGER_ONBOARDING_STEP_COMPLETE == onboarding_get_last_step()) {
    if (!core_confirmation(ui_text_start_verification_of_card,
                           &manager_send_error)) {
      return false;
    }
  }

  set_app_flow_status(MANAGER_AUTH_CARD_STATUS_USER_CONFIRMED);

  if (!prepare_card_auth_context(auth_card_data)) {
    return false;
  }

  return handle_sign_card_serial(auth_card_data);
}

static bool handle_auth_card_challenge_query(auth_card_data_t *auth_card_data) {
  if (AUTH_CARD_CHALLENGE != auth_card_data->state) {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return handle_sign_challenge(auth_card_data);
}

static bool handle_auth_card_result_query(auth_card_data_t *auth_card_data) {
  bool verified = auth_card_data->query->auth_card.result.verified;

  switch (auth_card_data->state) {
    case AUTH_CARD_CHALLENGE:
      if (false == verified) {
        send_flow_complete_response(auth_card_data);

        delay_scr_init(ui_text_card_authentication_failed, DELAY_TIME);
        return true;
      } else {
        manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_REQUEST);
        return false;
      }
      break;

    case AUTH_CARD_RESULT:
      if (false == verified) {
        send_flow_complete_response(auth_card_data);

        delay_scr_init(ui_text_card_authentication_failed, DELAY_TIME);
        return true;
      } else {
        if (true == auth_card_data->ctx.pair_card_required) {
          uint8_t card_number =
              decode_card_number(auth_card_data->ctx.acceptable_cards);

          uint32_t pairing_status = DEFAULT_VALUE_IN_FLASH;
          if (CARD_OPERATION_SUCCESS !=
              card_pair_without_retap(card_number, &pairing_status)) {
            LOG_ERROR("pairing error: %ld", pairing_status);
            manager_send_error(ERROR_COMMON_ERROR_CARD_ERROR_TAG,
                               get_card_error_from_nfc_status(pairing_status));
            if (SW_CONDITIONS_NOT_SATISFIED != pairing_status &&
                SW_FILE_INVALID != pairing_status) {
              delay_scr_init(ui_text_card_authentication_failed, DELAY_TIME);
            }
            return false;
          }
          set_app_flow_status(MANAGER_AUTH_CARD_STATUS_PAIRING_DONE);
        }

        send_flow_complete_response(auth_card_data);

        /**
         * Set onboarding complete here if 4th card auth is complete. The
         * onboarding_set_step_done API internally verfies if onboarding was
         * complete or not.
         */
        if (0x08 == auth_card_data->ctx.acceptable_cards) {
          onboarding_set_step_done(MANAGER_ONBOARDING_STEP_COMPLETE);
        }

        return true;
      }
      break;

    default:
      manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
      break;
  }

  return false;
}

static bool handle_auth_card_query(auth_card_data_t *auth_card_data) {
  switch (auth_card_data->query->auth_card.which_request) {
    case MANAGER_AUTH_CARD_REQUEST_INITIATE_TAG:
      return handle_auth_card_initiate_query(auth_card_data);
      break;

    case MANAGER_AUTH_CARD_REQUEST_CHALLENGE_TAG:
      return handle_auth_card_challenge_query(auth_card_data);
      break;

    case MANAGER_AUTH_CARD_REQUEST_RESULT_TAG:
      return handle_auth_card_result_query(auth_card_data);
      break;

    default:
      manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                         ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
      break;
  }

  return false;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
void card_auth_handler(manager_query_t *query) {
  ASSERT(NULL != query);

  /* Validate if this flow is allowed */
  if (!onboarding_step_allowed(MANAGER_ONBOARDING_STEP_CARD_AUTHENTICATION)) {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_QUERY_NOT_ALLOWED);
    return;
  }

  if (MANAGER_AUTH_CARD_REQUEST_INITIATE_TAG !=
      query->auth_card.which_request) {
    manager_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                       ERROR_DATA_FLOW_INVALID_REQUEST);
    return;
  }

  auth_card_data_t auth_card_data = {
      .ctx =
          {
              .acceptable_cards = ACCEPTABLE_CARDS_ALL,
              .heading = "",
              .message = "",
              .pair_card_required = false,
          },
      .query = query,
      .state = AUTH_CARD_SERIAL};

  while (1) {
    if (true != handle_auth_card_query(&auth_card_data)) {
      return;
    }

    if (FLOW_COMPLETE_STATE(&auth_card_data)) {
      return;
    }

    if (false ==
        manager_get_query(auth_card_data.query, MANAGER_QUERY_AUTH_CARD_TAG)) {
      return;
    }
  }
  return;
}
