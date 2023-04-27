/**
 * @file    flow_engine_tests.c
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
#include "flow_engine.h"
#include "nfc_events_priv.h"
#include "ui_events_priv.h"
#include "unity_fixture.h"
#include "usb_api_priv.h"

/*****************************************************************************
 * EXTERN VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * PRIVATE MACROS AND DEFINES
 *****************************************************************************/
#define ENGINE_TEST_NUM_FLOWS ENGINE_STACK_DEPTH
/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef struct {
  bool p0_event;
  bool ui_event;
  bool usb_event;
  bool nfc_event;
} event_callback_tester_t;

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
flow_step_t engine_test_flow[ENGINE_TEST_NUM_FLOWS + 1];

const evt_config_t engine_test_evt_config = {.abort_disabled = false,
                                             .evt_selection.byte = 0x7};

event_callback_tester_t callback_test = {.p0_event = false,
                                         .ui_event = false,
                                         .usb_event = false,
                                         .nfc_event = false};

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static void init_callback(const void *data_ptr) {
  callback_test.p0_event = false;
  callback_test.ui_event = false;
  callback_test.usb_event = false;
  callback_test.nfc_event = false;
  return;
}

static void p0_callback(p0_evt_t event, const void *data_ptr) {
  callback_test.p0_event = true;
  engine_next_flow_step(ENGINE_LIFO_A, &engine_test_flow[1]);
  return;
}

static void ui_callback(ui_event_t event, const void *data_ptr) {
  callback_test.ui_event = true;
  engine_next_flow_step(ENGINE_LIFO_A, &engine_test_flow[2]);
  return;
}

static void usb_callback(usb_event_t event, const void *data_ptr) {
  callback_test.usb_event = true;
  engine_next_flow_step(ENGINE_LIFO_A, &engine_test_flow[3]);
  return;
}

static void nfc_callback(nfc_event_t event, const void *data_ptr) {
  callback_test.nfc_event = true;

  /* This is the leaf node - functionality, which can choose to either call
   * engine_reset or engine_back */
  TEST_ASSERT_TRUE(engine_reset_flow(ENGINE_LIFO_A));

  return;
}

