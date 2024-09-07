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
#include "utils.h"
#include "wallet.h"
#include "wallet_list.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define DECRYPTED_CHUNK_SIZE (2048)
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
  delay_scr_init(ui_text_processing, DELAY_TIME);
  return true;
}

static bool decode_inheritance_encrypted_data(
    const uint8_t *data,
    uint16_t data_size,
    inheritance_decrypt_data_with_pin_encrypted_data_structure_t
        *encrypted_data) {
  if (NULL == data || NULL == encrypted_data || 0 == data_size) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_DECODING_FAILED);
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
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_DECODING_FAILED);
  }

  return status;
}

static bool inheritance_get_encrypted_data(inheritance_query_t *query) {
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
    if (!inheritance_get_query(query, INHERITANCE_QUERY_DECRYPT_TAG) ||
        !check_which_request(
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
      inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_DATA);
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
    return false;
  }
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, max_buffer_len);

  bool status = pb_encode(
      &stream,
      INHERITANCE_DECRYPT_DATA_WITH_PIN_DECRYPTED_DATA_STRUCTURE_FIELDS,
      result);

  if (true == status) {
    *bytes_written_out = stream.bytes_written;
  }

  return status;
}

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

  for (*index = 0; *index < total_count; (*index)++) {
    if (!inheritance_get_query(query, INHERITANCE_QUERY_DECRYPT_TAG) ||
        !check_which_request(
            query,
            INHERITANCE_DECRYPT_DATA_WITH_PIN_REQUEST_DECRYPTED_DATA_REQUEST_TAG)) {
      return false;
    }
    // chunk_payload validation checks
    if (query->decrypt.decrypted_data_request.has_chunk_ack == false ||
        query->decrypt.decrypted_data_request.chunk_ack.chunk_index != *index) {
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
  }
  return true;
}

static bool send_decrypted_data(inheritance_query_t *query) {
  uint8_t
      buffer[INHERITANCE_DECRYPT_DATA_WITH_PIN_DECRYPTED_DATA_STRUCTURE_SIZE] =
          {0};
  size_t bytes_encoded = 0;
  if (!get_pb_encoded_buffer(&decryption_context->response_payload,
                             buffer,
                             sizeof(buffer),
                             &bytes_encoded) ||
      !inheritance_send_in_chunks(query, buffer, bytes_encoded)) {
    // TODO: throw decryption failed error
    return false;
  }
  return true;
}

static bool decrypt_packet(void) {
  return session_aes_decrypt(decryption_context->encrypted_data.data.bytes,
                             &decryption_context->encrypted_data.data.size) ==
         SESSION_DECRYPT_PACKET_SUCCESS;
}

static bool deserialize_packet(void) {
  uint16_t packet_index = 0;
  decryption_context->data_count =
      decryption_context->encrypted_data.data.bytes[packet_index++];
  for (uint8_t index = 0; index < decryption_context->data_count; index++) {
    packet_index++;    ///< Skip tag

    decryption_context->data[index].encrypted_data_size = U16_READ_BE_ARRAY(
        &decryption_context->encrypted_data.data.bytes[packet_index]);
    packet_index += 2;    ///< Read length

    memcpy(decryption_context->data[index].encrypted_data,
           &decryption_context->encrypted_data.data.bytes[packet_index],
           decryption_context->data[index].encrypted_data_size);
    packet_index += decryption_context->data[index].encrypted_data_size;
  }

  return packet_index <= decryption_context->encrypted_data.data.size;
}

static bool decrypt_message_data(void) {
  return card_fetch_decrypt_data(decryption_context->wallet_id,
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
  delay_scr_init(ui_text_processing, DELAY_TIME);
  return status;
}

static bool show_data(void) {
  pb_size_t response_count = 0;

  for (uint8_t i = 0; i < decryption_context->data_count; i++) {
    uint8_t tag = decryption_context->data[i].plain_data[0];

    if (tag == INHERITANCE_ONLY_SHOW_ON_DEVICE) {
      if (!core_scroll_page(
              UI_TEXT_PIN,    ///< TODO: Figure out a way to make this generic
              (const char *)&decryption_context->data[i]
                  .plain_data[3],    ///< sizeof (tag) + sizeof (length) = 3
              inheritance_send_error)) {
        return false;
      }
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

void inheritance_decrypt_data(inheritance_query_t *query) {
  decryption_context = (inheritance_decryption_context_t *)malloc(
      sizeof(inheritance_decryption_context_t));
  memzero(decryption_context, sizeof(inheritance_decryption_context_t));

  if (inheritance_handle_initiate_query(query) &&
      inheritance_get_encrypted_data(query) && decrypt_data() && show_data() &&
      send_decrypted_data(query)) {
    delay_scr_init(ui_text_inheritance_decryption_flow_success, DELAY_TIME);
  } else {
    delay_scr_init(ui_text_inheritance_decryption_flow_failure, DELAY_TIME);
  }
  delay_scr_init(ui_text_check_cysync, DELAY_TIME);

  memzero(decryption_context, sizeof(inheritance_decryption_context_t));
  free(decryption_context);
  decryption_context = NULL;
}
