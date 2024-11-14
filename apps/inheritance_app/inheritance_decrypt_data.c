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
#include "memzero.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "reconstruct_wallet_flow.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_screens.h"
#include "ui_state_machine.h"
#include "utils.h"
#include "wallet.h"
#include "wallet_list.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef struct {
  decryption_error_type_e type;
  decryption_flow_t flow;
} decryption_error_info_t;

static decryption_error_info_t decryption_error;

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define DECRYPTED_CHUNK_SIZE (2048)
#define SET_ERROR_TYPE(x) decryption_error.type = x
#define SET_FLOW_TAG(x) decryption_error.flow = x

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Sets error @ref DECRYPTION_error to defaults.
 */
static void decryption_set_defaults();

/**
 * @brief Error handler for inheritance DECRYPTION flow
 */
static void decryption_handle_errors();
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
STATIC bool inheritance_decryption_handle_initiate_query(
    inheritance_query_t *query);

/**
 * @brief Function responsible for decoding pb_encoded buffer to @ref
 * inheritance_decrypt_data_with_pin_encrypted_data_structure_t
 *
 * @return true if decoding successful, false otherwise.
 */
static bool decode_inheritance_encrypted_data(
    const uint8_t *data,
    uint16_t data_size,
    inheritance_decrypt_data_with_pin_encrypted_data_structure_t
        *encrypted_data);

/**
 * @brief Retrieves pb_encoded encrypted data chunks from the host and creates
 * input buffer for @ref decode_inheritance_encrypted_data().
 *
 * @return true if data is successfully retrieved and decoded, false otherwise.
 */
static bool inheritance_get_encrypted_data(inheritance_query_t *query);

/**
 * @brief Creates a pb encoded buffer of @ref
 * inheritance_decrypt_data_with_pin_decrypted_data_structure_t to be sent to
 * host
 *
 * @return true if encoding is successful, false otherwise.
 */
static bool get_pb_encoded_buffer(
    const inheritance_decrypt_data_with_pin_decrypted_data_structure_t *result,
    uint8_t *buffer,
    uint16_t max_buffer_len,
    size_t *bytes_written_out);

/**
 * @brief Sends input buffer to host in chunks
 *
 * @return true if chunking successful, false otherwise.
 */
static bool inheritance_send_in_chunks(inheritance_query_t *query,
                                       const uint8_t *buffer,
                                       const size_t buffer_len);

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
 * @brief Displays decrypted data or performs related actions based on the
 * data.
 *
 * This function processes the decrypted data and either shows it on the
 * device screen (if the tag indicates display-only) or prepares it for
 * further response.
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

static void decryption_set_defaults() {
  SET_FLOW_TAG(DECRYPTION_DEFAULT_START_FLOW);
  SET_ERROR_TYPE(DECRYPTION_ERROR_DEFAULT);
}

static void decryption_handle_errors() {
  if (decryption_error.type == DECRYPTION_OK) {
    return;
  }
  LOG_ERROR("inheritance_encrypt_data Error Code:%d Flow Tag:%d ",
            decryption_error.type,
            decryption_error.flow);
  decryption_error_type_e type = decryption_error.type;
  switch (type) {
    case DECRYPTION_ERROR_DEFAULT:
    case DECRYPTION_INVALID_REQUEST_ERROR:
    case DECRYPTION_INVALID_DATA_ERROR:
    case DECRYPTION_PB_ENCODE_FAIL_ERROR:
    case DECRYPTION_CHUNK_DATA_INVALID_ERROR: {
      inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_DATA);
    } break;
    case DECRYPTION_PB_DECODE_FAIL_ERROR: {
      inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_DECODING_FAILED);
    } break;

    case DECRYPTION_INVALID_WALLET_ID_ERROR: {
      inheritance_send_error(ERROR_COMMON_ERROR_WALLET_NOT_FOUND_TAG,
                             ERROR_DATA_FLOW_INVALID_DATA);
    } break;
    case DECRYPTION_USER_ABORT_FAILURE: {
      inheritance_send_error(ERROR_COMMON_ERROR_USER_REJECTION_TAG,
                             ERROR_DATA_FLOW_INVALID_DATA);
    } break;
    case DECRYPTION_CARD_DECRYPTION_FAIL_ERROR:
    // case DECRYPTION_MESSAGE_MAX_COUNT_EXCEED_ERROR:
    case DECRYPTION_SESSION_DECRYPTION_FAIL_ERROR:
    case DECRYPTION_ASSERT_MALLOC_ERROR:
    default: {
      inheritance_send_error(ERROR_COMMON_ERROR_UNKNOWN_ERROR_TAG,
                             ERROR_DATA_FLOW_INVALID_DATA);
    } break;
  }
}

