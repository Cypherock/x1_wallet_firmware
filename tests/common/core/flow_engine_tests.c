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

/*****************************************************************************
 * PRIVATE TYPEDEFS
 *****************************************************************************/
typedef struct {
  bool p0_event;
  bool ui_event;
  bool usb_event;
} event_callback_tester_t;

/*****************************************************************************
 * STATIC VARIABLES
 *****************************************************************************/
flow_step_t engine_test_flow[10];

const evt_config_t engine_test_evt_config = {.evt_selection = 0x7,
                                             .timeout = 1200};

event_callback_tester_t callback_test = {.p0_event = true,
                                         .ui_event = true,
                                         .usb_event = true};

static const uint8_t core_msg[] = {10, 2, 8, 1};

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTION PROTOTYPES
 *****************************************************************************/

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/
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
  usb_set_event(4, core_msg, length >> 1, data);
}

static void init_callback(engine_ctx_t *ctx, const void *data_ptr) {
  callback_test.p0_event = false;
  callback_test.ui_event = false;
  callback_test.usb_event = false;
  return;
}

static void p0_callback(engine_ctx_t *ctx,
                        p0_evt_t event,
                        const void *data_ptr) {
  callback_test.p0_event = true;
  ui_set_confirm_event();

  // Enter and go to next step using ctx
  engine_add_next_flow_step(ctx, &engine_test_flow[1]);
  engine_goto_next_flow_step(ctx);
  return;
}

static void ui_callback(engine_ctx_t *ctx,
                        ui_event_t event,
                        const void *data_ptr) {
  callback_test.ui_event = true;
  usb_construct_event();

  // Enter and go to next step using ctx
  engine_add_next_flow_step(ctx, &engine_test_flow[2]);
  engine_goto_next_flow_step(ctx);
  return;
}

static void usb_callback(engine_ctx_t *ctx,
                         usb_event_t event,
                         const void *data_ptr) {
  callback_test.usb_event = true;
  nfc_set_card_detect_event();

  // Test flow reached it's end, so empty the buffer and go back to caller
  TEST_ASSERT_TRUE(engine_reset_flow(ctx));
  return;
}

/*****************************************************************************
 * GLOBAL FUNCTIONS
 *****************************************************************************/
TEST_GROUP(flow_engine_tests);

TEST_SETUP(flow_engine_tests) {
  // Fill flow_step_t for a dummy flow
  engine_test_flow[0].step_init_cb = init_callback;
  engine_test_flow[0].p0_cb = p0_callback;
  engine_test_flow[0].ui_cb = NULL;
  engine_test_flow[0].usb_cb = NULL;
  engine_test_flow[0].nfc_cb = NULL;
  engine_test_flow[0].evt_cfg_ptr = &engine_test_evt_config;
  engine_test_flow[0].flow_data_ptr = NULL;

#if USE_SIMULATOR == 1
  engine_test_flow[1].step_init_cb = init_callback;
#else
  engine_test_flow[1].step_init_cb = NULL;
#endif
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
}

TEST_TEAR_DOWN(flow_engine_tests) {
}

TEST(flow_engine_tests, engine_use_case_test) {
  // Create flow buffer to hold the flow_step_t for the steps required
  flow_step_t *engine_test_buffer[10];

  engine_ctx_t flow_list = {
      .array = &engine_test_buffer[0],
      .current_index = 0,
      .max_capacity = sizeof(engine_test_buffer) / sizeof(flow_step_t *),
      .size_of_element = sizeof(flow_step_t *),
      .num_of_elements = 0};

  /* Reset buffer */
  TEST_ASSERT_TRUE(engine_reset_flow(&flow_list));

#if USE_SIMULATOR == 1
  /* Push 1st step, following steps will be pushed via the callbacks */
  engine_add_next_flow_step(&flow_list, &engine_test_flow[1]);
  // Set dummy UI event so that the flow reaches the UI CB
  ui_set_confirm_event();
#else
  /* Push 1st step, following steps will be pushed via the callbacks */
  engine_add_next_flow_step(&flow_list, &engine_test_flow[0]);
#endif

  // Run the engine until it reaches the last step
  engine_run(&flow_list);

#if USE_SIMULATOR == 1
  TEST_ASSERT_FALSE(callback_test.p0_event);
#else
  TEST_ASSERT_TRUE(callback_test.p0_event);
#endif

  TEST_ASSERT_TRUE(callback_test.ui_event);
  TEST_ASSERT_TRUE(callback_test.usb_event);
}