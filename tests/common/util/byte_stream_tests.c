/**
 * @author  Cypherock X1 Team
 * @brief
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2024 by HODL TECH PTE LTD
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
#include <stdlib.h>
#include <string.h>

#include "byte_stream.h"
#include "unity.h"
#include "unity_fixture.h"
#include "utils.h"

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
static uint8_t *test_data = NULL;
static uint8_t *large_test_data = NULL;

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/
static byte_stream_writer_status_e sequential_error_writer(
    byte_stream_t *stream);
static byte_stream_writer_status_e stream_modifier_writer(
    byte_stream_t *stream);
static byte_stream_writer_status_e incremental_writer(byte_stream_t *stream);
static byte_stream_writer_status_e dummy_request_write_cb(
    byte_stream_t *stream);

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static byte_stream_writer_status_e sequential_error_writer(
    byte_stream_t *stream) {
  static int call_count = 0;
  call_count++;

  switch (call_count) {
    case 1:
      return BYTE_STREAM_WRITER_UNKNOWN_ERROR;
    case 2:
      stream->offset = -1;
      return BYTE_STREAM_WRITER_SUCCESS;
    case 3:
      stream->stream_pointer = NULL;
      return BYTE_STREAM_WRITER_SUCCESS;
    default:
      call_count = 0;
      return BYTE_STREAM_WRITER_SUCCESS;
  }
}

static byte_stream_writer_status_e stream_modifier_writer(
    byte_stream_t *stream) {
  // Simulates a writer that updates the stream with new data
  stream->offset = 0;
  stream->stream_pointer += stream->capacity;
  return BYTE_STREAM_WRITER_SUCCESS;
}

static byte_stream_writer_status_e incremental_writer(byte_stream_t *stream) {
  // Simulates a writer that provides data in small chunks
  stream->capacity = 10;
  stream->stream_pointer += stream->capacity;
  stream->offset = 0;
  return BYTE_STREAM_WRITER_SUCCESS;
}

static byte_stream_writer_status_e dummy_request_write_cb(
    byte_stream_t *stream) {
  // Simple dummy writer that just returns success
  return BYTE_STREAM_WRITER_SUCCESS;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
TEST_GROUP(byte_stream_tests);

TEST_SETUP(byte_stream_tests) {
  test_data = malloc(200);
  large_test_data = malloc(1000);

  // Initialize test data with pattern
  for (int i = 0; i < 200; i++) {
    test_data[i] = i & 0xFF;
  }

  for (int i = 0; i < 1000; i++) {
    large_test_data[i] = (i * 7) & 0xFF;
  }
}

TEST_TEAR_DOWN(byte_stream_tests) {
  free(test_data);
  free(large_test_data);
}

TEST(byte_stream_tests, returns_error_for_null_stream) {
  uint8_t read_buffer[10] = {0};
  byte_stream_status_e status = read_byte_stream(NULL, read_buffer, 10);
  TEST_ASSERT_EQUAL(status, BYTE_STREAM_INVALID_STREAM);
}

TEST(byte_stream_tests, returns_error_for_null_destination) {
  byte_stream_t stream = {.offset = 0,
                          .capacity = 100,
                          .stream_pointer = test_data,
                          .writer = dummy_request_write_cb};
  byte_stream_status_e status = read_byte_stream(&stream, NULL, 10);
  TEST_ASSERT_EQUAL(status, BYTE_STREAM_INVALID_DESTINATION);
}

TEST(byte_stream_tests, returns_error_for_null_stream_pointer) {
  byte_stream_t stream = {.offset = 0,
                          .capacity = 100,
                          .stream_pointer = NULL,
                          .writer = dummy_request_write_cb};
  uint8_t read_buffer[10] = {0};
  byte_stream_status_e status = read_byte_stream(&stream, read_buffer, 10);
  TEST_ASSERT_EQUAL(status, BYTE_STREAM_INVALID_STREAM);
}

TEST(byte_stream_tests, returns_error_for_null_writer_callback) {
  byte_stream_t stream = {.offset = 0,
                          .capacity = 100,
                          .stream_pointer = test_data,
                          .writer = NULL};
  uint8_t read_buffer[10] = {0};
  byte_stream_status_e status = read_byte_stream(&stream, read_buffer, 10);
  TEST_ASSERT_EQUAL(status, BYTE_STREAM_INVALID_STREAM);
}

TEST(byte_stream_tests, returns_error_for_negative_offset) {
  byte_stream_t stream = {.offset = -1,
                          .capacity = 100,
                          .stream_pointer = test_data,
                          .writer = dummy_request_write_cb};
  uint8_t read_buffer[10] = {0};
  byte_stream_status_e status = read_byte_stream(&stream, read_buffer, 10);
  TEST_ASSERT_EQUAL(status, BYTE_STREAM_INVALID_STREAM);
}

TEST(byte_stream_tests, returns_error_for_offset_greater_than_capacity) {
  byte_stream_t stream = {.offset = 150,
                          .capacity = 100,
                          .stream_pointer = test_data,
                          .writer = dummy_request_write_cb};
  uint8_t read_buffer[10] = {0};
  byte_stream_status_e status = read_byte_stream(&stream, read_buffer, 10);
  TEST_ASSERT_EQUAL(status, BYTE_STREAM_INVALID_STREAM);
}

TEST(byte_stream_tests, returns_error_when_writer_fails) {
  byte_stream_writer_status_e failing_writer(byte_stream_t * stream) {
    return BYTE_STREAM_WRITER_UNKNOWN_ERROR;
  }

  byte_stream_t stream = {.offset = 0,
                          .capacity = 10,
                          .stream_pointer = test_data,
                          .writer = failing_writer};
  uint8_t read_buffer[20] = {0};
  byte_stream_status_e status = read_byte_stream(&stream, read_buffer, 20);
  TEST_ASSERT_EQUAL(status, BYTE_STREAM_FAILED_WRITER);
}

TEST(byte_stream_tests, returns_error_for_zero_capacity) {
  byte_stream_t stream = {.offset = 0,
                          .capacity = 0,
                          .stream_pointer = test_data,
                          .writer = dummy_request_write_cb};
  uint8_t read_buffer[10] = {0};
  byte_stream_status_e status = read_byte_stream(&stream, read_buffer, 10);
  TEST_ASSERT_EQUAL(status, BYTE_STREAM_INVALID_STREAM);
}

TEST(byte_stream_tests, can_handle_large_reads_with_multiple_writer_calls) {
  byte_stream_t stream = {.offset = 0,
                          .capacity = 100,
                          .stream_pointer = large_test_data,
                          .writer = stream_modifier_writer};

  uint8_t read_buffer[500] = {0};
  byte_stream_status_e status = read_byte_stream(&stream, read_buffer, 500);

  TEST_ASSERT_EQUAL(BYTE_STREAM_SUCCESS, status);
  TEST_ASSERT_EQUAL_MEMORY(large_test_data, read_buffer, 500);
}

TEST(byte_stream_tests, can_handle_small_chunk_reads) {
  byte_stream_t stream = {.offset = 0,
                          .capacity = 10,
                          .stream_pointer = test_data,
                          .writer = incremental_writer};

  uint8_t read_buffer[50] = {0};
  byte_stream_status_e status = read_byte_stream(&stream, read_buffer, 50);

  TEST_ASSERT_EQUAL(BYTE_STREAM_SUCCESS, status);
  TEST_ASSERT_EQUAL_MEMORY(test_data, read_buffer, 50);
}

TEST(byte_stream_tests, skip_handles_large_offsets) {
  byte_stream_t stream = {.offset = 0,
                          .capacity = 100,
                          .stream_pointer = large_test_data,
                          .writer = stream_modifier_writer};

  byte_stream_status_e status = skip_byte_stream(&stream, 750);
  TEST_ASSERT_EQUAL(BYTE_STREAM_SUCCESS, status);
  TEST_ASSERT_EQUAL(50, stream.offset);    // 750 % 100 = 50
}

TEST(byte_stream_tests, handles_writer_changing_stream_parameters) {
  byte_stream_t stream = {.offset = 0,
                          .capacity = 50,
                          .stream_pointer = test_data,
                          .writer = sequential_error_writer};

  uint8_t read_buffer[100] = {0};
  byte_stream_status_e status = read_byte_stream(&stream, read_buffer, 100);

  TEST_ASSERT_EQUAL(BYTE_STREAM_FAILED_WRITER, status);
}

TEST(byte_stream_tests, read_exactly_capacity_size) {
  byte_stream_t stream = {.offset = 0,
                          .capacity = 100,
                          .stream_pointer = test_data,
                          .writer = stream_modifier_writer};

  uint8_t read_buffer[100] = {0};
  byte_stream_status_e status = read_byte_stream(&stream, read_buffer, 100);

  TEST_ASSERT_EQUAL(BYTE_STREAM_SUCCESS, status);
  TEST_ASSERT_EQUAL_MEMORY(test_data, read_buffer, 100);
}