// Copied from usb_evt_api_tests.c
static void usb_construct_event(void) {
  uint8_t data[1024] = {0};
  LOG_SWV("Setting up test...\n");
  const char *hex_str =
      "406272696566205465737420746865206265686176696f7572206f662077726f6e672063"
      "6d642068616e646c696e672041504973206174207573622d636f6d6d206d6f64756c652e"
      "204064657461696c7320546865206170706c69636174696f6e2073686f756c6420686176"
      "652072656c6576616e74204150497320746f2068616e646c6520756e6578706563746564"
      "20646174612065786368616e67652061742072756e74696d652e20546869732066756e63"
      "74696f6e207465737473206f6e6520737563682077617920746f2068616e646c6520756e"
      "657870656374656420646174612066726f6d20686f73743b207768696368206973206279"
      "20726573706f6e64696e67207769746820616e207573622d636f6d6d64206c6576656c20"
      "6572726f7220636f646520696e7369646520746865206572726f72207061636b65742073"
      "756368207468617420746865207573622d636f6d6d206d6f64756c652074726561747320"
      "697420617320616e206572726f7220636173652e";
  uint16_t length = strlen(hex_str);
  hex_string_to_byte_array(hex_str, length, data);
  usb_set_event(89, data, length >> 1);
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
TEST_GROUP(flow_engine_tests);

TEST_SETUP(flow_engine_tests) {
  engine_initialize();
}

TEST_TEAR_DOWN(flow_engine_tests) {
}

TEST(flow_engine_tests, queue_empty_operations) {
  flow_step_t *flow_step_ptr;
  /* Try to get data from empty queue */
  TEST_ASSERT_FALSE(engine_current_flow_step(ENGINE_LIFO_A, &flow_step_ptr));
  TEST_ASSERT_FALSE(engine_current_flow_step(ENGINE_FIFO_A, &flow_step_ptr));

  /* Try to pop data from empty queue */
  TEST_ASSERT_FALSE(engine_prev_flow_step(ENGINE_LIFO_A));
  TEST_ASSERT_FALSE(engine_prev_flow_step(ENGINE_FIFO_A));

  /* Try to get data from unimplemented engine queue */
  // Only 1 stack and 1 queue implemented right now, therefore 3 is invalid
  // stack index
  TEST_ASSERT_FALSE(engine_current_flow_step(3, &flow_step_ptr));

  /* Try to pop data from unimplemented engine queue */
  // Only 1 stack and 1 queue implemented right now, therefore 2 is invalid
  // stack index
  TEST_ASSERT_FALSE(engine_prev_flow_step(2));
}

TEST(flow_engine_tests, dummy_flows_across_diff_queues) {
  for (uint8_t i = 0; i < ENGINE_TEST_NUM_FLOWS; i++) {
    engine_test_flow[i].step_init_cb = NULL;
    engine_test_flow[i].p0_cb = NULL;
    engine_test_flow[i].ui_cb = NULL;
    engine_test_flow[i].usb_cb = NULL;
    engine_test_flow[i].nfc_cb = NULL;
    engine_test_flow[i].flow_data_ptr = NULL;
    engine_test_flow[i].evt_cfg_ptr = &engine_test_evt_config;
    TEST_ASSERT_TRUE(
        engine_next_flow_step(ENGINE_LIFO_A, &engine_test_flow[i]));

    TEST_ASSERT_TRUE(
        engine_next_flow_step(ENGINE_FIFO_A, &engine_test_flow[i]));
  }

  /* Verify flow engine by dequeuing from both LIFO and FIFO */
  for (uint8_t i = 0; i < ENGINE_TEST_NUM_FLOWS; i++) {
    flow_step_t *flow_cb_popped = NULL;

    TEST_ASSERT_TRUE(engine_current_flow_step(ENGINE_LIFO_A, &flow_cb_popped));
    TEST_ASSERT_EQUAL_PTR(&engine_test_flow[ENGINE_TEST_NUM_FLOWS - i - 1],
                          flow_cb_popped);
    TEST_ASSERT_TRUE(engine_prev_flow_step(ENGINE_LIFO_A));

    TEST_ASSERT_TRUE(engine_current_flow_step(ENGINE_FIFO_A, &flow_cb_popped));
    TEST_ASSERT_EQUAL_PTR(&engine_test_flow[i], flow_cb_popped);
    TEST_ASSERT_TRUE(engine_prev_flow_step(ENGINE_FIFO_A));
  }
}

TEST(flow_engine_tests, async_init_one_buffer) {
  for (uint8_t i = 0; i < ENGINE_TEST_NUM_FLOWS; i++) {
    engine_test_flow[i].step_init_cb = NULL;
    engine_test_flow[i].p0_cb = NULL;
    engine_test_flow[i].ui_cb = NULL;
    engine_test_flow[i].usb_cb = NULL;
    engine_test_flow[i].nfc_cb = NULL;
    engine_test_flow[i].flow_data_ptr = NULL;
    engine_test_flow[i].evt_cfg_ptr = &engine_test_evt_config;
    TEST_ASSERT_TRUE(
        engine_next_flow_step(ENGINE_LIFO_A, &engine_test_flow[i]));

    TEST_ASSERT_TRUE(
        engine_next_flow_step(ENGINE_FIFO_A, &engine_test_flow[i]));
  }

  /* Reset ENGINE_LIFO_A */
  TEST_ASSERT_TRUE(engine_reset_flow(ENGINE_LIFO_A));

  /* Verify flow engine by dequeuing from FIFO */
  for (uint8_t i = 0; i < ENGINE_TEST_NUM_FLOWS; i++) {
    flow_step_t *flow_cb_popped = NULL;

    TEST_ASSERT_FALSE(engine_current_flow_step(ENGINE_LIFO_A, &flow_cb_popped));
    TEST_ASSERT_FALSE(engine_prev_flow_step(ENGINE_LIFO_A));

    TEST_ASSERT_TRUE(engine_current_flow_step(ENGINE_FIFO_A, &flow_cb_popped));
    TEST_ASSERT_EQUAL_PTR(&engine_test_flow[i], flow_cb_popped);
    TEST_ASSERT_TRUE(engine_prev_flow_step(ENGINE_FIFO_A));
  }
}

TEST(flow_engine_tests, flow_push_beyond_stack_depth) {
  for (uint8_t i = 0; i < ENGINE_TEST_NUM_FLOWS; i++) {
    engine_test_flow[i].step_init_cb = NULL;
    engine_test_flow[i].p0_cb = NULL;
    engine_test_flow[i].ui_cb = NULL;
    engine_test_flow[i].usb_cb = NULL;
    engine_test_flow[i].nfc_cb = NULL;
    engine_test_flow[i].flow_data_ptr = NULL;
    engine_test_flow[i].evt_cfg_ptr = &engine_test_evt_config;
    TEST_ASSERT_TRUE(
        engine_next_flow_step(ENGINE_LIFO_A, &engine_test_flow[i]));

    TEST_ASSERT_TRUE(
        engine_next_flow_step(ENGINE_FIFO_A, &engine_test_flow[i]));
  }

  /* Enqueueing more steps in the buffer should give an error */
  TEST_ASSERT_FALSE(engine_next_flow_step(ENGINE_LIFO_A, &engine_test_flow[0]));

  TEST_ASSERT_FALSE(engine_next_flow_step(ENGINE_FIFO_A, &engine_test_flow[0]));
}

TEST(flow_engine_tests, engine_test_lifo) {
  // Fill flow_step_t for a dummy flow
  engine_test_flow[0].step_init_cb = init_callback;
  engine_test_flow[0].p0_cb = p0_callback;
  engine_test_flow[0].ui_cb = NULL;
  engine_test_flow[0].usb_cb = NULL;
  engine_test_flow[0].nfc_cb = NULL;
  engine_test_flow[0].evt_cfg_ptr = &engine_test_evt_config;
  engine_test_flow[0].flow_data_ptr = NULL;

  engine_test_flow[1].step_init_cb = init_callback;
  engine_test_flow[1].p0_cb = NULL;
  engine_test_flow[1].ui_cb = ui_callback;
  engine_test_flow[1].usb_cb = NULL;
  engine_test_flow[1].nfc_cb = NULL;
  engine_test_flow[1].evt_cfg_ptr = &engine_test_evt_config;
  engine_test_flow[1].flow_data_ptr = NULL;

  engine_test_flow[2].step_init_cb = NULL;
  engine_test_flow[2].p0_cb = NULL;
  engine_test_flow[2].ui_cb = NULL;
  engine_test_flow[2].usb_cb = usb_callback;
  engine_test_flow[2].nfc_cb = NULL;
  engine_test_flow[2].evt_cfg_ptr = &engine_test_evt_config;
  engine_test_flow[2].flow_data_ptr = NULL;

  engine_test_flow[3].step_init_cb = NULL;
  engine_test_flow[3].p0_cb = NULL;
  engine_test_flow[3].ui_cb = NULL;
  engine_test_flow[3].usb_cb = NULL;
  engine_test_flow[3].nfc_cb = nfc_callback;
  engine_test_flow[3].evt_cfg_ptr = &engine_test_evt_config;
  engine_test_flow[3].flow_data_ptr = NULL;

  /* Reset flow stack ENGINE_LIFO_A */
  TEST_ASSERT_TRUE(engine_reset_flow(ENGINE_LIFO_A));

  /* Push 1st step, following steps will be pushed via the callbacks */
  engine_next_flow_step(ENGINE_LIFO_A, &engine_test_flow[0]);

  engine_run(ENGINE_LIFO_A);

  TEST_ASSERT_TRUE(callback_test.p0_event);
  TEST_ASSERT_TRUE(callback_test.ui_event);
  TEST_ASSERT_TRUE(callback_test.usb_event);
  TEST_ASSERT_TRUE(callback_test.nfc_event);
}