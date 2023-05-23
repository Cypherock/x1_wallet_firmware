/**
 * @file    manager_api.c
 * @author  Cypherock X1 Team
 * @brief   Defines helpers apis for manager app.
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
#include "manager_api.h"

#include "pb_decode.h"
#include "pb_encode.h"
#include "usb_api.h"

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
bool decode_manager_query(const uint8_t *data,
                          uint16_t data_size,
                          manager_query_t *query_out) {
  if (NULL == data || NULL == query_out || 0 == data_size)
    return false;

  /* Initialize manager query */
  manager_query_t query = MANAGER_QUERY_INIT_ZERO;

  /* Create a stream that reads from the buffer. */
  pb_istream_t stream = pb_istream_from_buffer(data, data_size);

  /* Now we are ready to decode the message. */
  bool status = pb_decode(&stream, MANAGER_QUERY_FIELDS, &query);

  /* Copy query obj if status is true*/
  if (true == status) {
    memcpy(query_out, &query, sizeof(query));
  }

  return status;
}

bool encode_manager_result(manager_result_t *result,
                           uint8_t *buffer,
                           uint16_t max_buffer_len,
                           size_t *bytes_written_out) {
  if (NULL == result || NULL == buffer || NULL == bytes_written_out)
    return false;

  /* Create a stream that will write to our buffer. */
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, max_buffer_len);

  /* Now we are ready to encode the message! */
  bool status = pb_encode(&stream, MANAGER_RESULT_FIELDS, result);

  if (true == status) {
    *bytes_written_out = stream.bytes_written;
  }

  return status;
}

bool check_manager_request(const manager_query_t *query,
                           const pb_size_t exp_query_tag) {
  if ((NULL == query) || (exp_query_tag != query->which_request)) {
    return false;
  }
  return true;
}

manager_result_t get_manager_result_template(const pb_size_t result_tag) {
  manager_result_t result = MANAGER_RESULT_INIT_ZERO;
  result.which_response = result_tag;
  return result;
}

bool encode_and_send_manager_result(manager_result_t *result,
                                    uint8_t *out_buffer,
                                    const size_t size_of_buffer) {
  size_t bytes_encoded = 0;
  if (true == encode_manager_result(
                  result, &out_buffer[0], size_of_buffer, &bytes_encoded)) {
    usb_send_msg(&out_buffer[0], bytes_encoded);
    return true;
  }
  return false;
}