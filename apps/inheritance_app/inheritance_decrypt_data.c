/**
 * @author  Cypherock X1 Team
 * @brief   Inheritance message decryption login
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
#include <stdio.h>
#include <string.h>

#include "card_fetch_data.h"
#include "card_operation_typedefs.h"
#include "constant_texts.h"
#include "core_session.h"
#include "inheritance/core.pb.h"
#include "inheritance/decrypt_data_with_pin.pb.h"
#include "inheritance_api.h"
#include "inheritance_context.h"
#include "inheritance_priv.h"
#include "reconstruct_wallet_flow.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "utils.h"
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
 * @brief Checks if the given request matches the expected request type.
 *
 * @param query Pointer to the inheritance query.
 * @param which_request The expected request type.
 * @return true if the request type matches, false otherwise.
 */
static bool check_which_request(const inheritance_query_t *query,
                                pb_size_t which_request);

/**
 * @brief Validates the request data for decryption.
 *
 * @param request Pointer to the decryption request data.
 * @return true if the request data is valid, false otherwise.
 */
static bool validate_request_data(
    const inheritance_decrypt_data_with_pin_request_t *request);

/**
 * @brief Handles the initiation of the decryption query.
 *
 * @param query Pointer to the inheritance query.
 * @return true if the initiation is successful, false otherwise.
 */
STATIC bool inheritance_handle_initiate_query(inheritance_query_t *query);

/**
 * @brief Sends the decrypted data as a response.
 *
 * @param query Pointer to the inheritance query.
 * @return true if sending the response is successful, false otherwise.
 */
static bool send_decrypted_data(inheritance_query_t *query);

/**
 * @brief Decrypts the packet containing encrypted data.
 *
 * @return true if decryption is successful, false otherwise.
 */
static bool decrypt_packet(void);

/**
 * @brief Deserializes the decrypted packet into individual data items.
 *
 * @return true if deserialization is successful, false otherwise.
 */
static bool deserialize_packet(void);

/**
 * @brief Decrypts the message data within the packet.
 *
 * @return true if message data decryption is successful, false otherwise.
 */
static bool decrypt_message_data(void);

/**
 * @brief Decrypts the overall data.
 *
 * @return true if decryption is successful, false otherwise.
 */
static bool decrypt_data(void);

/**
 * @brief Displays decrypted data or performs related actions based on the data.
 *
 * This function processes the decrypted data and either shows it on the device
 * screen (if the tag indicates display-only) or prepares it for further
 * response.
 *
 * @return true if the operation is successful, false otherwise.
 */
static bool show_data(void);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

STATIC inheritance_decryption_context_t *decryption_context = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool check_which_request(const inheritance_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->decrypt.which_request) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  return true;
}

static bool validate_request_data(
    const inheritance_decrypt_data_with_pin_request_t *request) {
  bool status = true;

  // TODO: check the current session validity here

  return status;
}

STATIC bool inheritance_handle_initiate_query(inheritance_query_t *query) {
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(
          query, INHERITANCE_DECRYPT_DATA_WITH_PIN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->decrypt) ||
      !get_wallet_name_by_id(query->decrypt.initiate.wallet_id,
                             (uint8_t *)wallet_name,
                             inheritance_send_error)) {
    return false;
  }

  snprintf(msg,
           sizeof(msg),
           ui_text_inheritance_decryption_flow_confirmation,
           wallet_name);

  if (!core_confirmation(msg, inheritance_send_error)) {
    return false;
  }

  set_app_flow_status(INHERITANCE_DECRYPT_DATA_STATUS_USER_CONFIRMED);

  decryption_context->request_pointer = &(query->decrypt.initiate);
  return true;
}

static bool send_decrypted_data(inheritance_query_t *query) {
  inheritance_result_t result =
      init_inheritance_result(INHERITANCE_RESULT_DECRYPT_TAG);
  result.decrypt.which_response =
      INHERITANCE_DECRYPT_DATA_WITH_PIN_RESPONSE_MESSAGES_TAG;
  if (!inheritance_get_query(query, INHERITANCE_QUERY_DECRYPT_TAG) ||
      !check_which_request(
          query, INHERITANCE_DECRYPT_DATA_WITH_PIN_REQUEST_INITIATE_TAG)) {
    return false;
  }

  memcpy(&result.decrypt.messages,
         &decryption_context->response,
         sizeof(inheritance_decrypt_data_with_pin_messages_response_t));

  inheritance_send_result(&result);
  return true;
}

