/**
 * @file    fifo_queue_tests.c
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
#include "cQueue.h"
#include "unity_fixture.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define NUM_MAX_RECORDS 10

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
static queue_t queue;
static uint8_t *queue_buffer[NUM_MAX_RECORDS] = {0};
static uint8_t data[NUM_MAX_RECORDS] =
    {0x11, 0x22, 0x33, 0x44, 0x55, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

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
TEST_GROUP(fifo_queue_tests);

TEST_SETUP(fifo_queue_tests) {
  q_init_static(&queue,
                sizeof(uintptr_t),
                NUM_MAX_RECORDS,
                FIFO,
                &queue_buffer[0],
                sizeof(queue_buffer));
}

TEST_TEAR_DOWN(fifo_queue_tests) {
}

TEST(fifo_queue_tests, enqueue_dequeue) {
  /* Push pointer to data[i] into the stack; we can also push the data itself
   * and check. But currently, the requirement is to store pointer to variables
   * onto the stack/queue itself  */
  for (uint8_t i = 0; i < NUM_MAX_RECORDS; i++) {
    uint8_t *ptr = &data[i];
    uint8_t **dptr = &ptr;
    TEST_ASSERT_TRUE(q_push(&queue, (void *)dptr));

    /* Check if we can peek the FIRST element we pushed (as it's a queue),
     * without popping it from the queue */
    ptr = NULL;
    TEST_ASSERT_TRUE(q_peek(&queue, (void *)&ptr));
    TEST_ASSERT_EQUAL_PTR(&data[0], ptr);
    TEST_ASSERT_EQUAL_UINT8(data[0], *ptr);
  }

  /* Once the queue is full, we will not be able to push into the queue */
  uint8_t *ptr = &data[0];
  uint8_t **dptr = &ptr;
  TEST_ASSERT_FALSE(q_push(&queue, (void *)dptr));

  /* Verify the pointers and the data after pop */
  for (uint8_t i = 0; i < NUM_MAX_RECORDS; i++) {
    uint8_t *ptr;
    TEST_ASSERT_TRUE(q_pop(&queue, (void *)&ptr));

    /* Since it's a FIFO (queue), we will get the corresponding pointer */
    TEST_ASSERT_EQUAL_PTR(&data[i], ptr);
    TEST_ASSERT_EQUAL_UINT8(data[i], *ptr);
  }

  /* Once the queue is empty, we will not be able to pop from the queue */
  TEST_ASSERT_FALSE(q_pop(&queue, (void *)&ptr));
}
