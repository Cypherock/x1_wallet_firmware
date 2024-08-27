/**
 * @author  Cypherock X1 Team
 * @brief   Data encryption flow for inheritance
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

#include <stdint.h>

#include "card_fetch_data.h"
#include "core_session.h"
#include "inheritance/common.pb.h"
#include "inheritance/core.pb.h"
#include "inheritance/encrypt_data_with_pin.pb.h"
#include "inheritance_api.h"
#include "inheritance_priv.h"
#include "pb.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "verify_pin_flow.h"
#include "wallet.h"
#include "wallet_list.h"

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
 * @brief Checks if the provided query contains expected request.
 * @details The function performs the check on the request type and if the check
 * fails, then it will send an error to the host inheritance app and return
 * false.
 *
 * @param query Reference to an instance of inheritance_query_t containing query
 * received from host app
 * @param which_request The expected request type enum
 *
 * @return bool Indicating if the check succeeded or failed
 * @retval true If the query contains the expected request
 * @retval false If the query does not contain the expected request
 */
static bool check_which_request(const inheritance_query_t *query,
                                pb_size_t which_request);

/**
 * @brief Validates the derivation path received in the request from host
 * @details The function validates the provided account derivation path in the
 * request. If invalid path is detected, the function will send an error to the
 * host and return false.
 *
 * @param request Reference to an instance of inheritance_encrypt_data_request_t
 * @return bool Indicating if the verification passed or failed
 * @retval true If all the derivation path entries are valid
 * @retval false If any of the derivation path entries are invalid
 */
static bool validate_request_data(
    const inheritance_encrypt_data_with_pin_request_t *request);

/**
 * @brief Takes already received and decoded query for the user confirmation.
 * @details The function will verify if the query contains the
 * INHERITANCE_ENCRYPT_DATA type of request. Additionally, the wallet-id is
 * validated for sanity and the derivation path for the account is also
 * validated. After the validations, user is prompted about the action for
 * confirmation. The function returns true indicating all the validation and
 * user confirmation was a success. The function also duplicates the data from
 * query into the inheritance_txn_context  for further processing.
 *
 * @param query Constant reference to the decoded query received from the host
 *
 * @return bool Indicating if the function actions succeeded or failed
 * @retval true If all the validation and user confirmation was positive
 * @retval false If any of the validation or user confirmation was negative
 */
STATIC bool inheritance_handle_initiate_query(inheritance_query_t *query);

/**
 * @brief Aggregates user consent for the encrytion info
 * @details The function displays the required messages for user to very
 *
 *
 * @return bool Indicating if the user confirmed the messages
 * @retval true If user confirmed the messages displayed
 * @retval false Immediate return if any of the messages are disapproved
 */
STATIC bool inheritance_get_user_verification(void);

/**
 * @brief Sends the encrypted data to the host
 * @details The function encrypts the data and sends it to the host
 *
 * @param query Reference to an instance of inheritance_query_t to store
 * transient request from the host
 * @return bool Indicating if the encrypted data is sent to the host
 * @retval true If the encrypted data was sent to host successfully
 * @retval false If the host responded with unknown/wrong query
 */
static bool send_encrypted_data(inheritance_query_t *query);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

STATIC inheritance_encryption_context_t *context = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const inheritance_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->encrypt.which_request) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request_data(
    const inheritance_encrypt_data_with_pin_request_t *request) {
  bool status = true;
  Wallet wallet = {0};

  do {
    if (!get_wallet_data_by_id(
            request->initiate.wallet_id, &wallet, inheritance_send_error)) {
      status = false;
      break;
    }

    if (!WALLET_IS_PIN_SET(wallet.wallet_info)) {
      status = false;

      inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_REQUEST);
      break;
    }

  } while (0);

  return status;
}

STATIC bool inheritance_handle_initiate_query(inheritance_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(
          query, INHERITANCE_ENCRYPT_DATA_WITH_PIN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->encrypt) ||
      !get_wallet_name_by_id(query->encrypt.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             inheritance_send_error)) {
    return false;
  }

  snprintf(msg,
           sizeof(msg),
           "Proceed to encrypt data for %s",
           wallet_name);    // TODO: update message

  if (!core_confirmation(msg, inheritance_send_error)) {
    return false;
  }

  set_app_flow_status(INHERITANCE_ENCRYPT_DATA_STATUS_USER_CONFIRMED);

  context->request_pointer = &(query->encrypt.initiate);

  return true;
}

STATIC bool inheritance_get_user_verification(void) {
  for (int i = 0; i < context->request_pointer->plain_data_count; i++) {
    const inheritance_plain_data_t *data =
        &context->request_pointer->plain_data[i];

    if (data->has_is_verified_on_device && data->is_verified_on_device) {
      if (!core_scroll_page("Verify message",
                            (const char *)&data->message.bytes,
                            inheritance_send_error)) {
        return false;
      }
    }
  }

  set_app_flow_status(INHERITANCE_ENCRYPT_DATA_STATUS_MESSAGE_VERIFIED);
  return true;
}

static bool inheritance_verify_pin(void) {
  return verify_pin(context->request_pointer->wallet_id,
                    context->pin_value,
                    inheritance_send_error);
}