static bool decrypt_packet(void) {
  decryption_context->packet_size =
      decryption_context->request_pointer->encrypted_data.size;
  memcpy(decryption_context->packet,
         decryption_context->request_pointer->encrypted_data.bytes,
         decryption_context->packet_size);
  return session_aes_decrypt(decryption_context->packet,
                             &decryption_context->packet_size) ==
         SESSION_DECRYPT_PACKET_SUCCESS;
}

static bool deserialize_packet(void) {
  uint16_t packet_index = 0;
  decryption_context->data_count = decryption_context->packet[packet_index++];
  for (uint8_t index = 0; index < decryption_context->data_count; index++) {
    packet_index++;    ///< Skip tag

    decryption_context->data[index].encrypted_data_size =
        U16_READ_BE_ARRAY(&decryption_context->packet[packet_index]);
    packet_index += 2;    ///< Read length

    memcpy(decryption_context->data[index].encrypted_data,
           &decryption_context->packet[packet_index],
           decryption_context->data[index].encrypted_data_size);
    packet_index += decryption_context->data[index].encrypted_data_size;
  }

  return packet_index <= decryption_context->packet_size;
}

static bool decrypt_message_data(void) {
  return card_fetch_decrypt_data(decryption_context->request_pointer->wallet_id,
                                 decryption_context->data,
                                 decryption_context->data_count) ==
         CARD_OPERATION_SUCCESS;
}

static bool decrypt_data(void) {
  bool status = true;

  do {
    if (!decrypt_packet()) {
      // TODO: Throw packet decryption error
      status = false;
      break;
    }

    if (!deserialize_packet()) {
      // TODO: Throw packet serialization error
      status = false;
      break;
    }
    if (!decrypt_message_data()) {
      // TODO: Throw decryption failed
      status = false;
      break;
    }

  } while (0);
  set_app_flow_status(INHERITANCE_DECRYPT_DATA_STATUS_MESSAGE_DECRYPTED);
  return status;
}

static bool show_data(void) {
  pb_size_t response_count = 0;

  for (uint8_t i = 0; i < decryption_context->data_count; i++) {
    uint8_t tag = decryption_context->data[i].plain_data[0];

    if (tag == INHERITANCE_ONLY_SHOW_ON_DEVICE) {
      if (!core_scroll_page(
              UI_TEXT_VERIFY_MESSAGE,
              (const char *)&decryption_context->data[i]
                  .plain_data[3],    ///> sizeof (tag) + sizeof (length) = 3
              inheritance_send_error)) {
        return false;
      }
    } else {
      uint16_t offset = 1;    // Skip tag
      decryption_context->response.plain_data[response_count].message.size =
          U16_READ_BE_ARRAY(decryption_context->data[i].plain_data + offset);
      offset += 2;    // Skip length
      memcpy(
          decryption_context->response.plain_data[response_count].message.bytes,
          decryption_context->data[i].plain_data + offset,
          decryption_context->response.plain_data[response_count].message.size);
      decryption_context->response.plain_data_count = ++response_count;
    }
  }

  set_app_flow_status(INHERITANCE_DECRYPT_DATA_STATUS_PIN_VERIFIED);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void inheritance_decrypt_data(inheritance_query_t *query) {
  decryption_context = (inheritance_decryption_context_t *)malloc(
      sizeof(inheritance_decryption_context_t));
  memzero(decryption_context, sizeof(inheritance_decryption_context_t));

  if (inheritance_handle_initiate_query(query) && decrypt_data() &&
      show_data() && send_decrypted_data(query)) {
    delay_scr_init(ui_text_inheritance_decryption_flow_success, DELAY_TIME);
  } else {
    delay_scr_init(ui_text_inheritance_decryption_flow_failure, DELAY_TIME);
  }
  delay_scr_init(ui_text_check_cysync, DELAY_TIME);

  memzero(decryption_context, sizeof(inheritance_decryption_context_t));
  free(decryption_context);
  decryption_context = NULL;
}
