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

#include "bignum.h"
#include "card_fetch_data.h"
#include "constant_texts.h"
#include "core_session.h"
#include "inheritance/common.pb.h"
#include "inheritance/core.pb.h"
#include "inheritance/decrypt_data_with_pin.pb.h"
#include "inheritance/encrypt_data_with_pin.pb.h"
#include "inheritance_api.h"
#include "inheritance_context.h"
#include "inheritance_priv.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "status_api.h"
#include "ui_core_confirm.h"
#include "ui_delay.h"
#include "ui_input_text.h"
#include "utils.h"
#include "verify_pin_flow.h"
#include "wallet.h"
#include "wallet_list.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define ENCRYPTED_CHUNK_SIZE (2048)
/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Checks the type of request in the inheritance query.
 *
 * This function verifies whether the specified request type matches the one
 * contained in the query. If not, it sends an error message and returns false.
 *
 * @param query Pointer to the inheritance query.
 * @param which_request The expected request type.
 * @return True if the request type matches; false otherwise.
 */
static bool check_which_request(const inheritance_query_t *query,
                                pb_size_t which_request);

/**
 * @brief Validates the request data for encrypting data with a PIN.
 *
 * This function ensures that the provided request data is valid. It checks if
 * the wallet associated with the request has a set PIN. If not, it sends an
 * error message and returns false.
 *
 * @param request Pointer to the encrypt data request.
 * @return True if the request data is valid; false otherwise.
 */
static bool validate_request_data(
    const inheritance_encrypt_data_with_pin_request_t *request);

/**
 * @brief Handles the initiate query for encrypting data with a PIN.
 *
 * This function processes the initiate query, including verifying the request
 * type, validating the request data, and obtaining the wallet name. It prompts
 * the user for confirmation and sets the application flow status accordingly.
 *
 * @param query Pointer to the inheritance query.
 * @return True if the query is successfully handled; false otherwise.
 */
STATIC bool inheritance_encryption_handle_inititate_query(
    inheritance_query_t *query);

/**
 * @brief Function responsible for decoding pb_encoded buffer to @ref
 * inheritance_encrypt_data_with_pin_plain_data_structure_t
 *
 * @return true if decoding successful, false otherwise.
 */
static bool decode_inheritance_plain_data(
    const uint8_t *data,
    uint16_t data_size,
    inheritance_encrypt_data_with_pin_plain_data_structure_t *plain_data);

/**
 * @brief Retrieves pb_encoded plain data chunks from the host and creates input
 * buffer for @ref decode_inheritance_plain_data.
 *
 * @return true if data is successfully retrieved and decoded, false otherwise.
 */
static bool inheritance_get_plain_data(inheritance_query_t *query);

/**
 * @brief Obtains user verification for specific plain data messages.
 *
 * This function prompts the user to verify specific plain data messages. If the
 * messages are verified, it updates the application flow status accordingly.
 *
 * @return True if user verification is successful; false otherwise.
 */
STATIC bool inheritance_encryption_get_user_verification(void);

/**
 * @brief Verifies the PIN associated with the request.
 *
 * This function verifies the PIN for the specified wallet ID. It communicates
 * with the PIN verification mechanism and returns the verification status.
 *
 * @return True if the PIN is verified; false otherwise.
 */
static bool inheritance_verify_pin(void);

/**
 * @brief Fills a TLV (Tag-Length-Value) structure.
 *
 * This function constructs a TLV structure by populating the destination buffer
 * with the specified tag, length, and value. It updates the starting index
 * accordingly.
 *
 * @param destination Pointer to the destination buffer.
 * @param starting_index Pointer to the starting index within the buffer.
 * @param tag The TLV tag.
 * @param length The length of the value.
 * @param value Pointer to the value data.
 */
static void inheritance_fill_tlv(uint8_t *destination,
                                 uint16_t *starting_index,
                                 uint8_t tag,
                                 uint16_t length,
                                 const uint8_t *value);

/**
 * @brief Serializes the message data for encryption.
 *
 * This function prepares the plain data messages for encryption. It constructs
 * TLV (Tag-Length-Value) structures for each message, including a special tag
 * for the PIN. The resulting data is stored in the context data array.
 *
 * @return True if serialization is successful; false otherwise.
 */
static bool serialize_message_data(void);

/**
 * @brief Encrypts the prepared message data.
 *
 * This function fetches and encrypts the data associated with the wallet ID.
 * If successful, it updates the application flow status accordingly.
 *
 * @return True if encryption is successful; false otherwise.
 */
static bool encrypt_message_data(void);

/**
 * @brief Serializes the encrypted data into a packet.
 *
 * This function constructs a packet containing the serialized TLV structures
 * for the encrypted data messages. The last entry in the packet corresponds
 * to the encrypted PIN.
 *
 * @return True if serialization is successful; false otherwise.
 */
