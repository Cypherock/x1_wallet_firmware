/**
 * @file    usb_evt_api_tests.c
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
#include "usb_evt_api_tests.h"

#include <string.h>

#include "usb_api.h"
#include "usb_api_priv.h"
#include "utils.h"

const uint8_t core_msg[] = {10, 2, 8, 1};
static uint8_t data[1024] = {0};

TEST_GROUP(usb_evt_api_test);

/**
 * @brief Test setup for usb event consumer tests.
 * @details The function populates data in local buffer of USB communication
 * module so that the event getter has an event ready for dispatch for
 * performing tests. buffer of packet(s) of data.
 */
TEST_SETUP(usb_evt_api_test) {
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

/**
 * @brief Tear down the old test data
 * @details The function will perform cleanup of the current running test and
 * bring the state of execution to a fresh start. This is done by using purge
 * api of usb-event and clearing buffers using usb-comm APIs.
 */
TEST_TEAR_DOWN(usb_evt_api_test) {
  usb_clear_event();
  LOG_SWV("\n\n\n");
}

TEST(usb_evt_api_test, basic) {
  TEST_ASSERT(usb_get_event(NULL) == false);
}

bool verify_event(uint32_t cmd_id, uint16_t msg_len, usb_event_t *evt) {
  uint16_t len = CY_MIN(msg_len, evt->msg_size);
  return ((evt->flag == true) && (evt->msg_size == msg_len));
}

/**
 * @brief Test behaviour for consume and free without resposne.
 * @details This test will ensure that the USB module APIs safely allow the
 * application to consume the event and its associated data and free the USB
 * buffer without generating any response to the event.
 */
TEST(usb_evt_api_test, consume_and_free) {
  LOG_SWV("consume_and_free\n");
  usb_event_t usb_evt;
  TEST_ASSERT_TRUE(usb_get_event(&usb_evt));
  TEST_ASSERT_TRUE(verify_event(89, 380, &usb_evt));
  TEST_ASSERT(usb_evt.p_msg[0] == 0x40);
  LOG_SWV("%s\n", usb_evt.p_msg);

  usb_clear_event();
  TEST_ASSERT(usb_get_event(&usb_evt) == false);

  uint8_t msg[1] = {0};
  usb_set_event(4, core_msg, 1, msg);

  TEST_ASSERT_TRUE(usb_get_event(&usb_evt));
  TEST_ASSERT_TRUE(verify_event(89, 1, &usb_evt));
}

/**
 * @brief Test behaviour of one-shot response after event consumption.
 * @details Test a typical scenario where application will be comsuming data of
 * an usb event further sending some data in one-shot over usb against the
 * received usb event.
 */
TEST(usb_evt_api_test, consume_and_respond) {
  usb_event_t usb_evt;
  TEST_ASSERT_TRUE(usb_get_event(&usb_evt));
  TEST_ASSERT_TRUE(verify_event(89, 380, &usb_evt));

  // send response over USB
  usb_send_msg(core_msg, 1, data, 1);
  TEST_ASSERT(usb_get_event(&usb_evt) == false);
}

/**
 * @brief Test behaviour for data stitching at application level.
 * @details An application could potentially want to receive large chunks of
 * data (specifically > current local buffer of usb-comm module). The usb-comm
 * and usb-event module should support such expectations.
 */
TEST(usb_evt_api_test, stitch_data_chunks) {
  LOG_SWV("usb evt stitch_data_chunks\n");
}

/**
 * @brief Test behaviour for response spliting at application level.
 * @details An application could potentially want to send large/multiple chunks
 * of data (specifically > current local buffer of usb-comm module). The
 * usb-comm and usb-event module should support such expectations.
 */
TEST(usb_evt_api_test, send_data_chunks) {
  // TODO: Add test
}

/**
 * @brief Test the behaviour of usb-comm module due to unexpected
 * calls made by application.
 * @details The APIs provided by usb-comm module should not interfere. In other
 * words, if an application makes call to usb-comm APIs in any order, the APIs
 * should be safe enough to provide recovery or solve hiccups. If an exception
 * is there, it should be tested separately. Mention its exclusion below.
 * Exceptions:
 * <ol>
 *   <li>None</li>
 * </ol>
 */
TEST(usb_evt_api_test, api_interference_1) {
  usb_event_t usb_evt;

  // 1. responding without getting event
  usb_send_msg(core_msg, 1, data, 1);
  TEST_ASSERT(usb_get_event(&usb_evt) == false);
  TEST_usb_evt_api_test_SETUP();
  TEST_ASSERT_TRUE(usb_get_event(&usb_evt));
  TEST_ASSERT_TRUE(verify_event(89, 380, &usb_evt));
}
