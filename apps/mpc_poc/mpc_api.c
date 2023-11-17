/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "mpc_api.h"

#include <pb_decode.h>
#include <pb_encode.h>

#include "common_error.h"
#include "core_api.h"
#include "events.h"

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
 * STATIC VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
bool decode_mpc_query(const uint8_t *data,
                         uint16_t data_size,
                         mpc_poc_query_t *query_out) {
  if (NULL == data || NULL == query_out || 0 == data_size) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_DECODING_FAILED);
    return false;
  }

  // zeroise for safety from garbage in the query reference
  memzero(query_out, sizeof(mpc_poc_query_t));

  /* Create a stream that reads from the buffer. */
  pb_istream_t stream = pb_istream_from_buffer(data, data_size);

  /* Now we are ready to decode the message. */
  bool status = pb_decode(&stream, MPC_POC_QUERY_FIELDS, query_out);

  /* Send error to host if status is false*/
  if (false == status) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_DECODING_FAILED);
  }

  return status;
}

bool encode_mpc_result(const mpc_poc_result_t *result,
                          uint8_t *buffer,
                          uint16_t max_buffer_len,
                          size_t *bytes_written_out) {
  if (NULL == result || NULL == buffer || NULL == bytes_written_out)
    return false;

  /* Create a stream that will write to our buffer. */
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, max_buffer_len);

  /* Now we are ready to encode the message! */
  bool status = pb_encode(&stream, MPC_POC_RESULT_FIELDS, result);

  if (true == status) {
    *bytes_written_out = stream.bytes_written;
  }

  return status;
}

bool check_mpc_query(const mpc_poc_query_t *query, pb_size_t exp_query_tag) {
  if ((NULL == query) || (exp_query_tag != query->which_request)) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_QUERY);
    return false;
  }
  return true;
}

mpc_poc_result_t init_mpc_result(pb_size_t result_tag) {
  mpc_poc_result_t result = MPC_POC_RESULT_INIT_ZERO;
  result.which_response = result_tag;
  return result;
}

void mpc_send_error(pb_size_t which_error, uint32_t error_code) {
  mpc_poc_result_t result = init_mpc_result(MPC_POC_RESULT_COMMON_ERROR_TAG);
  result.common_error = init_common_error(which_error, error_code);
  mpc_send_result(&result);
}

void mpc_send_result(const mpc_poc_result_t *result) {
  // TODO: Set the options file for all
  uint8_t buffer[1700] = {0};
  size_t bytes_encoded = 0;
  ASSERT(encode_mpc_result(result, buffer, sizeof(buffer), &bytes_encoded));
  send_response_to_host(&buffer[0], bytes_encoded);
}

bool mpc_get_query(mpc_poc_query_t *query, pb_size_t exp_query_tag) {
  evt_status_t event = get_events(EVENT_CONFIG_USB, MAX_INACTIVITY_TIMEOUT);

  if (true == event.p0_event.flag) {
    return false;
  }

  if (!decode_mpc_query(
          event.usb_event.p_msg, event.usb_event.msg_size, query)) {
    return false;
  }

  if (!check_mpc_query(query, exp_query_tag)) {
    return false;
  }

  return true;
}