static bool check_which_request(const inheritance_query_t *query,
                                pb_size_t which_request) {
  if (which_request != query->decrypt.which_request) {
    SET_ERROR_TYPE(DECRYPTION_INVALID_REQUEST_ERROR);
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

STATIC bool inheritance_decryption_handle_initiate_query(
    inheritance_query_t *query) {
  SET_FLOW_TAG(DECRYPTION_QUERY_HANDLE_FLOW);
  char wallet_name[NAME_SIZE] = "";
  char msg[100] = "";

  if (!check_which_request(
          query, INHERITANCE_DECRYPT_DATA_WITH_PIN_REQUEST_INITIATE_TAG) ||
      !validate_request_data(&query->decrypt)) {
    return false;
  }

  if (get_wallet_name_by_id(
          query->decrypt.initiate.wallet_id, (uint8_t *)wallet_name, NULL)) {
    snprintf(msg,
             sizeof(msg),
             ui_text_inheritance_decryption_flow_confirmation,
             wallet_name);

  } else {
    snprintf(msg,
             sizeof(msg),
             "%s",
             ui_text_inheritance_decryption_flow_confirmation_generic);
  }

  if (!core_confirmation(msg, inheritance_send_error)) {
    SET_ERROR_TYPE(DECRYPTION_USER_ABORT_FAILURE);
    return false;
  }

  set_app_flow_status(INHERITANCE_DECRYPT_DATA_STATUS_USER_CONFIRMED);

  memcpy(decryption_context->wallet_id,
         query->decrypt.initiate.wallet_id,
         WALLET_ID_SIZE);
  inheritance_result_t result =
      init_inheritance_result(INHERITANCE_RESULT_DECRYPT_TAG);
  result.decrypt.which_response =
      INHERITANCE_DECRYPT_DATA_WITH_PIN_RESPONSE_CONFIRMATION_TAG;
  inheritance_send_result(&result);
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return true;
}

static bool decode_inheritance_encrypted_data(
    const uint8_t *data,
    uint16_t data_size,
    inheritance_decrypt_data_with_pin_encrypted_data_structure_t
        *encrypted_data) {
  if (NULL == data || NULL == encrypted_data) {
    SET_ERROR_TYPE(DECRYPTION_PB_DECODE_FAIL_ERROR);
    return false;
  }

  memzero(encrypted_data,
          sizeof(inheritance_decrypt_data_with_pin_encrypted_data_structure_t));

  pb_istream_t stream = pb_istream_from_buffer(data, data_size);

  bool status = pb_decode(
      &stream,
      INHERITANCE_DECRYPT_DATA_WITH_PIN_ENCRYPTED_DATA_STRUCTURE_FIELDS,
      encrypted_data);

  if (false == status) {
    SET_ERROR_TYPE(DECRYPTION_PB_DECODE_FAIL_ERROR);
  }

  return status;
}

// TODO: Make chunking logic generic to be used by any flow
static bool inheritance_get_encrypted_data(inheritance_query_t *query) {
  SET_FLOW_TAG(DECRYPTION_ENCRYPTED_DATA_GET_FLOW);
  uint8_t encoded_data[INHERITANCE_PACKET_MAX_SIZE] = {0};
  inheritance_result_t response =
      init_inheritance_result(INHERITANCE_RESULT_DECRYPT_TAG);
  const inheritance_decrypt_data_with_pin_encrypted_data_t *encrypted_data =
      &(query->decrypt.encrypted_data);
  const common_chunk_payload_t *payload = &(encrypted_data->chunk_payload);
  const common_chunk_payload_chunk_t *chunk = &(payload->chunk);
  uint32_t total_size = 0;
  uint32_t size = 0;
  while (1) {
    // req plain data chunk from host
    if (!inheritance_get_query(query, INHERITANCE_QUERY_DECRYPT_TAG)) {
      SET_ERROR_TYPE(DECRYPTION_QUERY_FETCH_FAIL_ERROR);
      return false;
    }
    if (!check_which_request(
            query,
            INHERITANCE_DECRYPT_DATA_WITH_PIN_REQUEST_ENCRYPTED_DATA_TAG)) {
      return false;
    }
    if (size == 0) {
      total_size = chunk->size + payload->remaining_size;
    }

    if (false == query->decrypt.encrypted_data.has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        size + chunk->size > total_size) {
      SET_ERROR_TYPE(DECRYPTION_CHUNK_DATA_INVALID_ERROR);
      return false;
    }

    memcpy(encoded_data + size, chunk->bytes, chunk->size);
    size += chunk->size;

    // Send chunk ack to host
    response.decrypt.which_response =
        INHERITANCE_DECRYPT_DATA_WITH_PIN_RESPONSE_DATA_ACCEPTED_TAG;
    response.decrypt.data_accepted.has_chunk_ack = true;
    response.decrypt.data_accepted.chunk_ack.chunk_index = payload->chunk_index;
    inheritance_send_result(&response);

    // If no data remaining to be received from the host, then exit
    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }
  if (!decode_inheritance_encrypted_data(
          encoded_data, total_size, &decryption_context->encrypted_data)) {
    return false;
  }

  return true;
}

static bool get_pb_encoded_buffer(
    const inheritance_decrypt_data_with_pin_decrypted_data_structure_t *result,
    uint8_t *buffer,
    uint16_t max_buffer_len,
    size_t *bytes_written_out) {
  if (NULL == result || NULL == buffer || NULL == bytes_written_out) {
    SET_ERROR_TYPE(DECRYPTION_PB_ENCODE_FAIL_ERROR);
    return false;
  }
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, max_buffer_len);

  bool status = pb_encode(
      &stream,
      INHERITANCE_DECRYPT_DATA_WITH_PIN_DECRYPTED_DATA_STRUCTURE_FIELDS,
      result);

  if (true == status) {
    *bytes_written_out = stream.bytes_written;
  } else {
    SET_ERROR_TYPE(DECRYPTION_PB_ENCODE_FAIL_ERROR);
  }

  return status;
}

// TODO: Make chunking logic generic to be used by any flow
static bool inheritance_send_in_chunks(inheritance_query_t *query,
                                       const uint8_t *buffer,
                                       const size_t buffer_len) {
  size_t total_count =
      ((buffer_len + DECRYPTED_CHUNK_SIZE - 1) / DECRYPTED_CHUNK_SIZE);
  size_t remaining_size = (size_t)buffer_len;
  size_t offset = 0;
  inheritance_result_t result =
      init_inheritance_result(INHERITANCE_RESULT_DECRYPT_TAG);
  result.decrypt.which_response =
      INHERITANCE_DECRYPT_DATA_WITH_PIN_RESPONSE_DECRYPTED_DATA_TAG;
  uint32_t *index = &result.decrypt.decrypted_data.chunk_payload.chunk_index;
  result.decrypt.decrypted_data.chunk_payload.total_chunks = total_count;

  *index = 0;
  do {
    if (!inheritance_get_query(query, INHERITANCE_QUERY_DECRYPT_TAG)) {
      SET_ERROR_TYPE(DECRYPTION_QUERY_FETCH_FAIL_ERROR);
      return false;
    }
    if (!check_which_request(
            query,
            INHERITANCE_DECRYPT_DATA_WITH_PIN_REQUEST_DECRYPTED_DATA_REQUEST_TAG)) {
      return false;
    }
    // chunk_payload validation checks
    if (query->decrypt.decrypted_data_request.has_chunk_ack == false ||
        query->decrypt.decrypted_data_request.chunk_ack.chunk_index != *index) {
      SET_ERROR_TYPE(DECRYPTION_CHUNK_DATA_INVALID_ERROR);
      return false;
    }
    size_t chunk_size = (remaining_size > DECRYPTED_CHUNK_SIZE)
                            ? DECRYPTED_CHUNK_SIZE
                            : remaining_size;
    remaining_size -= chunk_size;
    result.decrypt.decrypted_data.chunk_payload.remaining_size = remaining_size;
    result.decrypt.decrypted_data.has_chunk_payload = true;
    memcpy(result.decrypt.decrypted_data.chunk_payload.chunk.bytes,
           buffer + offset,
           chunk_size);
    result.decrypt.decrypted_data.chunk_payload.chunk.size = chunk_size;
    inheritance_send_result(&result);
    offset += chunk_size;
    if (remaining_size == 0) {
      break;
    }
    (*index)++;
  } while (*index < total_count);
  return true;
}

static bool send_decrypted_data(inheritance_query_t *query) {
  SET_FLOW_TAG(DECRYPTION_SEND_RESULT_FLOW);
  uint8_t
      buffer[INHERITANCE_DECRYPT_DATA_WITH_PIN_DECRYPTED_DATA_STRUCTURE_SIZE] =
          {0};
  size_t bytes_encoded = 0;
  if (!get_pb_encoded_buffer(&decryption_context->response_payload,
                             buffer,
                             sizeof(buffer),
                             &bytes_encoded) ||
      !inheritance_send_in_chunks(query, buffer, bytes_encoded)) {
    return false;
  }
  return true;
}

static bool decrypt_packet(void) {
  SET_FLOW_TAG(DECRYPTION_PACKET_DECRYPT_FLOW);
  bool was_decryption_successful =
      (session_aes_decrypt(decryption_context->encrypted_data.data.bytes,
                           &decryption_context->encrypted_data.data.size) ==
       SESSION_DECRYPTION_OK);
  if (!was_decryption_successful) {
    SET_ERROR_TYPE(DECRYPTION_SESSION_DECRYPTION_FAIL_ERROR);
  }

  return was_decryption_successful;
}

static bool deserialize_packet(void) {
  SET_FLOW_TAG(DECRYPTION_PACKET_DESERIALIZE_FLOW);
  uint16_t packet_index = 0;
  decryption_context->data_count =
      decryption_context->encrypted_data.data.bytes[packet_index++];
  for (uint8_t index = 0; index < decryption_context->data_count; index++) {
    decryption_context->response_payload.decrypted_data[index].tag =
        decryption_context->encrypted_data.data.bytes[packet_index];
    packet_index++;    ///< Tag

    decryption_context->data[index].encrypted_data_size = U16_READ_BE_ARRAY(
        &decryption_context->encrypted_data.data.bytes[packet_index]);
    packet_index += 2;    ///< Read length

    memcpy(decryption_context->data[index].encrypted_data,
           &decryption_context->encrypted_data.data.bytes[packet_index],
           decryption_context->data[index].encrypted_data_size);
    packet_index += decryption_context->data[index].encrypted_data_size;
  }
  bool status = packet_index <= decryption_context->encrypted_data.data.size;
  if (!status) {
    SET_ERROR_TYPE(DECRYPTION_INVALID_DATA_ERROR);
  }
  return status;
}

static bool decrypt_message_data(void) {
  SET_FLOW_TAG(DECRYPTION_MESSAGE_DECRYPT_FLOW);
  if (card_fetch_decrypt_data(decryption_context->wallet_id,
                              decryption_context->data,
                              decryption_context->data_count) !=
      CARD_OPERATION_SUCCESS) {
    SET_ERROR_TYPE(DECRYPTION_CARD_DECRYPTION_FAIL_ERROR);
    return false;
  }
  set_app_flow_status(INHERITANCE_DECRYPT_DATA_STATUS_MESSAGE_DECRYPTED);
  return true;
}

static bool decrypt_data(void) {
  bool status = true;

  do {
    if (!decrypt_packet()) {
      status = false;
      break;
    }

    if (!deserialize_packet()) {
      status = false;
      break;
    }
    if (!decrypt_message_data()) {
      status = false;
      break;
    }

  } while (0);
  delay_scr_init(ui_text_processing, DELAY_SHORT);
  return status;
}

static bool show_data(void) {
  SET_FLOW_TAG(DECRYPTION_USER_VERIFY_FLOW);
  pb_size_t response_count = 0;

  for (uint8_t i = 0; i < decryption_context->data_count; i++) {
    uint8_t tag = decryption_context->data[i].plain_data[0];

    if (tag == INHERITANCE_ONLY_SHOW_ON_DEVICE) {
      char msg[100] = {0};
      snprintf(msg,
               sizeof(msg),
               UI_TEXT_PIN,    ///< TODO: Make this generic
               &decryption_context->data[i].plain_data[3]);
      message_scr_init(msg);    ///< sizeof (tag) + sizeof (length) = 3
      // Do not care about the return value from confirmation screen
      (void)get_state_on_confirm_scr(0, 0, 0);
    } else {
      uint16_t offset = 1;    // Skip tag
      decryption_context->response_payload.decrypted_data[response_count]
          .message.size =
          U16_READ_BE_ARRAY(decryption_context->data[i].plain_data + offset);
      offset += 2;    // Skip length
      memcpy(decryption_context->response_payload.decrypted_data[response_count]
                 .message.bytes,
             decryption_context->data[i].plain_data + offset,
             decryption_context->response_payload.decrypted_data[response_count]
                 .message.size);
      decryption_context->response_payload.decrypted_data_count =
          ++response_count;
    }
  }

  set_app_flow_status(INHERITANCE_DECRYPT_DATA_STATUS_PIN_VERIFIED);
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

decryption_error_type_e inheritance_decrypt_data(inheritance_query_t *query) {
  decryption_set_defaults();
  decryption_context = (inheritance_decryption_context_t *)malloc(
      sizeof(inheritance_decryption_context_t));
  if (decryption_context == NULL) {
    SET_ERROR_TYPE(DECRYPTION_ASSERT_MALLOC_ERROR);
    decryption_handle_errors();
    ASSERT(decryption_context != NULL);
  }
  memzero(decryption_context, sizeof(inheritance_decryption_context_t));

  if (inheritance_decryption_handle_initiate_query(query) &&
      inheritance_get_encrypted_data(query) && decrypt_data() && show_data() &&
      send_decrypted_data(query)) {
    delay_scr_init(ui_text_inheritance_decryption_flow_success, DELAY_SHORT);
    SET_ERROR_TYPE(DECRYPTION_OK);
  } else {
    delay_scr_init(ui_text_inheritance_decryption_flow_failure, DELAY_SHORT);
  }
  decryption_handle_errors();

  delay_scr_init(ui_text_check_cysync, DELAY_TIME);

  memzero(decryption_context, sizeof(inheritance_decryption_context_t));
  free(decryption_context);
  decryption_context = NULL;
  return decryption_error.type;
}