static bool serialize_packet(void);

/**
 * @brief Encrypts the entire packet.
 *
 * This function performs AES encryption on the packet data. If successful,
 * it returns true; otherwise, false.
 *
 * @return True if packet encryption is successful; false otherwise.
 */
static bool encrypt_packet(void);

/**
 * @brief Creates a pb encoded buffer of @ref
 * inheritance_encrypt_data_with_pin_encrypted_data_structure_t to be sent to
 * host.
 *
 * @return true if encoding is successful, false otherwise.
 */
static bool get_pb_encoded_buffer(
    const inheritance_encrypt_data_with_pin_encrypted_data_structure_t *result,
    uint8_t *buffer,
    uint16_t max_buffer_len,
    size_t *bytes_written_out);

/**
 * @brief Sends input buffer to host in chunks.
 *
 * @return true if chunking successful, false otherwise.
 */
static bool inheritance_send_in_chunks(inheritance_query_t *query,
                                       const uint8_t *buffer,
                                       const size_t buffer_len);
/**
 * @brief Encrypts the data and sends the result.
 *
 * This function orchestrates the entire process of verifying the PIN,
 * serializing and encrypting the data, and sending the result back to the user.
 *
 * @param query Pointer to the inheritance query.
 * @return True if the process completes successfully; false otherwise.
 */
static bool send_encrypted_data(inheritance_query_t *query);

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/

STATIC inheritance_encryption_context_t *encryption_context = NULL;

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

    // TODO: check the current session validity here

  } while (0);

  return status;
}

STATIC bool inheritance_encryption_handle_inititate_query(
    inheritance_query_t *query) {
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
           ui_text_inheritance_encryption_flow_confirmation,
           wallet_name);

  if (!core_confirmation(msg, inheritance_send_error)) {
    return false;
  }

  set_app_flow_status(INHERITANCE_ENCRYPT_DATA_STATUS_USER_CONFIRMED);

  memcpy(encryption_context->wallet_id,
         query->encrypt.initiate.wallet_id,
         WALLET_ID_SIZE);
  inheritance_result_t result =
      init_inheritance_result(INHERITANCE_RESULT_ENCRYPT_TAG);
  result.encrypt.which_response =
      INHERITANCE_ENCRYPT_DATA_WITH_PIN_RESPONSE_CONFIRMATION_TAG;
  inheritance_send_result(&result);
  return true;
}

static bool decode_inheritance_plain_data(
    const uint8_t *data,
    uint16_t data_size,
    inheritance_encrypt_data_with_pin_plain_data_structure_t *plain_data) {
  if (NULL == data || NULL == plain_data || 0 == data_size) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_DECODING_FAILED);
    return false;
  }

  memzero(plain_data,
          sizeof(inheritance_encrypt_data_with_pin_plain_data_structure_t));

  pb_istream_t stream = pb_istream_from_buffer(data, data_size);

  bool status =
      pb_decode(&stream,
                INHERITANCE_ENCRYPT_DATA_WITH_PIN_PLAIN_DATA_STRUCTURE_FIELDS,
                plain_data);

  if (false == status) {
    inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                           ERROR_DATA_FLOW_DECODING_FAILED);
  }

  return status;
}

static bool inheritance_get_plain_data(inheritance_query_t *query) {
  uint8_t encoded_data[INHERITANCE_PACKET_MAX_SIZE] = {0};
  inheritance_result_t response =
      init_inheritance_result(INHERITANCE_RESULT_ENCRYPT_TAG);
  const inheritance_encrypt_data_with_pin_plain_data_t *plain_data =
      &(query->encrypt.plain_data);
  const common_chunk_payload_t *payload = &(plain_data->chunk_payload);
  const common_chunk_payload_chunk_t *chunk = &(payload->chunk);
  uint32_t total_size = 0;
  uint32_t size = 0;
  while (1) {
    // req plain data chunk from host
    if (!inheritance_get_query(query, INHERITANCE_QUERY_ENCRYPT_TAG) ||
        !check_which_request(
            query, INHERITANCE_ENCRYPT_DATA_WITH_PIN_REQUEST_PLAIN_DATA_TAG)) {
      return false;
    }
    if (size == 0) {
      total_size = chunk->size + payload->remaining_size;
    }

    if (false == query->encrypt.plain_data.has_chunk_payload ||
        payload->chunk_index >= payload->total_chunks ||
        size + chunk->size > total_size) {
      inheritance_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                             ERROR_DATA_FLOW_INVALID_DATA);
      return false;
    }

    memcpy(encoded_data + size, chunk->bytes, chunk->size);
    size += chunk->size;

    // Send chunk ack to host
    response.encrypt.which_response =
        INHERITANCE_ENCRYPT_DATA_WITH_PIN_RESPONSE_DATA_ACCEPTED_TAG;
    response.encrypt.data_accepted.has_chunk_ack = true;
    response.encrypt.data_accepted.chunk_ack.chunk_index = payload->chunk_index;
    inheritance_send_result(&response);

    // If no data remaining to be received from the host, then exit
    if (0 == payload->remaining_size ||
        payload->chunk_index + 1 == payload->total_chunks) {
      break;
    }
  }
  if (!decode_inheritance_plain_data(
          encoded_data, total_size, &encryption_context->plain_data)) {
    return false;
  }

  return true;
}