static void inheritance_fill_tlv(uint8_t *destination,
                                 uint16_t *starting_index,
                                 uint8_t tag,
                                 uint16_t length,
                                 const uint8_t *value) {
  destination[(*starting_index)++] = tag;
  destination[(*starting_index)++] = length;
  destination[(*starting_index)++] = (length >> 8);

  memcpy(destination + *starting_index, value, length);
  *starting_index = *starting_index + length;
}

static bool serialize_message_data(void) {
  if (context->request_pointer->plain_data_count >=
      INHERITANCE_MESSAGES_MAX_COUNT) {
    // TODO: Throw invalid message count error;
    return false;
  }
  pb_size_t index = 0;
  uint16_t zero_index = 0;
  for (index = 0; index < context->request_pointer->plain_data_count; index++) {
    uint16_t length = context->request_pointer->plain_data[index].message.size;
    if (length > (PLAIN_DATA_SIZE - 3)) {
      length = PLAIN_DATA_SIZE - 3;
    }
    zero_index = 0;
    inheritance_fill_tlv(
        context->data[index].plain_data,
        &zero_index,
        0x00,    ///< Default tag for every message
        length,
        context->request_pointer->plain_data[index].message.bytes);
    context->data[index].plain_data_size = length;
  }
  zero_index = 0;
  inheritance_fill_tlv(context->data[index].plain_data,
                       &zero_index,
                       0x01,    ///< Special tag for private messages
                       MAX_PIN_SIZE,
                       context->pin_value);
  context->data[index].plain_data_size = MAX_PIN_SIZE;

  context->data_count = context->request_pointer->plain_data_count + 1;
  memzero(context->pin_value, sizeof(context->pin_value));
  return true;
}

static bool encrypt_message_data(void) {
  card_error_type_e status = card_fetch_encrypt_data(
      context->request_pointer->wallet_id, context->data, context->data_count);

  if (status != CARD_OPERATION_SUCCESS) {
    // TODO: throw encryption failed error
    return false;
  }
  set_app_flow_status(INHERITANCE_AUTH_WALLET_STATUS_CARD_TAPPED);
  return true;
}

static bool serialize_packet(void) {
  context->packet_size = 0;
  context->packet[context->packet_size++] = context->data_count;
  pb_size_t index = 0;

  for (index = 0; index < context->data_count - 1; index++) {
    inheritance_fill_tlv(context->packet,
                         &context->packet_size,
                         0x00,
                         context->data[index].encrypted_data_size,
                         context->data[index].encrypted_data);
  }

  // The last encrypted message is the PIN
  inheritance_fill_tlv(context->packet,
                       &context->packet_size,
                       0x50,
                       context->data[index].encrypted_data_size,
                       context->data[index].encrypted_data);

  return true;
}

static bool encrypt_packet(void) {
  if (SESSION_ENCRYPT_PACKET_SUCCESS !=
      session_aes_encrypt(context->packet, &context->packet_size)) {
    return false;
  }

  return true;
}

static bool encrypt_data(void) {
  bool status = true;

  do {
    if (!inheritance_verify_pin()) {
      // TODO: Throw user rejceted
      core_confirmation("pin verification failed", inheritance_send_error);
      status = false;
      break;
    }
    if (!serialize_message_data()) {
      // TODO: Throw serialization failed
      core_confirmation("serialization failed", inheritance_send_error);
      status = false;
      break;
    }

    if (!encrypt_message_data()) {
      // TODO: Throw encryption failed
      core_confirmation("encryption failed", inheritance_send_error);
      status = false;
      break;
    }

    if (!serialize_packet()) {
      // TODO: Throw packet serialization error
      core_confirmation("packet serialization failed", inheritance_send_error);
      status = false;
      break;
    }

    if (!encrypt_packet()) {
      // TODO: Throw packet encryption error
      core_confirmation("packet encryption failed", inheritance_send_error);
      status = false;
      break;
    }
  } while (0);

  return status;
}

static bool send_encrypted_data(inheritance_query_t *query) {
  inheritance_result_t result =
      init_inheritance_result(INHERITANCE_RESULT_ENCRYPT_TAG);
  result.encrypt.which_response =
      INHERITANCE_ENCRYPT_DATA_WITH_PIN_RESPONSE_RESULT_TAG;
  if (!inheritance_get_query(query, INHERITANCE_QUERY_ENCRYPT_TAG) ||
      !check_which_request(
          query, INHERITANCE_ENCRYPT_DATA_WITH_PIN_REQUEST_INITIATE_TAG)) {
    return false;
  }

  result.encrypt.result.encrypted_data.size = context->packet_size;
  memcpy(&result.encrypt.result.encrypted_data.bytes,
         context->packet,
         context->packet_size);

  inheritance_send_result(&result);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void inheritance_encrypt_data(inheritance_query_t *query) {
  context = (inheritance_encryption_context_t *)malloc(
      sizeof(inheritance_encryption_context_t));
  ASSERT(context != NULL);
  memzero(context, sizeof(inheritance_encryption_context_t));

  if (inheritance_handle_initiate_query(query) &&
      inheritance_get_user_verification() && encrypt_data() &&
      send_encrypted_data(query)) {
    delay_scr_init(ui_text_check_cysync, DELAY_TIME);
  }

  core_confirmation("flow over", inheritance_send_error);

  free(context);
  context = NULL;
}
