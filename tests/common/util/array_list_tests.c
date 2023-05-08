/**
 * @file    array_list_tests.c
 * @author  Cypherock X1 Team
 * @brief
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
#include <string.h>

#include "array_list.h"
#include "unity_fixture.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define ELEMENTS 10
/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static array_list_t array_list = {0};
static void *array_buffer[ELEMENTS] = {0};
static uint8_t test_data[ELEMENTS] =
    {0x11, 0x22, 0x33, 0xbc, 0xab, 0xdd, 0xa5, 0x66, 0x00, 0xdd};
/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
static void initialize_array_with_data(void) {
  for (uint8_t i = 0; i < ELEMENTS; i++) {
    uint8_t *ptr = &test_data[i];
    uint8_t **dptr = &ptr;
    array_list_insert(&array_list, (void *)dptr);
  }
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
TEST_GROUP(array_list_tests);

TEST_SETUP(array_list_tests) {
  array_list.array = &array_buffer[0];
  array_list.current_index = 0;
  array_list.num_of_elements = 0;
  array_list.max_capacity = sizeof(array_buffer) / sizeof(array_buffer[0]);
  array_list.size_of_element = sizeof(array_buffer[0]);
  memset(&array_buffer[0], 0, sizeof(array_buffer));
}

TEST_TEAR_DOWN(array_list_tests) {
  memset(&array_buffer[0], 0, sizeof(array_buffer));
}

// Test insertion
TEST(array_list_tests, insert_multiple) {
  for (uint8_t i = 0; i < ELEMENTS; i++) {
    uint8_t *ptr = &test_data[i];
    uint8_t **dptr = &ptr;
    TEST_ASSERT_TRUE(array_list_insert(&array_list, (void *)dptr));

    TEST_ASSERT_EQUAL(0, array_list.current_index);
    TEST_ASSERT_EQUAL(i + 1, array_list.num_of_elements);
  }
}

TEST(array_list_tests, insert_in_full_array) {
  initialize_array_with_data();

  uint8_t value = 0xff;
  uint8_t *ptr_value = &value;
  TEST_ASSERT_FALSE(array_list_insert(&array_list, (void *)&ptr_value));

  for (uint8_t i = 0; i < ELEMENTS; i++) {
    uint8_t *ptr;
    array_list_get_element(&array_list, &ptr);
    TEST_ASSERT_EQUAL_PTR(&test_data[i], ptr);
    TEST_ASSERT_FALSE(ptr == ptr_value);
    TEST_ASSERT_EQUAL_UINT8(test_data[i], *ptr);
    array_list_iterate_next(&array_list);
  }
}

TEST(array_list_tests, insert_NULL) {
  for (uint8_t i = 0; i < ELEMENTS / 2; i++) {
    uint8_t *ptr = &test_data[i];
    uint8_t **dptr = &ptr;
    array_list_insert(&array_list, (void *)dptr);
  }

  TEST_ASSERT_FALSE(array_list_insert(&array_list, NULL));
  TEST_ASSERT_EQUAL(ELEMENTS / 2, array_list.num_of_elements);
}

// Test deletions
TEST(array_list_tests, delete_single_element) {
  uint8_t *ptr = &test_data[0];
  uint8_t **dptr = &ptr;
  array_list_insert(&array_list, (void *)dptr);
  TEST_ASSERT_TRUE(array_list_delete_entry(&array_list));
}

TEST(array_list_tests, delete_first_element) {
  initialize_array_with_data();

  TEST_ASSERT_TRUE(array_list_delete_entry(&array_list));

  uint8_t *ptr;
  TEST_ASSERT_TRUE(array_list_get_element(&array_list, &ptr));
  TEST_ASSERT_EQUAL_PTR(&test_data[1], ptr);
  TEST_ASSERT_EQUAL_UINT8(test_data[1], *ptr);
}

TEST(array_list_tests, delete_last_element) {
  initialize_array_with_data();

  while (array_list_iterate_next(&array_list))
    ;

  TEST_ASSERT_TRUE(array_list_delete_entry(&array_list));

  uint8_t *ptr;
  TEST_ASSERT_TRUE(array_list_get_element(&array_list, &ptr));
  TEST_ASSERT_EQUAL_PTR(&test_data[ELEMENTS - 2], ptr);
  TEST_ASSERT_EQUAL_UINT8(test_data[ELEMENTS - 2], *ptr);

  while (array_list_iterate_back(&array_list))
    ;

  TEST_ASSERT_TRUE(array_list_get_element(&array_list, &ptr));
  TEST_ASSERT_EQUAL_PTR(&test_data[0], ptr);
  TEST_ASSERT_EQUAL_UINT8(test_data[0], *ptr);
}

TEST(array_list_tests, delete_middle_element) {
  initialize_array_with_data();

  for (uint8_t i = 0; i < ELEMENTS / 2; i++) {
    array_list_iterate_next(&array_list);
  }

  uint8_t *ptr;
  uint8_t *ptr_deleted = NULL;
  TEST_ASSERT_TRUE(array_list_get_element(&array_list, &ptr_deleted));
  TEST_ASSERT(NULL != ptr_deleted);

  TEST_ASSERT_TRUE(array_list_delete_entry(&array_list));

  // Check deletion
  TEST_ASSERT_TRUE(array_list_get_element(&array_list, &ptr));
  TEST_ASSERT_EQUAL_PTR(&test_data[ELEMENTS / 2 - 1], ptr);
  TEST_ASSERT_EQUAL_UINT8(test_data[ELEMENTS / 2 - 1], *ptr);

  // Check end
  while (array_list_iterate_next(&array_list))
    ;

  TEST_ASSERT_TRUE(array_list_get_element(&array_list, &ptr));
  TEST_ASSERT_EQUAL_PTR(&test_data[ELEMENTS - 1], ptr);
  TEST_ASSERT_EQUAL_UINT8(test_data[ELEMENTS - 1], *ptr);
  TEST_ASSERT_EQUAL(ELEMENTS - 2, array_list.current_index);

  // Check beginning
  while (array_list_iterate_back(&array_list))
    ;

  TEST_ASSERT_TRUE(array_list_get_element(&array_list, &ptr));
  TEST_ASSERT_EQUAL_PTR(&test_data[0], ptr);
  TEST_ASSERT_EQUAL_UINT8(test_data[0], *ptr);

  // Additionally test element insertion at the end and verify the same
  TEST_ASSERT_EQUAL(ELEMENTS - 1, array_list.num_of_elements);
  uint8_t value = 0xff;
  uint8_t *ptr_value = &value;
  TEST_ASSERT_TRUE(array_list_insert(&array_list, (void *)&ptr_value));

  while (array_list_iterate_next(&array_list)) {
    array_list_get_element(&array_list, &ptr);
    TEST_ASSERT_FALSE(ptr == ptr_deleted);
  }

  TEST_ASSERT_TRUE(array_list_get_element(&array_list, &ptr));
  TEST_ASSERT_EQUAL_PTR(ptr_value, ptr);
  TEST_ASSERT_EQUAL_UINT8(value, *ptr);
}

TEST(array_list_tests, delete_element_when_list_empty) {
  TEST_ASSERT_FALSE(array_list_delete_entry(&array_list));
}