STATIC bool inheritance_encryption_get_user_verification(void) {
  for (int i = 0; i < encryption_context->plain_data.data_count; i++) {
    const inheritance_plain_data_t *data =
        &encryption_context->plain_data.data[i];

    if (data->has_is_verified_on_device && data->is_verified_on_device) {
      if (!core_scroll_non_sticky_heading_page(
              UI_TEXT_VERIFY_MESSAGE,
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
  return verify_pin(encryption_context->wallet_id,
                    encryption_context->pin_value,
                    inheritance_send_error);
}

static void inheritance_fill_tlv(uint8_t *destination,
                                 uint16_t *starting_index,
                                 uint8_t tag,
                                 uint16_t length,
                                 const uint8_t *value) {
  destination[(*starting_index)++] = tag;
  destination[(*starting_index)++] = (length >> 8);
  destination[(*starting_index)++] = length;

  memcpy(destination + *starting_index, value, length);
  *starting_index = *starting_index + length;
}

static bool serialize_message_data(void) {
  if (encryption_context->plain_data.data_count >=
      INHERITANCE_MESSAGES_MAX_COUNT) {
    // TODO: Throw invalid message count error;
    return false;
  }
  pb_size_t index = 0;
  uint16_t written_length = 0;
  for (index = 0; index < encryption_context->plain_data.data_count; index++) {
    uint16_t length = encryption_context->plain_data.data[index].message.size;
    if (length > (PLAIN_DATA_SIZE - 3)) {
      length = PLAIN_DATA_SIZE - 3;
    }
    written_length = 0;
    inheritance_fill_tlv(
        encryption_context->data[index].plain_data,
        &written_length,
        INHERITANCE_DEFAULT_MESSAGE,
        length,
        encryption_context->plain_data.data[index].message.bytes);
    encryption_context->data[index].plain_data_size = written_length;
  }
  written_length = 0;
  inheritance_fill_tlv(encryption_context->data[index].plain_data,
                       &written_length,
                       INHERITANCE_ONLY_SHOW_ON_DEVICE,
                       MAX_PIN_SIZE,
                       encryption_context->pin_value);
  encryption_context->data[index].plain_data_size = written_length;

  encryption_context->data_count =
      encryption_context->plain_data.data_count + 1;
  memzero(encryption_context->pin_value, sizeof(encryption_context->pin_value));
  return true;
}

static bool encrypt_message_data(void) {
  card_error_type_e status =
      card_fetch_encrypt_data(encryption_context->wallet_id,
                              encryption_context->data,
                              encryption_context->data_count);

  if (status != CARD_OPERATION_SUCCESS) {
    // TODO: throw encryption failed error
    return false;
  }
  return true;
}

static bool serialize_packet(void) {
  encryption_context->payload.encrypted_data.size = 0;
  encryption_context->payload.encrypted_data
      .bytes[encryption_context->payload.encrypted_data.size++] =
      encryption_context->data_count;
  pb_size_t index = 0;

  for (index = 0; index < encryption_context->data_count - 1; index++) {
    inheritance_fill_tlv(encryption_context->payload.encrypted_data.bytes,
                         &encryption_context->payload.encrypted_data.size,
                         0x00,    ///< TODO: take this from sdk
                         encryption_context->data[index].encrypted_data_size,
                         encryption_context->data[index].encrypted_data);
  }

  // The last encrypted message is the PIN
  inheritance_fill_tlv(encryption_context->payload.encrypted_data.bytes,
                       &encryption_context->payload.encrypted_data.size,
                       INHERITANCE_PIN_TAG,
                       encryption_context->data[index].encrypted_data_size,
                       encryption_context->data[index].encrypted_data);

  return true;
}

static bool encrypt_packet(void) {
  if (SESSION_ENCRYPT_PACKET_SUCCESS !=
      session_aes_encrypt(encryption_context->payload.encrypted_data.bytes,
                          &encryption_context->payload.encrypted_data.size)) {
    return false;
  }

  return true;
}

static bool encrypt_data(void) {
  bool status = true;

  do {
    if (!inheritance_verify_pin()) {
      // TODO: Throw user rejceted
      status = false;
      break;
    }
    if (!serialize_message_data()) {
      // TODO: Throw serialization failed
      status = false;
      break;
    }

    if (!encrypt_message_data()) {
      // TODO: Throw encryption failed
      status = false;
      break;
    }

    if (!serialize_packet()) {
      // TODO: Throw packet serialization error
      status = false;
      break;
    }

    if (!encrypt_packet()) {
      // TODO: Throw packet encryption error
      status = false;
      break;
    }
  } while (0);

  return status;
}

static bool get_pb_encoded_buffer(
    const inheritance_encrypt_data_with_pin_encrypted_data_structure_t *result,
    uint8_t *buffer,
    uint16_t max_buffer_len,
    size_t *bytes_written_out) {
  if (NULL == result || NULL == buffer || NULL == bytes_written_out) {
    return false;
  }
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, max_buffer_len);

  bool status = pb_encode(
      &stream,
      INHERITANCE_ENCRYPT_DATA_WITH_PIN_ENCRYPTED_DATA_STRUCTURE_FIELDS,
      result);

  if (true == status) {
    *bytes_written_out = stream.bytes_written;
  }

  return status;
}

static bool inheritance_send_in_chunks(inheritance_query_t *query,
                                       const uint8_t *buffer,
                                       const size_t buffer_len) {
  size_t total_count = ((buffer_len + 1) / ENCRYPTED_CHUNK_SIZE);
  size_t remaining_size = (size_t)buffer_len;
  size_t offset = 0;
  inheritance_result_t result =
      init_inheritance_result(INHERITANCE_RESULT_ENCRYPT_TAG);
  result.encrypt.which_response =
      INHERITANCE_ENCRYPT_DATA_WITH_PIN_RESPONSE_ENCRYPTED_DATA_TAG;
  uint32_t *index = &result.encrypt.encrypted_data.chunk_payload.chunk_index;
  result.encrypt.encrypted_data.chunk_payload.total_chunks = total_count;

  for (*index = 0; *index < total_count; (*index)++) {
    if (!inheritance_get_query(query, INHERITANCE_QUERY_ENCRYPT_TAG) ||
        !check_which_request(
            query,
            INHERITANCE_ENCRYPT_DATA_WITH_PIN_REQUEST_ENCRYPTED_DATA_REQUEST_TAG)) {
      return false;
    }
    // chunk_payload validation checks
    if (query->encrypt.encrypted_data_request.has_chunk_ack == false ||
        query->encrypt.encrypted_data_request.chunk_ack.chunk_index != *index) {
      return false;
    }
    size_t chunk_size = (remaining_size > ENCRYPTED_CHUNK_SIZE)
                            ? ENCRYPTED_CHUNK_SIZE
                            : remaining_size;
    remaining_size -= chunk_size;
    result.encrypt.encrypted_data.chunk_payload.remaining_size = remaining_size;
    result.encrypt.encrypted_data.has_chunk_payload = true;
    memcpy(result.encrypt.encrypted_data.chunk_payload.chunk.bytes,
           buffer + offset,
           chunk_size);
    result.encrypt.encrypted_data.chunk_payload.chunk.size = chunk_size;
    inheritance_send_result(&result);
    offset += chunk_size;
    if (remaining_size == 0) {
      break;
    }
  }
  return true;
}

static bool send_encrypted_data(inheritance_query_t *query) {
  uint8_t
      buffer[INHERITANCE_ENCRYPT_DATA_WITH_PIN_ENCRYPTED_DATA_STRUCTURE_SIZE] =
          {0};
  size_t bytes_encoded = 0;
  if (!get_pb_encoded_buffer(&encryption_context->payload,
                             buffer,
                             sizeof(buffer),
                             &bytes_encoded) ||
      !inheritance_send_in_chunks(query, buffer, bytes_encoded)) {
    return false;
  }
  return true;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/

void inheritance_encrypt_data(inheritance_query_t *query) {
  encryption_context = (inheritance_encryption_context_t *)malloc(
      sizeof(inheritance_encryption_context_t));
  ASSERT(encryption_context != NULL);
  memzero(encryption_context, sizeof(inheritance_encryption_context_t));

  if (inheritance_encryption_handle_inititate_query(query) &&
      inheritance_get_plain_data(query) &&
      inheritance_encryption_get_user_verification() && encrypt_data() &&
      send_encrypted_data(query)) {
    delay_scr_init(ui_text_inheritance_encryption_flow_success, DELAY_TIME);
  } else {
    delay_scr_init(ui_text_inheritance_encryption_flow_failure, DELAY_TIME);
  }
  delay_scr_init(ui_text_check_cysync, DELAY_TIME);

  memzero(encryption_context, sizeof(inheritance_encryption_context_t));
  free(encryption_context);
  encryption_context = NULL;
}
